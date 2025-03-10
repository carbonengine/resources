#include "BundleResourceGroupImpl.h"

//#include "BundleResource.h"

#include <yaml-cpp/yaml.h>

namespace CarbonResources
{

    BundleResourceGroupImpl::BundleResourceGroupImpl( ) :
	    ResourceGroupImpl()
    {
		m_type = TypeId();
    }

    BundleResourceGroupImpl::~BundleResourceGroupImpl()
    {

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
		return Result::SUCCESS;
    }

    Result BundleResourceGroupImpl::ExportGroupSpecialisedYaml( YAML::Emitter& out, Version outputDocumentVersion ) const
    {
		return Result::SUCCESS;
    }

}