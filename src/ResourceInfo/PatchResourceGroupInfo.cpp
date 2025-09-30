// Copyright © 2025 CCP ehf.

#include "PatchResourceGroupInfo.h"

#include <sstream>

#include <ResourceTools.h>

namespace CarbonResources
{

PatchResourceGroupInfo::PatchResourceGroupInfo( const PatchResourceGroupInfoParams& params ) :
	ResourceInfo( params )
{
	m_type = TypeId();
}

PatchResourceGroupInfo::~PatchResourceGroupInfo()
{
}

std::string PatchResourceGroupInfo::TypeId()
{
	return "PatchResourceGroup";
}

}