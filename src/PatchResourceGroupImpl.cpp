// Copyright © 2025 CCP ehf.

#include "PatchResourceGroupImpl.h"

#include "Patching.h"

#include "BundleResourceGroupImpl.h"

#include <yaml-cpp/yaml.h>

#include <ResourceTools.h>

#include <ScopedFile.h>

#include <FileDataStreamIn.h>

#include <FileDataStreamOut.h>

#include <Md5ChecksumStream.h>

namespace CarbonResources
{
PatchResourceGroup::PatchResourceGroupImpl::PatchResourceGroupImpl() :
	ResourceGroupImpl()
{
	m_resourceGroupParameter = new ResourceGroupInfo( {} );

	m_type = TypeId();
}

Result PatchResourceGroup::PatchResourceGroupImpl::SetResourceGroup( const ResourceGroupInfo& resourceGroup )
{
	// Creates a deep copy
	ResourceGroupInfo* info = m_resourceGroupParameter.GetValue();

	Result result = info->SetParametersFromResource( &resourceGroup, m_versionParameter.GetValue() );

	if( result.type != ResultType::SUCCESS )
	{
		return result;
	}

	// Change the relative path to include the checksum as part of the file name.
	// This should prevent issues with name collisions between files called "ResourceGroup.yaml"
	std::filesystem::path relativePath;

	result = info->GetRelativePath( relativePath );

	if( result.type != ResultType::SUCCESS )
	{
		return result;
	}

	std::string checksum;

	result = info->GetChecksum( checksum );

	if( result.type != ResultType::SUCCESS )
	{
		return result;
	}

	const std::string extension = relativePath.extension().string();

	relativePath.replace_filename( "DiffResourceGroup_" + checksum + extension );

	info->SetRelativePath( relativePath );

	return Result{ ResultType::SUCCESS };
}

Result PatchResourceGroup::PatchResourceGroupImpl::SetMaxInputChunkSize( uintmax_t maxInputChunkSize )
{
	if( maxInputChunkSize <= 0 )
	{
		return Result{ ResultType::INVALID_CHUNK_SIZE };
	}

	m_maxInputChunkSize = maxInputChunkSize;

	return Result{ ResultType::SUCCESS };
}

PatchResourceGroup::PatchResourceGroupImpl::~PatchResourceGroupImpl()
{
	delete m_resourceGroupParameter.GetValue();
}

std::string PatchResourceGroup::PatchResourceGroupImpl::GetType() const
{
	return TypeId();
}

std::string PatchResourceGroup::PatchResourceGroupImpl::TypeId()
{
	return "PatchGroup";
}

Result PatchResourceGroup::PatchResourceGroupImpl::CreateResourceFromYaml( YAML::Node& resource, ResourceInfo*& resourceOut )
{
	PatchResourceInfo* patchResource = new PatchResourceInfo( PatchResourceInfoParams{} );

	Result importFromYamlResult = patchResource->ImportFromYaml( resource, m_versionParameter.GetValue() );

	if( importFromYamlResult.type != ResultType::SUCCESS )
	{
		delete patchResource;

		return importFromYamlResult;
	}
	else
	{
		resourceOut = patchResource;

		return Result{ ResultType::SUCCESS };
	}
}

Result PatchResourceGroup::PatchResourceGroupImpl::ImportGroupSpecialisedYaml( YAML::Node& resourceGroupFile )
{
	if( m_resourceGroupParameter.IsParameterExpectedInDocumentVersion( m_versionParameter.GetValue() ) )
	{
		YAML::Node resourceGroupNode = resourceGroupFile[m_resourceGroupParameter.GetTag()];

		ResourceInfo* resource = nullptr;

		Result createResourceFromYaml = ResourceGroupImpl::CreateResourceFromYaml( resourceGroupNode, resource );

		if( createResourceFromYaml.type != ResultType::SUCCESS )
		{
			return createResourceFromYaml;
		}

		// Ensure that resource is of base type ResourceGroup
		ResourceGroupInfo* resourceGroupInfo = dynamic_cast<ResourceGroupInfo*>( resource );

		if( !resourceGroupInfo )
		{
			return Result{ ResultType::MALFORMED_RESOURCE_GROUP };
		}

		m_resourceGroupParameter = reinterpret_cast<ResourceGroupInfo*>( resource );
	}


	if( m_maxInputChunkSize.IsParameterExpectedInDocumentVersion( m_versionParameter.GetValue() ) )
	{
		if( YAML::Node parameter = resourceGroupFile[m_maxInputChunkSize.GetTag()] )
		{
			m_maxInputChunkSize = parameter.as<uintmax_t>();
		}
		else
		{
			return Result{ ResultType::MALFORMED_RESOURCE_INPUT };
		}
	}

	if( m_removedResources.IsParameterExpectedInDocumentVersion( m_versionParameter.GetValue() ) )
	{
		YAML::Node parameter = resourceGroupFile[m_removedResources.GetTag()];
		if( parameter.IsDefined() && parameter.IsSequence() )
		{
			for( size_t i = 0; i < parameter.size(); ++i )
			{
				std::filesystem::path path = parameter[i].as<std::string>();
				m_removedResources.PushBack( path );
			}
		}
	}

	return Result{ ResultType::SUCCESS };
}

Result PatchResourceGroup::PatchResourceGroupImpl::ExportGroupSpecialisedYaml( YAML::Emitter& out, VersionInternal outputDocumentVersion ) const
{
	if( m_resourceGroupParameter.IsParameterExpectedInDocumentVersion( outputDocumentVersion ) )
	{
		out << YAML::Key << m_resourceGroupParameter.GetTag();

		out << YAML::Value << YAML::BeginMap;

		m_resourceGroupParameter.GetValue()->ExportToYaml( out, outputDocumentVersion );

		out << YAML::EndMap;
	}

	// Data offset
	if( m_maxInputChunkSize.IsParameterExpectedInDocumentVersion( outputDocumentVersion ) )
	{
		if( !m_maxInputChunkSize.HasValue() )
		{
			return Result{ ResultType::REQUIRED_RESOURCE_PARAMETER_NOT_SET };
		}

		out << YAML::Key << m_maxInputChunkSize.GetTag();
		out << YAML::Value << m_maxInputChunkSize.GetValue();
	}

	if( m_removedResources.IsParameterExpectedInDocumentVersion( outputDocumentVersion ) )
	{
		out << YAML::Key << m_removedResources.GetTag();
		out << YAML::Value << YAML::BeginSeq;
		for( auto path : *m_removedResources.GetValue() )
		{
			out << path.string();
		}
		out << YAML::EndSeq;
	}

	return Result{ ResultType::SUCCESS };
}


Result PatchResourceGroup::PatchResourceGroupImpl::GetTargetResourcePatches( const ResourceInfo* resource, std::vector<const PatchResourceInfo*>& patches ) const
{
	std::filesystem::path resourceRelativePath;

	Result resourceRelativePathResult = resource->GetRelativePath( resourceRelativePath );

	if( resourceRelativePathResult.type != ResultType::SUCCESS )
	{
		return resourceRelativePathResult;
	}

	for( ResourceInfo* patchResource : m_resourcesParameter )
	{
		PatchResourceInfo* patch = reinterpret_cast<PatchResourceInfo*>( patchResource );

		std::filesystem::path patchTargetResource;

		Result getPatchTargetResource = patch->GetTargetResourceRelativePath( patchTargetResource );

		if( getPatchTargetResource.type != ResultType::SUCCESS )
		{
			return getPatchTargetResource;
		}

		if( resourceRelativePath == patchTargetResource )
		{
			patches.push_back( patch );
		}
	}

	return Result{ ResultType::SUCCESS };
}

Result PatchResourceGroup::PatchResourceGroupImpl::Apply( PatchApplyParams& params, StatusSettings& statusSettings )
{
	statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 0, 10, "Applying Patch." );

	// Will be removed when falls out of scope
	ResourceTools::ScopedFile temporaryFileScope( params.temporaryFilePath );

	ResourceGroupInfo* resourceGroupResource = m_resourceGroupParameter.GetValue();


	// Load the resourceGroup from the resourceGroupResource
	std::string resourceGroupData;

	ResourceGetDataParams resourceGroupDataParams;

	resourceGroupDataParams.resourceSourceSettings = params.patchBinarySourceSettings;

	resourceGroupDataParams.data = &resourceGroupData;

    resourceGroupDataParams.downloadSettings = params.downloadSettings;

	Result resourceGroupGetDataResult = m_resourceGroupParameter.GetValue()->GetData( resourceGroupDataParams );

	if( resourceGroupGetDataResult.type != ResultType::SUCCESS )
	{
		return resourceGroupGetDataResult;
	}

	ResourceGroupImpl resourceGroup;

    {
		StatusSettings importFromDataStatusSettings;
		statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 10, 10, "Applying Patch.", &importFromDataStatusSettings );

		Result resourceGroupImportFromDataResult = resourceGroup.ImportFromData( resourceGroupData, importFromDataStatusSettings );

		if( resourceGroupImportFromDataResult.type != ResultType::SUCCESS )
		{
			return resourceGroupImportFromDataResult;
		}
	}

    std::vector<ResourceInfo*> resourcesToCopy;

    {
		StatusSettings patchingStatusSettings;
		statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 20, 65, "Applying Patch.", &patchingStatusSettings );

		size_t numPatchesProcessed = 0;

		for( ResourceInfo* resource : resourceGroup )
		{
			{
			    // See if there is a patch available for resource
			    std::vector<const PatchResourceInfo*> patchesForResource;

			    Result getTargetResourcePatchesResult = GetTargetResourcePatches( resource, patchesForResource );

			    if( getTargetResourcePatchesResult.type != ResultType::SUCCESS )
			    {
				    return getTargetResourcePatchesResult;
			    }

                StatusSettings chunkStatusSettings;

				if( patchingStatusSettings.RequiresStatusUpdates() && patchesForResource.size() > 0 )
				{
					std::filesystem::path relativePath;

					if( resource->GetRelativePath( relativePath ).type != ResultType::SUCCESS )
					{
						return Result{ ResultType::FAIL };
					}

					float totalPatches = static_cast<float>( m_resourcesParameter.GetSize() );
                    float percentage = ( 100.0f / totalPatches ) * static_cast<float>( numPatchesProcessed );
					float step = ( 100.0f / totalPatches ) * static_cast<float>( patchesForResource.size() );

					std::string message = "Patching: " + relativePath.string();

					patchingStatusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, percentage, step, message, &chunkStatusSettings );
				}

			    numPatchesProcessed += patchesForResource.size();

			    // Open a stream to write a temp file of the patched resource
				std::filesystem::path resourceRelativeDestinationPath;
				Result getDestinationPathResult = resource->GetDestinationPathRelative( params.resourcesToPatchDestinationSettings, resourceRelativeDestinationPath );

                if (getDestinationPathResult.type != ResultType::SUCCESS)
                {
				    return getDestinationPathResult;
                }

                std::filesystem::path tempResourceDestinationPath = params.temporaryFilePath / resourceRelativeDestinationPath;

			    ResourceTools::FileDataStreamOut temporaryResourceDataStreamOut;

			    if( !temporaryResourceDataStreamOut.StartWrite( tempResourceDestinationPath ) )
			    {
				    return Result{ ResultType::FAILED_TO_OPEN_FILE };
			    }

                resourcesToCopy.push_back( resource );

			    // This can be skipped if the file is new and new files are skipped in params
			    bool skipChecksumCheck = false;

			    // Incrementally calculate checksum for temporary patch file
			    ResourceTools::Md5ChecksumStream patchedFileChecksumStream;

			    if( patchesForResource.size() > 0 )
			    {
				    // Open stream for resource
				    auto resourceDataStreamIn = std::make_shared<ResourceTools::FileDataStreamIn>( m_maxInputChunkSize.GetValue() );

				    ResourceGetDataStreamParams resourceDataStreamParams;

				    resourceDataStreamParams.resourceSourceSettings = params.resourcesToPatchSourceSettings;

				    resourceDataStreamParams.dataStream = resourceDataStreamIn;

				    resourceDataStreamParams.downloadSettings = params.downloadSettings;

				    Result getResourceDataStream = resource->GetDataStream( resourceDataStreamParams );

				    if( getResourceDataStream.type != ResultType::SUCCESS )
				    {
					    return getResourceDataStream;
				    }

				    size_t patchesForResourceProcessed = 0;

				    for( auto patchIter = patchesForResource.begin(); patchIter != patchesForResource.end(); patchIter++ )
				    {
					    const PatchResourceInfo* patch = ( *patchIter );

					    if( chunkStatusSettings.RequiresStatusUpdates() )
					    {
						    std::filesystem::path relativePath;

						    if( patch->GetRelativePath( relativePath ).type != ResultType::SUCCESS )
						    {
							    return Result{ ResultType::FAIL };
						    }

						    float step = static_cast<float>( 100.0 / patchesForResource.size() );
						    float percentage = static_cast<float>( step * patchesForResourceProcessed );

						    std::string message = "Applying patch: " + relativePath.string();

						    chunkStatusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, percentage, step, message );

						    patchesForResourceProcessed++;
					    }

					    // Patch found, Retreive and apply
					    std::string patchData;

					    ResourceGetDataParams patchGetDataParams;

					    patchGetDataParams.resourceSourceSettings = params.patchBinarySourceSettings;

					    patchGetDataParams.data = &patchData;

					    patchGetDataParams.downloadSettings = params.downloadSettings;

					    std::string location;
					    Result patchGetLocationResult = patch->GetLocation( location );
					    if( patchGetLocationResult.type != ResultType::SUCCESS )
					    {
						    return patchGetLocationResult;
					    }
					    bool hasPatchFile{ !location.empty() };

					    if( hasPatchFile )
					    {
						    Result getPatchDataResult = patch->GetData( patchGetDataParams );

						    if( getPatchDataResult.type != ResultType::SUCCESS )
						    {
							    return getPatchDataResult;
						    }
					    }

					    // Get previous data
					    uintmax_t dataOffset;
					    uintmax_t sourceOffset;
					    Result getPatchDataOffset = patch->GetDataOffset( dataOffset );

					    if( getPatchDataOffset.type != ResultType::SUCCESS )
					    {
						    return getPatchDataOffset;
					    }

					    Result getPatchSourceOffset = patch->GetSourceOffset( sourceOffset );
					    if( getPatchSourceOffset.type != ResultType::SUCCESS )
					    {
						    return getPatchSourceOffset;
					    }

					    std::string previousResourceData;

					    // Get previous size of resource
					    uintmax_t previousUncompressedSize;

					    Result getPreviousUncompressedSize = resource->GetUncompressedSize( previousUncompressedSize );

					    if( getPreviousUncompressedSize.type != ResultType::SUCCESS )
					    {
						    return getPreviousUncompressedSize;
					    }

					    if( dataOffset < previousUncompressedSize )
					    {
						    int64_t previousSourcePosition = resourceDataStreamIn->GetCurrentPosition();
						    // Get to location of patch
						    while( temporaryResourceDataStreamOut.GetFileSize() < dataOffset )
						    {
							    std::string dataChunk;
							    uint64_t remaining = dataOffset - temporaryResourceDataStreamOut.GetFileSize();
							    if( remaining < m_maxInputChunkSize.GetValue() )
							    {
								    if( !resourceDataStreamIn->ReadBytes( remaining, dataChunk ) )
								    {
									    return Result{ ResultType::FAILED_TO_READ_FROM_STREAM };
								    }
							    }
							    else if( !( *resourceDataStreamIn >> dataChunk ) )
							    {
								    return Result{ ResultType::FAILED_TO_READ_FROM_STREAM };
							    }

							    if( !( temporaryResourceDataStreamOut << dataChunk ) )
							    {
								    return Result{ ResultType::FAILED_TO_WRITE_TO_STREAM };
							    }

							    // Add to incremental checksum calculation
							    if( !( patchedFileChecksumStream << dataChunk ) )
							    {
								    return Result{ ResultType::FAILED_TO_GENERATE_CHECKSUM };
							    }
							    previousSourcePosition += dataChunk.size();
						    }
						    if( resourceDataStreamIn->IsFinished() )
						    {
							    resourceDataStreamIn->StartRead( resourceDataStreamIn->GetPath() );
						    }
						    resourceDataStreamIn->Seek( previousSourcePosition );


						    // Apply the patch to the previous data
						    std::string patchedResourceData;

						    if( hasPatchFile )
						    {
							    // Apply patch to data
							    resourceDataStreamIn->Seek( sourceOffset );
							    if( !( *resourceDataStreamIn >> previousResourceData ) )
							    {
								    return Result{ ResultType::FAILED_TO_READ_FROM_STREAM };
							    }
							    if( !ResourceTools::ApplyPatch( previousResourceData, patchData, patchedResourceData ) )
							    {
								    return Result{ ResultType::FAILED_TO_APPLY_PATCH };
							    }
							    // Write the patch result to file
							    if( !( temporaryResourceDataStreamOut << patchedResourceData ) )
							    {
								    return Result{ ResultType::FAILED_TO_WRITE_TO_STREAM };
							    }

							    // Add to incremental checksum calculation
							    if( !( patchedFileChecksumStream << patchedResourceData ) )
							    {
								    return Result{ ResultType::FAILED_TO_GENERATE_CHECKSUM };
							    }
						    }
						    else
						    {

							    auto sourceDataStreamIn = std::make_shared<ResourceTools::FileDataStreamIn>( m_maxInputChunkSize.GetValue() );

							    ResourceGetDataStreamParams getDataStreamParams;

							    getDataStreamParams.dataStream = sourceDataStreamIn;

							    getDataStreamParams.resourceSourceSettings = params.resourcesToPatchSourceSettings;

							    getDataStreamParams.downloadSettings = params.downloadSettings;

							    Result getDataStreamResult = resource->GetDataStream( getDataStreamParams );

							    if( getDataStreamResult.type != ResultType::SUCCESS )
							    {
								    return getDataStreamResult;
							    }

							    uintmax_t sourceOffset{ 0 };
							    Result getSourceOffsetResult = patch->GetSourceOffset( sourceOffset );
							    if( getSourceOffsetResult.type != ResultType::SUCCESS )
							    {
								    return getSourceOffsetResult;
							    }
							    uintmax_t unCompressedSize{ 0 };
							    Result getUncompressedSizeResult = patch->GetUncompressedSize( unCompressedSize );
							    if( getUncompressedSizeResult.type != ResultType::SUCCESS )
							    {
								    return getUncompressedSizeResult;
							    }
							    sourceDataStreamIn->Seek( sourceOffset );
							    while( unCompressedSize )
							    {
								    std::string sourceData;
								    if( unCompressedSize >= m_maxInputChunkSize.GetValue() )
								    {
									    *sourceDataStreamIn >> sourceData;
								    }
								    else
								    {
									    sourceDataStreamIn->ReadBytes( unCompressedSize, sourceData );
								    }

								    if( sourceData.empty() )
								    {
									    return Result{ ResultType::FAILED_TO_READ_FROM_STREAM };
								    }
								    *resourceDataStreamIn >> previousResourceData;
								    if( sourceData.size() > unCompressedSize )
								    {
									    sourceData = sourceData.substr( unCompressedSize );
								    }
								    unCompressedSize -= std::min( sourceData.size(), unCompressedSize );

								    // Write the data from the source file
								    if( !( temporaryResourceDataStreamOut << sourceData ) )
								    {
									    return Result{ ResultType::FAILED_TO_WRITE_TO_STREAM };
								    }

								    // Add to incremental checksum calculation
								    if( !( patchedFileChecksumStream << sourceData ) )
								    {
									    return Result{ ResultType::FAILED_TO_GENERATE_CHECKSUM };
								    }
							    }
						    }
					    }
					    else
					    {
						    // New data, append on to end
						    if( !( temporaryResourceDataStreamOut << previousResourceData ) )
						    {
							    return Result{ ResultType::FAILED_TO_WRITE_TO_STREAM };
						    }

						    // Add to incremental checksum calculation
						    if( !( patchedFileChecksumStream << previousResourceData ) )
						    {
							    return Result{ ResultType::FAILED_TO_GENERATE_CHECKSUM };
						    }
					    }
				    }

				    // Stream out the remaining expected data
				    uintmax_t expectedResourceSize = 0;

				    Result getResourceUncompressedSizeResult = resource->GetUncompressedSize( expectedResourceSize );

				    if( getResourceUncompressedSizeResult.type != ResultType::SUCCESS )
				    {
					    return getResourceUncompressedSizeResult;
				    }

				    temporaryResourceDataStreamOut.Finish();
			    }
			    else
			    {
				    // No Patch found, indicates this is just a new file
				    if( !params.skipNewFiles )
				    {
					    // Just replace file directly
					    auto resourceStreamIn = std::make_shared<ResourceTools::FileDataStreamIn>( m_maxInputChunkSize.GetValue() );

					    ResourceGetDataStreamParams resourceGetDataParams;

					    resourceGetDataParams.resourceSourceSettings = params.nextBuildResourcesSourceSettings;

					    resourceGetDataParams.dataStream = resourceStreamIn;

					    resourceGetDataParams.downloadSettings = params.downloadSettings;

					    Result resourceGetDataResult = resource->GetDataStream( resourceGetDataParams );

					    if( resourceGetDataResult.type != ResultType::SUCCESS )
					    {
						    return resourceGetDataResult;
					    }

					    while( !resourceStreamIn->IsFinished() )
					    {
						    std::string resourceData;

						    if( !( *resourceStreamIn >> resourceData ) )
						    {
							    return Result{ ResultType::FAILED_TO_READ_FROM_STREAM };
						    }

						    if( !( temporaryResourceDataStreamOut << resourceData ) )
						    {
							    return Result{ ResultType::FAILED_TO_WRITE_TO_STREAM };
						    }

						    // Add to incremental checksum calculation
						    if( !( patchedFileChecksumStream << resourceData ) )
						    {
							    return Result{ ResultType::FAILED_TO_GENERATE_CHECKSUM };
						    }
					    }

					    temporaryResourceDataStreamOut.Finish();
				    }
				    else
				    {
					    skipChecksumCheck = true;
				    }
			    }

			    if( !skipChecksumCheck )
			    {
				    // Test checksum against expected
				    std::string destinationExpectedChecksum;

				    Result getChecksumResult = resource->GetChecksum( destinationExpectedChecksum );

				    if( getChecksumResult.type != ResultType::SUCCESS )
				    {
					    return getChecksumResult;
				    }

				    std::string patchedFileChecksum;

				    if( !patchedFileChecksumStream.Retrieve( patchedFileChecksum ) )
				    {
					    return Result{ ResultType::FAILED_TO_GENERATE_CHECKSUM };
				    }

				    if( patchedFileChecksum != destinationExpectedChecksum )
				    {
					    return Result{ ResultType::UNEXPECTED_PATCH_CHECKSUM_RESULT };
				    }
			    } 
		    }

        }
    }

    // Move or copy all the patched files to the correct location
	{
		StatusSettings moveStatusSettings;
		statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 85, 5, "Moving patched files to destination.", &moveStatusSettings );

        uintmax_t totalBytesToMove = 0;
        uintmax_t bytesMoved = 0;

        for( ResourceInfo* resource : resourcesToCopy )
		{
			uintmax_t uncompressedSize;
			Result getUncompressedSizeResult = resource->GetUncompressedSize( uncompressedSize );

            if (getUncompressedSizeResult.type != ResultType::SUCCESS)
            {
				return getUncompressedSizeResult;
            }

            totalBytesToMove += uncompressedSize;
		}

		for( ResourceInfo* resource : resourcesToCopy )
		{
			StatusSettings copyStatusSettings;

            std::filesystem::path relativePath;

			if( resource->GetRelativePath( relativePath ).type != ResultType::SUCCESS )
			{
				return Result{ ResultType::FAIL };
			}

			uintmax_t uncompressedSize;
			Result getUncompressedSizeResult = resource->GetUncompressedSize( uncompressedSize );

			if( getUncompressedSizeResult.type != ResultType::SUCCESS )
			{
				return getUncompressedSizeResult;
			}

			if( moveStatusSettings.RequiresStatusUpdates() )
			{
                float ratio = static_cast<float>( 100.0 / totalBytesToMove );
				float step = static_cast<float>( ratio * uncompressedSize );
				float percentage = static_cast<float>( ratio * bytesMoved );

				std::string message = "Moving: " + relativePath.string();

				moveStatusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, percentage, step, message, &copyStatusSettings );

				bytesMoved += uncompressedSize;
			}

			std::filesystem::path resourceRelativePath;

			Result getDestinationPathRelativeResult = resource->GetDestinationPathRelative( params.resourcesToPatchDestinationSettings, resourceRelativePath );

			if( getDestinationPathRelativeResult.type != ResultType::SUCCESS )
			{
				return getDestinationPathRelativeResult;
			}

			std::filesystem::path tempResourcePath = params.temporaryFilePath / resourceRelativePath;

			if( !std::filesystem::exists( tempResourcePath ) )
			{
				return Result{ ResultType::FILE_NOT_FOUND, "Failed to find finished patched resource" };
			}

			std::filesystem::path destinationPath = params.resourcesToPatchDestinationSettings.basePath / resourceRelativePath;

            // If a move is not possible we don't want copy behaviour to be called which is standard on Windows
            // MoveFilePlatformNormalised gets all platforms to act the same way so that we can control copy
            // and provide more logging
			if( params.forceCopyPatchedFiles || !ResourceTools::MoveFilePlatformNormalised( tempResourcePath, destinationPath ) )
            {
                // Perform a copy instead
				// Open output stream
				ResourceTools::FileDataStreamOut resourceStreamOut;

				ResourcePutDataStreamParams patchedResourceResourcePutDataStreamParams;

				patchedResourceResourcePutDataStreamParams.resourceDestinationSettings = params.resourcesToPatchDestinationSettings;

				patchedResourceResourcePutDataStreamParams.dataStream = &resourceStreamOut;

				Result putResourceDataStreamResult = resource->PutDataStream( patchedResourceResourcePutDataStreamParams );

				if( putResourceDataStreamResult.type != ResultType::SUCCESS )
				{
					return putResourceDataStreamResult;
				}


				// Open input stream
				ResourceTools::FileDataStreamIn tempPatchedResourceIn( m_maxInputChunkSize.GetValue() );

				if( !tempPatchedResourceIn.StartRead( tempResourcePath ) )
				{
					return Result{ ResultType::FAILED_TO_READ_FROM_STREAM };
				}

				while( !tempPatchedResourceIn.IsFinished() )
				{
					std::string data;

                    if (copyStatusSettings.RequiresStatusUpdates())
                    {
						float step = static_cast<float>( 100.0 / uncompressedSize );
						float percentage = static_cast<float>( step * resourceStreamOut.GetFileSize() );

						std::string message = "Copying: " + relativePath.string();

						copyStatusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, percentage, step, message );
                    }

					if( !( tempPatchedResourceIn >> data ) )
					{
						return Result{ ResultType::FAILED_TO_READ_FROM_STREAM };
					}

					if( !( resourceStreamOut << data ) )
					{
						return Result{ ResultType::FAILED_TO_WRITE_TO_STREAM };
					}
				}

				resourceStreamOut.Finish();
            }
			

		}
	}
	

    {
		StatusSettings removingFilesStatusSettings;
		statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 90, 10, "Removing files.", &removingFilesStatusSettings );

        int i = 0;
		for( const auto& path : *m_removedResources.GetValue() )
		{
			auto toRemove = std::filesystem::absolute( params.resourcesToPatchDestinationSettings.basePath / path );

            if (removingFilesStatusSettings.RequiresStatusUpdates())
            {
				float step = static_cast<float>( 100.0 / m_removedResources.GetSize() );
				float percentage = static_cast<float>( step * i );
				removingFilesStatusSettings.Update( StatusProgressType::PERCENTAGE, percentage, step, toRemove.u8string() );
            }

            params.resourcesToRemove.push_back( path );

			std::error_code ec;
			if( std::filesystem::exists( toRemove ) )
			{
				bool removed = std::filesystem::remove( toRemove, ec );
				if( !removed )
				{
					statusSettings.Update( StatusProgressType::WARNING, 0, 0, "Failed to remove file " + toRemove.u8string() );
				}
			}

			// Remove any empty directories left over
			toRemove = toRemove.parent_path();
			while( std::filesystem::is_directory( toRemove ) && std::filesystem::is_empty( toRemove ) )
			{
				bool removed = std::filesystem::remove( toRemove, ec );
				if( !removed )
				{
					statusSettings.Update( StatusProgressType::WARNING, 0, 0, "Failed to remove empty directory " + toRemove.u8string() );
				}
			}
		}
    }

	return Result{ ResultType::SUCCESS };
}

Result PatchResourceGroup::PatchResourceGroupImpl::SetRemovedResourceRelativePaths( const std::vector<std::filesystem::path>& paths )
{
	for( auto path : paths )
	{
		m_removedResources.PushBack( path );
	}
	return Result{ ResultType::SUCCESS };
}

Result PatchResourceGroup::PatchResourceGroupImpl::GetGroupSpecificResourcesToBundle( std::vector<ResourceInfo*>& toBundle ) const
{
	if( m_resourceGroupParameter.HasValue() )
	{
		toBundle.emplace_back( m_resourceGroupParameter.GetValue() );
	}

	return Result{ ResultType::SUCCESS };
}

}