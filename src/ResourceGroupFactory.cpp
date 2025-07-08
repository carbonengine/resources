#include "ResourceGroupFactory.h"

#include <yaml-cpp/yaml.h>

#include "ResourceGroupImpl.h"
#include "PatchResourceGroupImpl.h"
#include "BundleResourceGroupImpl.h"

#include "ResourceInfo/BundleResourceGroupInfo.h"
#include "ResourceInfo/BundleResourceInfo.h"
#include "ResourceInfo/PatchResourceGroupInfo.h"
#include "ResourceInfo/PatchResourceInfo.h"
#include "ResourceInfo/ResourceGroupInfo.h"
#include "ResourceInfo/ResourceInfo.h"


namespace CarbonResources
{
    Result CreateResourceGroupFromYamlString( const std::string& yamlString, std::shared_ptr<ResourceGroup::ResourceGroupImpl>& out )
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

        Result createFromStringResult = CreateResourceGroupFromString( resourceGroupTypeString, out );

        if( createFromStringResult.type != ResultType::SUCCESS )
        {
			return createFromStringResult;
        }

		return out->ImportFromYaml( root );
	}

    Result CreateResourceGroupFromString( std::string& string, std::shared_ptr<ResourceGroup::ResourceGroupImpl>& out )
    {
		if( string == ResourceGroup::ResourceGroupImpl::TypeId() )
		{
			out = std::make_shared<ResourceGroup::ResourceGroupImpl>();
		}
		else if( string == PatchResourceGroup::PatchResourceGroupImpl::TypeId() )
		{
			out = std::make_shared<PatchResourceGroup::PatchResourceGroupImpl>();
		}
		else if( string == BundleResourceGroup::BundleResourceGroupImpl::TypeId() )
		{
			out = std::make_shared<BundleResourceGroup::BundleResourceGroupImpl>();
		}
		else
		{
			return Result{ ResultType::MALFORMED_RESOURCE_GROUP, "Unexpected Resource Group Type: '" + string + "'." };
		}

        return Result{ ResultType::SUCCESS };
    }




    Result CreateResourceInfoFromYamlNode( YAML::Node& resource, std::unique_ptr<ResourceInfo>& out, const VersionInternal& documentVersion )
	{
		YAML::Node type = resource["Type"];
		if( !type.IsDefined() )
		{
			return Result{ ResultType::MALFORMED_RESOURCE, "Tried to load a resource info without a 'Type' attribute." };
		}
		auto resourceGroupTypeString = type.as<std::string>();

		Result createFromStringResult = CreateResourceInfoFromString( resourceGroupTypeString, out );

		if( createFromStringResult.type != ResultType::SUCCESS )
		{
			return createFromStringResult;
		}

		return out->ImportFromYaml( resource, documentVersion );
	}

	Result CreateResourceInfoFromString( std::string& string, std::unique_ptr<ResourceInfo>& out )
	{

		if( string == BundleResourceGroupInfo::TypeId() )
		{
			out = std::make_unique<BundleResourceGroupInfo>( BundleResourceGroupInfoParams {});
		}
		else if( string == BundleResourceInfo::TypeId() )
		{
			out = std::make_unique<BundleResourceInfo>( BundleResourceInfoParams{} );
		}
		else if( string == PatchResourceGroupInfo::TypeId() )
		{
			out = std::make_unique<PatchResourceGroupInfo>( PatchResourceGroupInfoParams{} );
		}
		else if( string == PatchResourceInfo::TypeId() )
		{
			out = std::make_unique<PatchResourceInfo>( PatchResourceInfoParams{} );
		}
		else if( string == ResourceGroupInfo::TypeId() )
		{
			out = std::make_unique<ResourceGroupInfo>( ResourceGroupInfoParams{} );
		}
		else if( string == ResourceInfo::TypeId() )
		{
			out = std::make_unique<ResourceInfo>( ResourceInfoParams{} );
		}
		
		else
		{
			return Result{ ResultType::MALFORMED_RESOURCE, "Unexpected Resource Info Type: '" + string + "'." };
		}

		return Result{ ResultType::SUCCESS };
	}
	}