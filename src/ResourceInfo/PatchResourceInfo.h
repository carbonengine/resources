/* 
	*************************************************************************

	PatchResourceInfo.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/
#pragma once
#ifndef PatchResourceInfo_H
#define PatchResourceInfo_H

#include "ResourceGroupInfo.h"

namespace CarbonResources
{
    struct PatchResourceInfoParams : public ResourceGroupInfoParams
    {
    
    };

    class ResourceGroup;

    class PatchResourceInfo : public ResourceInfo
    {
    public:
	    PatchResourceInfo( const PatchResourceInfoParams& params );

	    ~PatchResourceInfo();

        static std::string TypeId( );
    };


}

#endif // PatchResourceInfo_H