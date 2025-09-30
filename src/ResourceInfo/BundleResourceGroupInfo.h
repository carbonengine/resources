// Copyright © 2025 CCP ehf.

#pragma once
#ifndef BundleResourceGroupInfo_H
#define BundleResourceGroupInfo_H

#include "ResourceGroupInfo.h"

namespace CarbonResources
{
struct BundleResourceGroupInfoParams : public ResourceGroupInfoParams
{
};

class BundleResourceGroupInfo : public ResourceInfo
{
public:
	BundleResourceGroupInfo( const BundleResourceGroupInfoParams& params );

	~BundleResourceGroupInfo();

	static std::string TypeId();
};

}

#endif // BundleResourceGroupInfo_H