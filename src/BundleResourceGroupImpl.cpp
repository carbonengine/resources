#include "BundleResourceGroupImpl.h"

#include <yaml-cpp/yaml.h>

#include <ResourceTools.h>

#include <BundleStreamIn.h>

#include <FileDataStreamOut.h>

#include <Md5ChecksumStream.h>

namespace CarbonResources
{

    BundleResourceGroupImpl::BundleResourceGroupImpl( ) :
	    ResourceGroupImpl()
    {
		m_chunkSize = 1000;

        m_resourceGroupParameter = new ResourceGroupInfo( {} );

		m_type = TypeId();
    }

    BundleResourceGroupImpl::~BundleResourceGroupImpl()
    {
		delete m_resourceGroupParameter.GetValue();
    }

    void BundleResourceGroupImpl::SetChunkSize( uintmax_t size )
    {
		m_chunkSize = size;
    }

    Result BundleResourceGroupImpl::SetResourceGroup( const ResourceGroupInfo& resourceGroup )
    {
		// Creates a deep copy
		return m_resourceGroupParameter.GetValue()->SetParametersFromResource( &resourceGroup, m_versionParameter.GetValue() );
    }

    Result BundleResourceGroupImpl::Unpack( const BundleUnpackParams& params )
    {
		if( params.statusCallback )
		{
			params.statusCallback( CarbonResources::STATUS_LEVEL::PROCEDURE, CarbonResources::STATUS_PROGRESS_TYPE::PERCENTAGE, 0, "Unpacking" );
		}

		ResourceGroupInfo* resourceGroupResource = m_resourceGroupParameter.GetValue();


		// Load the resourceGroup from the resourceGroupResource
		std::string resourceGroupData;

		ResourceGetDataParams resourceGroupDataParams;

		resourceGroupDataParams.resourceSourceSettings = params.chunkSourceSettings;

		resourceGroupDataParams.data = &resourceGroupData;

		Result resourceGroupGetDataResult = m_resourceGroupParameter.GetValue()->GetData( resourceGroupDataParams );

		if( resourceGroupGetDataResult.type != ResultType::SUCCESS )
		{
			return resourceGroupGetDataResult;
		}

		ResourceGroupImpl resourceGroup;

		Result resourceGroupImportFromDataResult = resourceGroup.ImportFromData( resourceGroupData );

		if( resourceGroupImportFromDataResult.type != ResultType::SUCCESS )
		{
			return resourceGroupImportFromDataResult;
		}

        // Create stream
		ResourceTools::BundleStreamIn bundleStream(m_chunkSize.GetValue());

        auto chunkIterator = m_resourcesParameter.begin();

        if( params.statusCallback )
		{
			params.statusCallback( CarbonResources::STATUS_LEVEL::PROCEDURE, CarbonResources::STATUS_PROGRESS_TYPE::PERCENTAGE, 20, "Rebuilding resources." );
		}

        // Reconstitute the resources in the bundle
		auto numResources = resourceGroup.GetSize();
		int numProcessed = 0;
        
        for( ResourceInfo* resource : resourceGroup )
		{
			if( params.statusCallback )
			{
				std::filesystem::path relativePath;

                if (resource->GetRelativePath(relativePath).type != ResultType::SUCCESS)
                {
					return Result{ ResultType::FAIL };
                }

                float percentage = ( 100.0 / numResources ) * numProcessed;

                std::string message = "Rebuilding: " + relativePath.string();

				params.statusCallback( CarbonResources::STATUS_LEVEL::DETAIL, CarbonResources::STATUS_PROGRESS_TYPE::PERCENTAGE, percentage, message );

                numProcessed++;
			}

            uintmax_t resourceFileUncompressedSize;

            Result getUncompressedDataSizeResult = resource->GetUncompressedSize( resourceFileUncompressedSize );

            if (getUncompressedDataSizeResult.type != ResultType::SUCCESS)
            {
				return getUncompressedDataSizeResult;
            }


            ResourceTools::FileDataStreamOut resourceDataStreamOut;

            ResourcePutDataStreamParams resourcePutDataStreamParams;

            resourcePutDataStreamParams.resourceDestinationSettings = params.resourceDestinationSettings;

            resourcePutDataStreamParams.dataStream = &resourceDataStreamOut;

            Result resourcePutDataStreamResult = resource->PutDataStream( resourcePutDataStreamParams );

            if (resourcePutDataStreamResult.type != ResultType::SUCCESS)
            {
				return resourcePutDataStreamResult;
            }

			ResourceTools::GetFile file;

            file.fileSize = resourceFileUncompressedSize;

            // Calculate checksum while processing chunks
			ResourceTools::Md5ChecksumStream resourceChecksumStream;

            while( resourceDataStreamOut.GetFileSize() < resourceFileUncompressedSize )
            {
                if (chunkIterator != m_resourcesParameter.end())
                {
					ResourceInfo* chunk = ( *chunkIterator );

					// Get chunk data
					std::string chunkData;

					ResourceGetDataParams resourceGetDataParams;

					resourceGetDataParams.resourceSourceSettings = params.chunkSourceSettings;

					resourceGetDataParams.data = &chunkData;

					Result getChunkDataResult = chunk->GetData( resourceGetDataParams );

					if( getChunkDataResult.type != ResultType::SUCCESS )
					{
						return getChunkDataResult;
					}

					// Add to chunk stream
					if( !( bundleStream << chunkData ) )
					{
						return Result{ ResultType::FAIL }; //TODO make more descriptive
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

                if( !( resourceChecksumStream << resourceChunkData ) )
                {
					return Result{ ResultType::FAILED_TO_GENERATE_CHECKSUM };
                }

                if( !( resourceDataStreamOut << resourceChunkData ) )
                {
					return Result{ ResultType::FAILED_TO_SAVE_TO_STREAM };
                }

                if( chunkIterator != m_resourcesParameter.end() )
				{
					chunkIterator++;
				}
            }

            // Validate the resource data
            // TODO: perhaps make this optional
			std::string recreatedResourceChecksum;

            if (!resourceChecksumStream.FinishAndRetrieve(recreatedResourceChecksum))
            {
				return Result{ ResultType::FAILED_TO_GENERATE_CHECKSUM };
            }

           
            std::string resourceChecksum;

            Result getChecksumResult = resource->GetChecksum( resourceChecksum );

            if (getChecksumResult.type != ResultType::SUCCESS)
            {
				return getChecksumResult;
            }

            if (recreatedResourceChecksum != resourceChecksum)
            {
				return Result{ ResultType::UNEXPECTED_CHUNK_CHECKSUM_RESULT };
            }

            /*
			// Export data TODO
			ResourcePutDataParams resourcePutDataParams;

			resourcePutDataParams.resourceDestinationSettings = params.resourceDestinationSettings;

			resourcePutDataParams.data = &resourceData;

			Result putResourceDataResult = resource->PutData( resourcePutDataParams );

			if( putResourceDataResult != Result::SUCCESS )
			{
				return putResourceDataResult;
			}
            */

		}

        return Result{ ResultType::SUCCESS };

    }

    std::string BundleResourceGroupImpl::GetType() const
	{
		return TypeId();
	}

    std::string BundleResourceGroupImpl::TypeId()
	{
		return "BundleGroup";
	}

    Result BundleResourceGroupImpl::CreateResourceFromYaml( YAML::Node& resource, ResourceInfo*& resourceOut )
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

    Result BundleResourceGroupImpl::ImportGroupSpecialisedYaml( YAML::Node& resourceGroupFile )
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

			//TODO ensure that resource is of base type ResourceGroup

			m_resourceGroupParameter = reinterpret_cast<ResourceGroupInfo*>( resource );
		}

        if (m_chunkSize.IsParameterExpectedInDocumentVersion(m_versionParameter.GetValue()))
        {
            // TODO handle failure
			m_chunkSize = resourceGroupFile[m_chunkSize.GetTag()].as<uintmax_t>();
        }

		return Result{ ResultType::SUCCESS };
    }

    Result BundleResourceGroupImpl::ExportGroupSpecialisedYaml( YAML::Emitter& out, VersionInternal outputDocumentVersion ) const
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