#include "BundleResourceGroupImpl.h"

//#include "BundleResource.h"

#include <yaml-cpp/yaml.h>

#include <ResourceTools.h>

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

    void BundleResourceGroupImpl::SetChunkSize( unsigned long size )
    {
		m_chunkSize = size;
    }

    Result BundleResourceGroupImpl::SetResourceGroup( const ResourceGroupInfo& resourceGroup )
    {
		// Creates a deep copy
		return m_resourceGroupParameter.GetValue()->SetParametersFromResource( &resourceGroup );
    }

    Result BundleResourceGroupImpl::Unpack( const BundleUnpackParams& params )
    {
		ResourceGroupInfo* resourceGroupResource = m_resourceGroupParameter.GetValue();


		// Load the resourceGroup from the resourceGroupResource
		std::string resourceGroupData;

		ResourceGetDataParams resourceGroupDataParams;

		resourceGroupDataParams.resourceSourceSettings = params.chunkSourceSettings;

		resourceGroupDataParams.data = &resourceGroupData;

		Result resourceGroupGetDataResult = m_resourceGroupParameter.GetValue()->GetData( resourceGroupDataParams );

		if( resourceGroupGetDataResult != Result::SUCCESS )
		{
			return resourceGroupGetDataResult;
		}

		ResourceGroupImpl resourceGroup;

		Result resourceGroupImportFromDataResult = resourceGroup.ImportFromData( resourceGroupData );

		if( resourceGroupImportFromDataResult != Result::SUCCESS )
		{
			return resourceGroupImportFromDataResult;
		}


        // Create stream
		ResourceTools::ChunkStream chunkStream(m_chunkSize.GetValue());

        auto chunkIterator = m_resourcesParameter.begin();

        // Reconstitute the resources in the bundle
        for( ResourceInfo* resource : resourceGroup.m_resourcesParameter )
		{

            // Add chunks to chunkStream until enough have been reconstituted to produce resource
			std::string resourceData;

			ResourceTools::GetFile resourceFile;

            resourceFile.data = &resourceData;

            Result getUncompressedDataSizeResult = resource->GetUncompressedSize( resourceFile.fileSize );

            if (getUncompressedDataSizeResult != Result::SUCCESS)
            {
				return getUncompressedDataSizeResult;
            }

            while( !( chunkStream >> resourceFile ) )
            {
                if (chunkIterator == m_resourcesParameter.end())
                {
					return Result::UNEXPECTED_END_OF_CHUNKS;
                }

				ResourceInfo* chunk = ( *chunkIterator );

                // Get chunk data
				std::string chunkData;

				ResourceGetDataParams resourceGetDataParams;

				resourceGetDataParams.resourceSourceSettings = params.chunkSourceSettings;

				resourceGetDataParams.data = &chunkData;

				Result getChunkDataResult = chunk->GetData( resourceGetDataParams );

				if( getChunkDataResult != Result::SUCCESS )
				{
					return getChunkDataResult;
				}

                chunkStream << chunkData;

                chunkIterator++;
            }

            // Validate the resource data
            // TODO: perhaps make this optional
			std::string recreatedResourceChecksum;

            if( !ResourceTools::GenerateMd5Checksum( resourceData, recreatedResourceChecksum ) )
            {
				return Result::FAILED_TO_GENERATE_CHECKSUM;
            }

            std::string resourceChecksum;

            Result getChecksumResult = resource->GetChecksum( resourceChecksum );

            if (getChecksumResult != Result::SUCCESS)
            {
				return getChecksumResult;
            }

            if (recreatedResourceChecksum != resourceChecksum)
            {
				return Result::UNEXPECTED_CHUNK_CHECKSUM_RESULT;
            }

			// Export data
			ResourcePutDataParams resourcePutDataParams;

			resourcePutDataParams.resourceDestinationSettings = params.resourceDestinationSettings;

			resourcePutDataParams.data = &resourceData;

			Result putResourceDataResult = resource->PutData( resourcePutDataParams );

			if( putResourceDataResult != Result::SUCCESS )
			{
				return putResourceDataResult;
			}

		}

        return Result::SUCCESS;

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

		if( importFromYamlResult != Result::SUCCESS )
		{
			delete bundleResourceInfo;

			return importFromYamlResult;
		}
		else
		{
			resourceOut = bundleResourceInfo;

			return Result::SUCCESS;
		}

	}

    Result BundleResourceGroupImpl::ImportGroupSpecialisedYaml( YAML::Node& resourceGroupFile )
    {
		if( m_resourceGroupParameter.IsParameterExpectedInDocumentVersion( m_versionParameter.GetValue() ) )
		{
			YAML::Node resourceGroupNode = resourceGroupFile[m_resourceGroupParameter.GetTag()];

			ResourceInfo* resource = nullptr;

			Result createResourceFromYaml = ResourceGroupImpl::CreateResourceFromYaml( resourceGroupNode, resource );

			if( createResourceFromYaml != Result::SUCCESS )
			{
				return createResourceFromYaml;
			}

			//TODO ensure that resource is of base type ResourceGroup

			m_resourceGroupParameter = reinterpret_cast<ResourceGroupInfo*>( resource );
		}

        if (m_chunkSize.IsParameterExpectedInDocumentVersion(m_versionParameter.GetValue()))
        {
            // TODO handle failure
			m_chunkSize = resourceGroupFile[m_chunkSize.GetTag()].as<unsigned long>();
        }

		return Result::SUCCESS;
    }

    Result BundleResourceGroupImpl::ExportGroupSpecialisedYaml( YAML::Emitter& out, Version outputDocumentVersion ) const
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

		return Result::SUCCESS;
    }

}