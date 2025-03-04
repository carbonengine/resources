#include "PatchResourceGroupImpl.h"

#include "PatchResource.h"
#include "PatchResourceImpl.h"

#include <yaml-cpp/yaml.h>

namespace CarbonResources
{

    PatchResourceGroupImpl::PatchResourceGroupImpl( const std::string& relativePath ):
	    ResourceGroupImpl(relativePath)
    {

    }

    PatchResourceGroupImpl::~PatchResourceGroupImpl()
    {

    }

    std::string PatchResourceGroupImpl::Type() const
    {
		return "PatchGroup";
    }

	Resource* PatchResourceGroupImpl::CreateResourceFromYaml( YAML::Node& resource )
	{
		PatchResource* createdResource = new PatchResource( PatchResourceParams{} );

		Result importFromYamlResult = createdResource->m_impl->ImportFromYaml( resource, m_versionParameter.GetValue() );

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
		if( m_resourceGroupPathParameter.IsParameterExpectedInDocumentVersion( m_versionParameter.GetValue() ) )
		{
			m_resourceGroupPathParameter = resourceGroupFile[m_resourceGroupPathParameter.GetTag()].as<std::string>();
		}

		return Result::SUCCESS;
    }

    Result PatchResourceGroupImpl::ExportGroupSpecialisedYaml( YAML::Emitter& out, Version outputDocumentVersion ) const
    {
        if (m_resourceGroupPathParameter.IsParameterExpectedInDocumentVersion(outputDocumentVersion))
        {
			out << YAML::Key << m_resourceGroupPathParameter.GetTag();
			out << YAML::Value << m_resourceGroupPathParameter.GetValue();
        }

		return Result::SUCCESS;
    }

    Result PatchResourceGroupImpl::SetResourceGroupPath( const std::string& resourceGroupPath )
    {
		m_resourceGroupPathParameter = resourceGroupPath;

		return Result::SUCCESS;
    }

}