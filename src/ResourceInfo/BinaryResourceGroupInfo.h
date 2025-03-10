/* 
	*************************************************************************

	BinaryResourceGroupInfo.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/
#pragma once
#ifndef BinaryResourceGroupInfo_H
#define BinaryResourceGroupInfo_H

#include "ResourceGroupInfo.h"

namespace CarbonResources
{
    struct BinaryResourceGroupInfoParams : public ResourceGroupInfoParams
    {

    };

    class BinaryResourceGroupInfo : public ResourceInfo
    {
    public:
		BinaryResourceGroupInfo( const BinaryResourceGroupInfoParams& params );

        ~BinaryResourceGroupInfo();

        static std::string TypeId( );

    };

}

#endif // BinaryResourceGroupInfo_H