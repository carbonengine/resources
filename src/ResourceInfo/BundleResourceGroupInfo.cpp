// Copyright © 2025 CCP ehf.

#include "BundleResourceGroupInfo.h"

#include <sstream>

#include <ResourceTools.h>

namespace CarbonResources
{

BundleResourceGroupInfo::BundleResourceGroupInfo( const BundleResourceGroupInfoParams& params ) :
	ResourceInfo( params )
{
	m_type = TypeId();
}

BundleResourceGroupInfo::~BundleResourceGroupInfo()
{
}

std::string BundleResourceGroupInfo::TypeId()
{
	return "BundleResourceGroup";
}

}