#include "BundleResourceGroupImpl.h"

#include "BundleResource.h"

#include <yaml-cpp/yaml.h>

namespace CarbonResources
{

    BundleResourceGroup::BundleResourceGroupImpl::BundleResourceGroupImpl( )
    {

    }

    BundleResourceGroup::BundleResourceGroupImpl::~BundleResourceGroupImpl()
    {

    }

    std::string BundleResourceGroup::BundleResourceGroupImpl::Type() const
	{
		return "BundleGroup";
	}

    Resource* BundleResourceGroup::BundleResourceGroupImpl::CreateResourceFromYaml( YAML::Node& resource )
	{
		CarbonResources::BundleResourceParams bundleResourceParams;

		bundleResourceParams.ImportFromYaml( resource, m_versionParameter.GetValue() );

		return new BundleResource( bundleResourceParams );
	}

    Result BundleResourceGroup::BundleResourceGroupImpl::ImportGroupSpecialisedYaml( YAML::Node& resourceGroupFile )
    {
		return Result::SUCCESS;
    }

    Result BundleResourceGroup::BundleResourceGroupImpl::ExportGroupSpecialisedYaml( YAML::Emitter& out, Version outputDocumentVersion ) const
    {
		return Result::SUCCESS;
    }

}