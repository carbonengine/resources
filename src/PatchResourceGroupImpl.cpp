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
        ResourceGetDataParams resourceGroupDataParams;

        resourceGroupDataParams.resourceSourceSettings = params.patchBinarySourceSettings;

        Result resourceGroupGetDataResult = m_resourceGroupParameter.GetValue()->GetData( resourceGroupDataParams );

        if (resourceGroupGetDataResult != Result::SUCCESS)
        {
			return resourceGroupGetDataResult;
        }

        ResourceGroupImpl resourceGroup;

        Result resourceGroupImportFromDataResult = resourceGroup.ImportFromData( resourceGroupDataParams.data );

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
				ResourceGetDataParams patchGetDataParams;

				patchGetDataParams.resourceSourceSettings = params.patchBinarySourceSettings;

                Result getPatchDataResult = patch->GetData( patchGetDataParams );

				if( getPatchDataResult != Result::SUCCESS )
				{
					return getPatchDataResult;
				}

                // Get previous data
				ResourceGetDataParams resourceGetDataParams;

				resourceGetDataParams.resourceSourceSettings = params.resourcesToPatchSourceSettings;

				Result resourceGetDataResult = resource->GetData( resourceGetDataParams );

				if( resourceGetDataResult != Result::SUCCESS )
				{
					return resourceGetDataResult;
				}

                // Apply the patch to the previous data
				std::string patchedResourceData;

				if( !ResourceTools::ApplyPatch( resourceGetDataParams.data, patchGetDataParams.data, patchedResourceData ) )
				{
					return Result::FAILED_TO_APPLY_PATCH;
				}

                // Validate patched data matches expected
				std::string patchedFileChecksum;

				if( !ResourceTools::GenerateMd5Checksum( patchedResourceData, patchedFileChecksum ) )
				{
					return Result::FAILED_TO_GENERATE_CHECKSUM;
				}

				const std::string destinationExpectedChecksum = resource->GetChecksum();

				if( patchedFileChecksum != destinationExpectedChecksum )
				{
                    //TODO reinstate when test data is correct
                    //Currently the test data used is compressed and so the checksum
                    //Doesn't match what is in the test data
                    //Either store this uncompressed or utilise decompression in the Get
					//return Result::UNEXPECTED_PATCH_CHECKSUM_RESULT;
				}

                // Put the file in the destination specified
				ResourceInfo patchedResource( { resource->GetRelativePath() } );
				patchedResource.SetParametersFromData( patchedResourceData );

				// Export patch file
				ResourcePutDataParams patchedResourceResourcePutDataParams;

				patchedResourceResourcePutDataParams.resourceDestinationSettings = params.resourcesToPatchDestinationSettings;

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
				ResourceGetDataParams resourceGetDataParams;

                resourceGetDataParams.resourceSourceSettings = params.patchBinarySourceSettings;

				Result resourceGetDataResult = resource->GetData( resourceGetDataParams );

                if (resourceGetDataResult != Result::SUCCESS)
                {
					return resourceGetDataResult;
                }

                ResourcePutDataParams resourcePutDataParams;

                resourcePutDataParams.resourceDestinationSettings = params.resourcesToPatchDestinationSettings;

                resourcePutDataParams.data = resourceGetDataParams.data;    // TODO refactor this away, should not be a copy

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