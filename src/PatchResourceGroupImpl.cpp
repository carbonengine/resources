#include "PatchResourceGroupImpl.h"

#include "PatchResource.h"

#include <yaml-cpp/yaml.h>

namespace CarbonResources
{
    PatchResourceGroupImpl::PatchResourceGroupImpl( const std::string& relativePath ):
	    ResourceGroupImpl(relativePath)
    {

    }

    Result PatchResourceGroupImpl::SetResourceGroup( const ResourceGroupImpl* resourceGroup )
    {
		// TODO this is all a bit scrappy
		ResourceParams resourceGroupParams;

		resourceGroupParams.relativePath = resourceGroup->GetRelativePath().GetValue().ToString();

		resourceGroupParams.location = resourceGroup->GetLocation().GetValue();

		resourceGroupParams.checksum = resourceGroup->GetChecksum().GetValue();

		resourceGroupParams.compressedSize = resourceGroup->GetCompressedSize().GetValue();

		resourceGroupParams.uncompressedSize = resourceGroup->GetUncompressedSize().GetValue();

		resourceGroupParams.something = resourceGroup->GetSomething().GetValue();

		m_resourceGroupParameter = new Resource( resourceGroupParams );

        return Result::SUCCESS;
    }

    /*
    PatchResourceGroupImpl::PatchResourceGroupImpl( const std::string& relativePath, const ResourceGroupImpl* resourceGroup ) :
	    ResourceGroupImpl(relativePath)
    {
        // TODO this is all a bit scrappy
		ResourceParams resourceGroupParams;

        resourceGroupParams.relativePath = resourceGroup->GetRelativePath().GetValue().ToString();

        resourceGroupParams.location = resourceGroup->GetLocation().GetValue();

        resourceGroupParams.checksum = resourceGroup->GetChecksum().GetValue();

        resourceGroupParams.compressedSize = resourceGroup->GetCompressedSize().GetValue();

        resourceGroupParams.uncompressedSize = resourceGroup->GetUncompressedSize().GetValue();

        resourceGroupParams.something = resourceGroup->GetSomething().GetValue();

		m_resourceGroupParameter = new Resource( resourceGroupParams );
    }
    */

    PatchResourceGroupImpl::~PatchResourceGroupImpl()
    {
		delete m_resourceGroupParameter.GetValue();
    }

    std::string PatchResourceGroupImpl::Type() const
    {
		return "PatchGroup";
    }

	Resource* PatchResourceGroupImpl::CreateResourceFromYaml( YAML::Node& resource )
	{
		PatchResource* createdResource = new PatchResource( PatchResourceParams{} );

		Result importFromYamlResult = createdResource->ImportFromYaml( resource, m_versionParameter.GetValue() );

		if( importFromYamlResult != Result::SUCCESS )
		{
			delete createdResource;
			return nullptr;
		}
		else
		{
			return createdResource;
		}

		return nullptr;
	}

    Result PatchResourceGroupImpl::ImportGroupSpecialisedYaml( YAML::Node& resourceGroupFile )
    {
		if( m_resourceGroupParameter.IsParameterExpectedInDocumentVersion( m_versionParameter.GetValue() ) )
		{
			YAML::Node resourceGroupNode = resourceGroupFile[m_resourceGroupParameter.GetTag()];

			m_resourceGroupParameter = CreateResourceFromYaml( resourceGroupNode );
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

}