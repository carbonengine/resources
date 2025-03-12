#include "PatchResourceGroupImpl.h"

//#include "PatchResource.h"

#include <yaml-cpp/yaml.h>

#include <ResourceTools.h>

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

		return Result::SUCCESS;
    }

    Result PatchResourceGroupImpl::Apply( const PatchApplyParams& params )
    {
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
			auto patchIter = m_resourcesParameter.Find( resource );

			if( patchIter != m_resourcesParameter.end() )
			{
				ResourceInfo* patch = ( *patchIter );

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
				std::string previousResourceData;

				ResourceGetDataParams resourceGetDataParams;

				resourceGetDataParams.resourceSourceSettings = params.resourcesToPatchSourceSettings;

                resourceGetDataParams.data = &previousResourceData;

				Result resourceGetDataResult = resource->GetData( resourceGetDataParams );

				if( resourceGetDataResult != Result::SUCCESS )
				{
					return resourceGetDataResult;
				}

                // Apply the patch to the previous data
				std::string patchedResourceData;

				if( !ResourceTools::ApplyPatch( previousResourceData, patchData, patchedResourceData ) )
				{
					return Result::FAILED_TO_APPLY_PATCH;
				}

                // Validate patched data matches expected
				std::string patchedFileChecksum;

				if( !ResourceTools::GenerateMd5Checksum( patchedResourceData, patchedFileChecksum ) )
				{
					return Result::FAILED_TO_GENERATE_CHECKSUM;
				}

                std::string destinationExpectedChecksum;

                Result getChecksumResult = resource->GetChecksum( destinationExpectedChecksum );

                if (getChecksumResult != Result::SUCCESS)
                {
					return getChecksumResult;
                }

				if( patchedFileChecksum != destinationExpectedChecksum )
				{
					return Result::UNEXPECTED_PATCH_CHECKSUM_RESULT;
				}

                // Put the file in the destination specified
				std::filesystem::path resourceRelativePath;

                Result getResourceRelativePath = resource->GetRelativePath( resourceRelativePath );

                if (getResourceRelativePath != Result::SUCCESS)
                {
					return getResourceRelativePath;
                }

				ResourceInfo patchedResource( { resourceRelativePath } );
				patchedResource.SetParametersFromData( patchedResourceData );

				// Export patch file
				ResourcePutDataParams patchedResourceResourcePutDataParams;

				patchedResourceResourcePutDataParams.resourceDestinationSettings = params.resourcesToPatchDestinationSettings;

                patchedResourceResourcePutDataParams.data = &patchedResourceData;

				Result putResourceDataResult = patchedResource.PutData( patchedResourceResourcePutDataParams );

				if( putResourceDataResult != Result::SUCCESS )
				{
					return putResourceDataResult;
				}

			}
            else
            {
                // No Patch found, indicates this is just a new file
                // Just download file directly
				std::string resourceData;

				ResourceGetDataParams resourceGetDataParams;

                resourceGetDataParams.resourceSourceSettings = params.patchBinarySourceSettings;

                resourceGetDataParams.data = &resourceData;

				Result resourceGetDataResult = resource->GetData( resourceGetDataParams );

                if (resourceGetDataResult != Result::SUCCESS)
                {
					return resourceGetDataResult;
                }

                ResourcePutDataParams resourcePutDataParams;

                resourcePutDataParams.resourceDestinationSettings = params.resourcesToPatchDestinationSettings;

                resourcePutDataParams.data = &resourceData; 

                Result resourcePutDataResult = resource->PutData( resourcePutDataParams );

                if (resourcePutDataResult != Result::SUCCESS)
                {
					return resourcePutDataResult;
                }
            }
        }

        return Result::SUCCESS;


    }

}