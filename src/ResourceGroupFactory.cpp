#include "ResourceGroupFactory.h"

#include <yaml-cpp/yaml.h>

#include "ResourceGroupImpl.h"
#include "PatchResourceGroupImpl.h"
#include "BundleResourceGroupImpl.h"


namespace CarbonResources
{
	Result CreateFromYamlString( const std::string& yamlString, std::shared_ptr<ResourceGroupImpl>& out )
	{
	    YAML::Node root;
    	try
    	{
    		root = YAML::Load( yamlString );
    	}
    	catch( YAML::ParserException& )
    	{
    		return Result{ ResultType::FAILED_TO_PARSE_YAML };
    	}
		YAML::Node type = root["Type"];
		if( !type.IsDefined() )
		{
			return Result{ ResultType::MALFORMED_RESOURCE_GROUP, "Tried to load a resource group without a 'Type' attribute." };
		}
		auto resourceGroupTypeString = type.as<std::string>();

		if( resourceGroupTypeString == ResourceGroupImpl::TypeId() )
		{
			out = std::make_shared<ResourceGroupImpl>();
		}
		else if( resourceGroupTypeString == PatchResourceGroupImpl::TypeId() )
		{
			out = std::make_shared<PatchResourceGroupImpl>();
		}
		else if( resourceGroupTypeString == BundleResourceGroupImpl::TypeId() )
		{
			out = std::make_shared<BundleResourceGroupImpl>();
		}
		else
		{
			return Result{ ResultType::MALFORMED_RESOURCE_GROUP, "Unexpected Resource Group Type: '" + resourceGroupTypeString + "'." };
		}
		return out->ImportFromYaml( root );
	}
}