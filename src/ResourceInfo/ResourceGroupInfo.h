/* 
	*************************************************************************

	ResourceGroupInfo.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/
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