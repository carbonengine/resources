#include "PatchResourceGroupImpl.h"

#include "Patching.h"

#include <yaml-cpp/yaml.h>

#include <ResourceTools.h>

#include <ScopedFile.h>

#include <FileDataStreamIn.h>

#include <FileDataStreamOut.h>

#include <Md5ChecksumStream.h>

namespace CarbonResources
{
    PatchResourceGroupImpl::PatchResourceGroupImpl( ) :
	    ResourceGroupImpl()
    {
		m_resourceGroupParameter = new ResourceGroupInfo({});

		m_type = TypeId();
    }

    Result PatchResourceGroupImpl::SetResourceGroup( const ResourceGroupInfo& resourceGroup )
    {
        // Creates a deep copy
		return m_resourceGroupParameter.GetValue()->SetParametersFromResource( &resourceGroup );
    }

    void PatchResourceGroupImpl::SetMaxInputChunkSize( unsigned long maxInputChunkSize )
    {
		m_maxInputChunkSize = maxInputChunkSize;
    }

    PatchResourceGroupImpl::~PatchResourceGroupImpl()
    {
		delete m_resourceGroupParameter.GetValue();
    }

    std::string PatchResourceGroupImpl::GetType() const
	{
		return TypeId();
	}

    std::string PatchResourceGroupImpl::TypeId()
    {
		return "PatchGroup";
    }

	Result PatchResourceGroupImpl::CreateResourceFromYaml( YAML::Node& resource, ResourceInfo*& resourceOut )
	{
		PatchResourceInfo* patchResource = new PatchResourceInfo( PatchResourceInfoParams{} );

		Result importFromYamlResult = patchResource->ImportFromYaml( resource, m_versionParameter.GetValue() );

		if( importFromYamlResult != Result::SUCCESS )
		{
			delete patchResource;

			return importFromYamlResult;
		}
		else
		{
			resourceOut = patchResource;

			return Result::SUCCESS;
		}

	}

    Result PatchResourceGroupImpl::ImportGroupSpecialisedYaml( YAML::Node& resourceGroupFile )
    {
		if( m_resourceGroupParameter.IsParameterExpectedInDocumentVersion( m_versionParameter.GetValue() ) )
		{
			YAML::Node resourceGroupNode = resourceGroupFile[m_resourceGroupParameter.GetTag()];

            ResourceInfo* resource = nullptr;

            Result createResourceFromYaml = ResourceGroupImpl::CreateResourceFromYaml( resourceGroupNode, resource );

            if (createResourceFromYaml != Result::SUCCESS)
            {
				return createResourceFromYaml;
            }

            //TODO ensure that resource is of base type ResourceGroup

			m_resourceGroupParameter = reinterpret_cast<ResourceGroupInfo*>( resource );
		}


        if( m_maxInputChunkSize.IsParameterExpectedInDocumentVersion( m_versionParameter.GetValue() ) )
		{
			if( YAML::Node parameter = resourceGroupFile[m_maxInputChunkSize.GetTag()] )
			{
				m_maxInputChunkSize = parameter.as<unsigned long>();
			}
			else
			{
				return Result::MALFORMED_RESOURCE_INPUT;
			}
		}

		return Result::SUCCESS;
    }

    Result PatchResourceGroupImpl::ExportGroupSpecialisedYaml( YAML::Emitter& out, Version outputDocumentVersion ) const
    {
        if (m_resourceGroupParameter.IsParameterExpectedInDocumentVersion(outputDocumentVersion))
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
				return Result::REQUIRED_RESOURCE_PARAMETER_NOT_SET;
			}

			out << YAML::Key << m_maxInputChunkSize.GetTag();
			out << YAML::Value << m_maxInputChunkSize.GetValue();
		}

		return Result::SUCCESS;
    }


    Result PatchResourceGroupImpl::GetTargetResourcePatches( const ResourceInfo* resource, std::vector<const PatchResourceInfo*>& patches ) const
    {
		std::filesystem::path resourceRelativePath;

		Result resourceRelativePathResult = resource->GetRelativePath( resourceRelativePath );

        if( resourceRelativePathResult != Result::SUCCESS )
        {
			return resourceRelativePathResult;
        }

        for (ResourceInfo* patchResource : m_resourcesParameter)
        {
			PatchResourceInfo* patch = reinterpret_cast<PatchResourceInfo*>( patchResource );

			std::filesystem::path patchTargetResource;

			Result getPatchTargetResource = patch->GetTargetResourceRelativePath( patchTargetResource );

			if( getPatchTargetResource != Result::SUCCESS )
			{
				return getPatchTargetResource;
			}

            if (resourceRelativePath == patchTargetResource)
            {
				patches.push_back( patch );
            }
        }

        return Result::SUCCESS;
    }

    Result PatchResourceGroupImpl::Apply( const PatchApplyParams& params )
    {
        // Will be removed when falls out of scope
		ResourceTools::ScopedFile temporaryFileScope( params.temporaryFilePath );

		ResourceGroupInfo* resourceGroupResource = m_resourceGroupParameter.GetValue();

        
        // Load the resourceGroup from the resourceGroupResource
		std::string resourceGroupData;

        ResourceGetDataParams resourceGroupDataParams;

        resourceGroupDataParams.resourceSourceSettings = params.patchBinarySourceSettings;

        resourceGroupDataParams.data = &resourceGroupData;

        Result resourceGroupGetDataResult = m_resourceGroupParameter.GetValue()->GetData( resourceGroupDataParams );

        if (resourceGroupGetDataResult != Result::SUCCESS)
        {
			return resourceGroupGetDataResult;
        }

        ResourceGroupImpl resourceGroup;



        Result resourceGroupImportFromDataResult = resourceGroup.ImportFromData( resourceGroupData );

        if( resourceGroupImportFromDataResult != Result::SUCCESS )
        {
			return resourceGroupImportFromDataResult;
        }

        
        for( ResourceInfo* resource : resourceGroup.m_resourcesParameter )
        {
            // See if there is a patch available for resource
            std::vector<const PatchResourceInfo*> patchesForResource; 

            Result getTargetResourcePatchesResult = GetTargetResourcePatches(resource, patchesForResource);

            if (getTargetResourcePatchesResult != Result::SUCCESS)
            {
				return getTargetResourcePatchesResult;
            }


            // Open a stream to write a temp file of the patched resource
			ResourceTools::FileDataStreamOut temporaryResourceDataStreamOut;

			if( !temporaryResourceDataStreamOut.StartWrite( params.temporaryFilePath ) )
			{
				return Result::FAILED_TO_OPEN_FILE;
			}

			// Incrementally calculate checksum for temporary patch file
			ResourceTools::Md5ChecksumStream patchedFileChecksumStream;



			if( patchesForResource.size() > 0 )
			{
                // Open stream for resource
				ResourceTools::FileDataStreamIn resourceDataStreamIn( m_maxInputChunkSize.GetValue() );

				ResourceGetDataStreamParams resourceDataStreamParams;

                resourceDataStreamParams.resourceSourceSettings = params.resourcesToPatchSourceSettings;

                resourceDataStreamParams.dataStream = &resourceDataStreamIn;

				Result getResourceDataStream = resource->GetDataStream( resourceDataStreamParams );

                if (getResourceDataStream != Result::SUCCESS)
                {
					return getResourceDataStream;
                }


				for( auto patchIter = patchesForResource.begin(); patchIter != patchesForResource.end(); patchIter++ )
                {

				    const PatchResourceInfo* patch = (*patchIter );

				    // Patch found, Retreive and apply
				    std::string patchData;

				    ResourceGetDataParams patchGetDataParams;

				    patchGetDataParams.resourceSourceSettings = params.patchBinarySourceSettings;

                    patchGetDataParams.data = &patchData;
  
                    Result getPatchDataResult = patch->GetData( patchGetDataParams );

				    if( getPatchDataResult != Result::SUCCESS )
				    {
					    return getPatchDataResult;
				    }

                    // Get previous data
					unsigned long dataOffset;

					Result getPatchDataOffset = patch->GetDataOffset( dataOffset );

                    if (getPatchDataOffset != Result::SUCCESS)
                    {
						return getPatchDataOffset;
                    }

				    std::string previousResourceData;

                    // Get previous size of resource
					unsigned long previousUncompressedSize;

					Result getPreviousUncompressedSize = resource->GetUncompressedSize( previousUncompressedSize );

                    if (getPreviousUncompressedSize != Result::SUCCESS)
                    {
						return getPreviousUncompressedSize;
                    }

                    if (dataOffset < previousUncompressedSize)
                    {

                        // Get to location of patch
                        while (resourceDataStreamIn.GetCurrentPosition() < dataOffset)
                        {
							std::string dataChunk;

                            if (!(resourceDataStreamIn >> dataChunk))
                            {
								return Result::FAILED_TO_READ_FROM_STREAM;
                            }

                            if( !( temporaryResourceDataStreamOut << dataChunk ) )
                            {
								return Result::FAILED_TO_WRITE_TO_STREAM;
                            }

                            // Add to incremental checksum calculation
							if( !( patchedFileChecksumStream << dataChunk ) )
                            {
								return Result::FAILED_TO_GENERATE_CHECKSUM;
                            }

                        }

                        // Apply patch to data
						if( !( resourceDataStreamIn >> previousResourceData ) )
						{
							return Result::FAILED_TO_READ_FROM_STREAM;
						}

						// Apply the patch to the previous data
						std::string patchedResourceData;

						if( !ResourceTools::ApplyPatch( previousResourceData, patchData, patchedResourceData ) )
						{
							return Result::FAILED_TO_APPLY_PATCH;
						}

                        // Write the patch result to file
						if( !( temporaryResourceDataStreamOut << patchedResourceData ) )
                        {
							return Result::FAILED_TO_WRITE_TO_STREAM;
                        }

                        // Add to incremental checksum calculation
						if( !( patchedFileChecksumStream << patchedResourceData ) )
						{
							return Result::FAILED_TO_GENERATE_CHECKSUM;
						}

                    }
                    else
                    {
                        // New data, append on to end
						if( !( temporaryResourceDataStreamOut << previousResourceData ) )
						{
							return Result::FAILED_TO_WRITE_TO_STREAM;
						}

                        // Add to incremental checksum calculation
						if( !( patchedFileChecksumStream << previousResourceData ) )
						{
							return Result::FAILED_TO_GENERATE_CHECKSUM;
						}

                    }

				}

                temporaryResourceDataStreamOut.Finish();

			}
            else
            {
                // No Patch found, indicates this is just a new file
                // Just replace file directly
                ResourceTools::FileDataStreamIn resourceStreamIn(m_maxInputChunkSize.GetValue());

				ResourceGetDataStreamParams resourceGetDataParams;

                resourceGetDataParams.resourceSourceSettings = params.newBuildResourcesSourceSettings;

                resourceGetDataParams.dataStream = &resourceStreamIn;

				Result resourceGetDataResult = resource->GetDataStream( resourceGetDataParams );

                if (resourceGetDataResult != Result::SUCCESS)
                {
					return resourceGetDataResult;
                }

                while (!resourceStreamIn.IsFinished())
                {
					std::string resourceData;

                    if (!(resourceStreamIn >> resourceData))
                    {
						return Result::FAILED_TO_READ_FROM_STREAM;
                    }

                    if (!(temporaryResourceDataStreamOut << resourceData))
                    {
						return Result::FAILED_TO_WRITE_TO_STREAM;
                    }

                    // Add to incremental checksum calculation
					if( !( patchedFileChecksumStream << resourceData ) )
					{
						return Result::FAILED_TO_GENERATE_CHECKSUM;
					}
                }

                temporaryResourceDataStreamOut.Finish();

            }


            // Test checksum against expected
			std::string destinationExpectedChecksum;

			Result getChecksumResult = resource->GetChecksum( destinationExpectedChecksum );

			if( getChecksumResult != Result::SUCCESS )
			{
				return getChecksumResult;
			}

			std::string patchedFileChecksum;

			if( !patchedFileChecksumStream.FinishAndRetrieve( patchedFileChecksum ) )
			{
				return Result::FAILED_TO_GENERATE_CHECKSUM;
			}

			if( patchedFileChecksum != destinationExpectedChecksum )
			{
				return Result::UNEXPECTED_PATCH_CHECKSUM_RESULT;
			}


			// Copy temp file to replace the old resource file

            // Open output stream
			ResourceTools::FileDataStreamOut resourceStreamOut;

			ResourcePutDataStreamParams patchedResourceResourcePutDataStreamParams;

			patchedResourceResourcePutDataStreamParams.resourceDestinationSettings = params.resourcesToPatchDestinationSettings;

			patchedResourceResourcePutDataStreamParams.dataStream = &resourceStreamOut;

			Result putResourceDataStreamResult = resource->PutDataStream( patchedResourceResourcePutDataStreamParams );

			if( putResourceDataStreamResult != Result::SUCCESS )
			{
				return putResourceDataStreamResult;
			}


			// Open input stream
			ResourceTools::FileDataStreamIn tempPatchedResourceIn(m_maxInputChunkSize.GetValue());

            if (!tempPatchedResourceIn.StartRead(params.temporaryFilePath))
            {
				return Result::FAILED_TO_READ_FROM_STREAM;
            }

            while (!tempPatchedResourceIn.IsFinished())
            {
				std::string data;

                if (!(tempPatchedResourceIn >> data))
                {
					return Result::FAILED_TO_READ_FROM_STREAM;
                }

                if (!(resourceStreamOut << data))
                {
					return Result::FAILED_TO_WRITE_TO_STREAM;
                }
            }

            resourceStreamOut.Finish();
        }

        return Result::SUCCESS;


    }

}