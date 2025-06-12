#ifndef RESOURCEGROUPFACTORY_H
#define RESOURCEGROUPFACTORY_H
#include "ResourceGroup.h"

namespace CarbonResources
{
	Result CreateFromYamlString( const std::string& yamlString, std::shared_ptr<ResourceGroupImpl>& out );
}

#endif //RESOURCEGROUPFACTORY_H
