#include "BundleResourceGroupImpl.h"

#include "BundleResource.h"

#include <yaml-cpp/yaml.h>

namespace CarbonResources
{

    BundleResourceGroupImpl::BundleResourceGroupImpl( const std::string& relativePath ):
	    ResourceGroupImpl(relativePath)
    {

    }

    BundleResourceGroupImpl::~BundleResourceGroupImpl()
    {

    }

    std::string BundleResourceGroupImpl::Type() const
	{
		return "BundleGroup";
	}

    Resource* BundleResourceGroupImpl::CreateResourceFromYaml( YAML::Node& resource )
	{
		BundleResource* createdResource = new BundleResource( BundleResourceParams{} );

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

    Result BundleResourceGroupImpl::ImportGroupSpecialisedYaml( YAML::Node& resourceGroupFile )
    {
		return Result::SUCCESS;
    }

    Result BundleResourceGroupImpl::ExportGroupSpecialisedYaml( YAML::Emitter& out, Version outputDocumentVersion ) const
    {
		return Result::SUCCESS;
    }

}