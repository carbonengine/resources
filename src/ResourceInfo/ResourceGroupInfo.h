// Copyright © 2025 CCP ehf.

#pragma once
#ifndef ResourceGroupInfo_H
#define ResourceGroupInfo_H

#include "ResourceInfo.h"

namespace CarbonResources
{
    struct ResourceGroupInfoParams : public ResourceInfoParams
    {

    };

    class ResourceGroupInfo : public ResourceInfo
    {
    public:
	    ResourceGroupInfo( const ResourceGroupInfoParams& params );

        ~ResourceGroupInfo();

        static std::string TypeId( );

    };

}

#endif // ResourceGroupInfo_H