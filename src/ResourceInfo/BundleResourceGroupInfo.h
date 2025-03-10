/* 
	*************************************************************************

	BundleResourceGroupInfo.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/
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

        static std::string TypeId( );

    };

}

#endif // BundleResourceGroupInfo_H