// Copyright © 2025 CCP ehf.

#pragma once
#ifndef PatchResourceGroupInfo_H
#define PatchResourceGroupInfo_H

#include "ResourceGroupInfo.h"

namespace CarbonResources
{
    struct PatchResourceGroupInfoParams : public ResourceGroupInfoParams
    {

    };

    class PatchResourceGroupInfo : public ResourceInfo
    {
    public:
	    PatchResourceGroupInfo( const PatchResourceGroupInfoParams& params );

        ~PatchResourceGroupInfo();

        static std::string TypeId( );

    };

}

#endif // PatchResourceGroupInfo_H