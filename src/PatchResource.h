/* 
	*************************************************************************

	PatchResource.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/
#pragma once
#ifndef PatchResource_H
#define PatchResource_H

#include "Resource.h"

namespace CarbonResources
{
    struct PatchResourceParams : public ResourceParams
    {
    
    };

    class ResourceGroup;

    class PatchResource : public Resource
    {
    public:
	    PatchResource( const PatchResourceParams& params );

	    ~PatchResource();

        virtual Result GetPathPrefix( std::string& prefix ) const override;
    };


}

#endif // PatchResource_H