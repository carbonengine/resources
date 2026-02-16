// Copyright © 2025 CCP ehf.

#include "BundleResourceGroupImpl.h"

#include <yaml-cpp/yaml.h>

#include <ResourceTools.h>

#include <BundleStreamIn.h>

#include <FileDataStreamOut.h>

#include <Md5ChecksumStream.h>

#include "ResourceGroupFactory.h"

#include "PatchResourceGroupImpl.h"

namespace CarbonResources
{

BundleResourceGroup::BundleResourceGroupImpl::BundleResourceGroupImpl() :
	ResourceGroup::ResourceGroupImpl()
{
	m_chunkSize = 1000;

	m_resourceGroupParameter = new ResourceGroupInfo( {} );

	m_type = TypeId();
}

BundleResourceGroup::BundleResourceGroupImpl::~BundleResourceGroupImpl()
{
	delete m_resourceGroupParameter.GetValue();
}

Result BundleResourceGroup::BundleResourceGroupImpl::SetChunkSize( uintmax_t size )
{
	if( size <= 0 )
	{
		return Result{ ResultType::INVALID_CHUNK_SIZE };
	}

	m_chunkSize = size;

	return Result{ ResultType::SUCCESS };
}

Result BundleResourceGroup::BundleResourceGroupImpl::SetResourceGroup( const ResourceGroupInfo& resourceGroup )
{
	// Creates a deep copy
	return m_resourceGroupParameter.GetValue()->SetParametersFromResource( &resourceGroup, m_versionParameter.GetValue() );
}

Result BundleResourceGroup::BundleResourceGroupImpl::Unpack( const BundleUnpackParams& params, StatusSettings& statusSettings )
{
	statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 0, 20, "Rebuilding resources." );

	ResourceGroupInfo* resourceGroupResource = m_resourceGroupParameter.GetValue();


	// Load the resourceGroup from the resourceGroupResource
	std::string resourceGroupData;

	ResourceGetDataParams resourceGroupDataParams;

	resourceGroupDataParams.resourceSourceSettings = params.chunkSourceSettings;

	resourceGroupDataParams.data = &resourceGroupData;

	Result getChecksumResult = resourceGroupResource->GetChecksum( resourceGroupDataParams.expectedChecksum );

	if( getChecksumResult.type != ResultType::SUCCESS )
	{
		return getChecksumResult;
	}

	Result resourceGroupGetDataResult = m_resourceGroupParameter.GetValue()->GetData( resourceGroupDataParams );

	if( resourceGroupGetDataResult.type != ResultType::SUCCESS )
	{
		return resourceGroupGetDataResult;
	}

    std::shared_ptr<ResourceGroupImpl> resourceGroup;

    {
		StatusSettings createResourceGroupFromYamlStatusSettings;
		statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 20, 20, "Rebuilding resources.", &createResourceGroupFromYamlStatusSettings );

		
		Result createResult = CreateResourceGroupFromYamlString( resourceGroupData, resourceGroup, createResourceGroupFromYamlStatusSettings );
		if( createResult.type != ResultType::SUCCESS )
		{
			std::stringstream ss;
			ss << "Failed to import resource group data from the following paths:";
			for( auto path : resourceGroupDataParams.resourceSourceSettings.basePaths )
			{
				ss << " \"" << path.string() << "\"";
			}
			createResult.info = ss.str();
			return createResult;
		}
    }
    

	// Create stream
	ResourceTools::BundleStreamIn bundleStream( m_chunkSize.GetValue() );

	auto chunkIterator = m_resourcesParameter.begin();

	// Reconstitute the resources in the bundle
	auto numResources = resourceGroup->GetSize();
	int numProcessed = 0;

	std::vector<ResourceInfo*> toBundle;

	std::copy( resourceGroup->begin(), resourceGroup->end(), std::back_inserter( toBundle ) );

	Result getGroupSpecificResourcesToBundleResult = resourceGroup->GetGroupSpecificResourcesToBundle( toBundle );

	if( getGroupSpecificResourcesToBundleResult.type != ResultType::SUCCESS )
	{
		return getGroupSpecificResourcesToBundleResult;
	}

    {
		StatusSettings innerStatusUpdate;
		statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 40, 40, "Rebuilding resources.", &innerStatusUpdate );

		for( ResourceInfo* resource : toBundle )
		{
			std::string location;

			Result getLocationResult = resource->GetLocation( location );

			if( getLocationResult.type != ResultType::SUCCESS )
			{
				return getLocationResult;
			}

			// Only process if required for the process
			if( innerStatusUpdate.RequiresStatusUpdates() )
			{
				std::filesystem::path relativePath;

				if( resource->GetRelativePath( relativePath ).type != ResultType::SUCCESS )
				{
					return Result{ ResultType::FAIL };
				}

				std::string message;

				if( location.empty() )
				{
					message = "Nothing to rebuild: " + relativePath.string();
				}
				else
				{
					message = "Rebuilding: " + relativePath.string();
				}

				float step = static_cast<float>( 100 * numProcessed );
				float percentage = static_cast<float>( step / toBundle.size() );

				innerStatusUpdate.Update( CarbonResources::StatusProgressType::PERCENTAGE, percentage, step, message );

				numProcessed++;
			}

			if( location.empty() )
			{
				continue;
			}

			uintmax_t resourceFileUncompressedSize;

			Result getUncompressedDataSizeResult = resource->GetUncompressedSize( resourceFileUncompressedSize );

			if( getUncompressedDataSizeResult.type != ResultType::SUCCESS )
			{
				return getUncompressedDataSizeResult;
			}


			ResourceTools::FileDataStreamOut resourceDataStreamOut;

			ResourcePutDataStreamParams resourcePutDataStreamParams;

			resourcePutDataStreamParams.resourceDestinationSettings = params.resourceDestinationSettings;

			resourcePutDataStreamParams.dataStream = &resourceDataStreamOut;

			Result resourcePutDataStreamResult = resource->PutDataStream( resourcePutDataStreamParams );

			if( resourcePutDataStreamResult.type != ResultType::SUCCESS )
			{
				return resourcePutDataStreamResult;
			}

			ResourceTools::GetFile file;

			file.fileSize = resourceFileUncompressedSize;

            // Calculate checksum while processing chunks
            ResourceTools::Md5ChecksumStream resourceChecksumStream;

            while (resourceDataStreamOut.GetFileSize() < resourceFileUncompressedSize)
            {
                if (chunkIterator != m_resourcesParameter.end())
                {
                    ResourceInfo* chunk = (*chunkIterator);

                    // Get chunk data
                    std::string chunkData;

                    ResourceGetDataParams resourceGetDataParams;

                    resourceGetDataParams.resourceSourceSettings = params.chunkSourceSettings;

                    resourceGetDataParams.data = &chunkData;

                    Result getChunkChecksumResult = chunk->GetChecksum(resourceGetDataParams.expectedChecksum);

                    if (getChunkChecksumResult.type != ResultType::SUCCESS)
                    {
                        return getChunkChecksumResult;
                    }

                    Result getChunkDataResult = chunk->GetData(resourceGetDataParams);

                    if (getChunkDataResult.type != ResultType::SUCCESS)
                    {
                        return getChunkDataResult;
                    }

                    // Add to chunk stream
                    if (!(bundleStream << chunkData))
                    {
                        return Result{ ResultType::FAIL };
                    }
                }
                else
                {
                    if (bundleStream.GetCacheSize() == 0)
                    {
                        return Result{ ResultType::UNEXPECTED_END_OF_CHUNKS };
                    }
                }


                std::string resourceChunkData;

                file.data = &resourceChunkData;

                // Retreive chunk from stream
                // This ensures that we only get the data expected
                // for this resource, extra is cached for next resource
                if (!(bundleStream >> file))
                {
                    return Result{ ResultType::FAILED_TO_RETRIEVE_CHUNK_DATA };
                }

                if (!(resourceChecksumStream << resourceChunkData))
                {
                    return Result{ ResultType::FAILED_TO_GENERATE_CHECKSUM };
                }

                if (!(resourceDataStreamOut << resourceChunkData))
                {
                    return Result{ ResultType::FAILED_TO_SAVE_TO_STREAM };
                }

                if (chunkIterator != m_resourcesParameter.end())
                {
                    chunkIterator++;
                }
            }

            // Validate the resource data
            std::string recreatedResourceChecksum;

            if (!resourceChecksumStream.FinishAndRetrieve(recreatedResourceChecksum))
            {
                return Result{ ResultType::FAILED_TO_GENERATE_CHECKSUM };
            }


            std::string resourceChecksum;

            Result getChecksumResult = resource->GetChecksum(resourceChecksum);

            if (getChecksumResult.type != ResultType::SUCCESS)
            {
                return getChecksumResult;
            }

            if (recreatedResourceChecksum != resourceChecksum)
            {
                return Result{ ResultType::UNEXPECTED_CHUNK_CHECKSUM_RESULT };
            }
        }
    }

    {
		StatusSettings exportStatusSettings;
		statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 80, 20, "Exporting data.", &exportStatusSettings );

		// Export the resource group file.
		ResourceGroupExportToFileParams exportParams;
		std::filesystem::path resourceGroupRelativePath;
		Result getResourceGroupRelativePathResult = resourceGroupResource->GetRelativePath( resourceGroupRelativePath );
		if( getResourceGroupRelativePathResult.type != ResultType::SUCCESS )
		{
			return getResourceGroupRelativePathResult;
		}
		exportParams.filename = params.resourceDestinationSettings.basePath / resourceGroupRelativePath;

		Result exportResult = resourceGroup->ExportToFile( exportParams, exportStatusSettings );
		if( exportResult.type != ResultType::SUCCESS )
		{
			return exportResult;
		}
    }

	return Result{ ResultType::SUCCESS };
}

std::string BundleResourceGroup::BundleResourceGroupImpl::GetType() const
{
	return TypeId();
}

std::string BundleResourceGroup::BundleResourceGroupImpl::TypeId()
{
	return "BundleGroup";
}

Result BundleResourceGroup::BundleResourceGroupImpl::CreateResourceFromYaml( YAML::Node& resource, ResourceInfo*& resourceOut )
{
	BundleResourceInfo* bundleResourceInfo = new BundleResourceInfo( BundleResourceInfoParams{} );

	Result importFromYamlResult = bundleResourceInfo->ImportFromYaml( resource, m_versionParameter.GetValue() );

	if( importFromYamlResult.type != ResultType::SUCCESS )
	{
		delete bundleResourceInfo;

		return importFromYamlResult;
	}
	else
	{
		resourceOut = bundleResourceInfo;

		return Result{ ResultType::SUCCESS };
	}
}

Result BundleResourceGroup::BundleResourceGroupImpl::ImportGroupSpecialisedYaml( YAML::Node& resourceGroupFile )
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

	if( m_chunkSize.IsParameterExpectedInDocumentVersion( m_versionParameter.GetValue() ) )
	{

		if( YAML::Node chunkSize = resourceGroupFile[m_chunkSize.GetTag()] )
		{
			m_chunkSize = chunkSize.as<uintmax_t>();
		}
		else
		{
			return Result{ ResultType::MALFORMED_RESOURCE_GROUP };
		}
	}

	return Result{ ResultType::SUCCESS };
}

Result BundleResourceGroup::BundleResourceGroupImpl::ExportGroupSpecialisedYaml( YAML::Emitter& out, VersionInternal outputDocumentVersion ) const
{
	if( m_resourceGroupParameter.IsParameterExpectedInDocumentVersion( outputDocumentVersion ) )
	{
		out << YAML::Key << m_resourceGroupParameter.GetTag();

		out << YAML::Value << YAML::BeginMap;

		m_resourceGroupParameter.GetValue()->ExportToYaml( out, outputDocumentVersion );

		out << YAML::EndMap;
	}

	if( m_chunkSize.IsParameterExpectedInDocumentVersion( outputDocumentVersion ) )
	{
		out << YAML::Key << m_chunkSize.GetTag();

		out << YAML::Value << m_chunkSize.GetValue();
	}

	return Result{ ResultType::SUCCESS };
}

}