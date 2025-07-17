// Copyright © 2025 CCP ehf.

#include "ResourceGroupInfo.h"

#include <sstream>

#include <ResourceTools.h>

namespace CarbonResources
{

    ResourceGroupInfo::ResourceGroupInfo( const ResourceGroupInfoParams& params ) :
      ResourceInfo(params)
    {
		m_type = TypeId();
    }

    ResourceGroupInfo::~ResourceGroupInfo()
    {

    }

    std::string ResourceGroupInfo::TypeId()
    {
		return "ResourceGroup";
    }

}