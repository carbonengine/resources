#include "PatchResourceGroupImpl.h"

#include "PatchResource.h"

#include <yaml-cpp/yaml.h>

namespace CarbonResources
{

    PatchResourceGroup::PatchResourceGroupImpl::PatchResourceGroupImpl( )
    {

    }

    PatchResourceGroup::PatchResourceGroupImpl::~PatchResourceGroupImpl()
    {

    }

    std::string PatchResourceGroup::PatchResourceGroupImpl::Type() const
    {
		return "PatchGroup";
    }

	Resource* PatchResourceGroup::PatchResourceGroupImpl::CreateResourceFromYaml( YAML::Node& resource )
	{
		CarbonResources::PatchResourceParams patchResourceParams;

		patchResourceParams.ImportFromYaml( resource, m_versionParameter.GetValue() );

		return new PatchResource( patchResourceParams );
	}

    Result PatchResourceGroup::PatchResourceGroupImpl::ImportGroupSpecialisedYaml( YAML::Node& resourceGroupFile )
    {
		if( m_resourceGroupPathParameter.IsParameterExpectedInDocumentVersion( m_versionParameter.GetValue() ) )
		{
			m_resourceGroupPathParameter = resourceGroupFile[m_resourceGroupPathParameter.GetTag()].as<std::string>();
		}

		return Result::SUCCESS;
    }

    Result PatchResourceGroup::PatchResourceGroupImpl::ExportGroupSpecialisedYaml( YAML::Emitter& out, Version outputDocumentVersion ) const
    {
        if (m_resourceGroupPathParameter.IsParameterExpectedInDocumentVersion(outputDocumentVersion))
        {
			out << YAML::Key << m_resourceGroupPathParameter.GetTag();
			out << YAML::Value << m_resourceGroupPathParameter.GetValue();
        }

		return Result::SUCCESS;
    }

}