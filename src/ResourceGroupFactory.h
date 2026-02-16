// Copyright © 2025 CCP ehf.

#ifndef RESOURCEGROUPFACTORY_H
#define RESOURCEGROUPFACTORY_H
#include "ResourceGroup.h"
#include "ResourceInfo/ResourceInfo.h"
#include "StatusSettings.h"

namespace CarbonResources
{
Result CreateResourceGroupFromYamlString( const std::string& yamlString, std::shared_ptr<ResourceGroup::ResourceGroupImpl>& out, StatusSettings& statusSettings );

Result CreateResourceGroupFromString( std::string& string, std::shared_ptr<ResourceGroup::ResourceGroupImpl>& out );

Result CreateResourceInfoFromYamlNode( YAML::Node& resource, std::unique_ptr<ResourceInfo>& out, const VersionInternal& documentVersion );

Result CreateResourceInfoFromString( std::string& string, std::unique_ptr<ResourceInfo>& out );

}

#endif //RESOURCEGROUPFACTORY_H
