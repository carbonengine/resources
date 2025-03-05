/* 
	*************************************************************************

	BundleResource.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/
#pragma once
#ifndef BundleResource_H
#define BundleResource_H

#include "Resource.h"

namespace CarbonResources
{
    struct BundleResourceParams : public ResourceParams
    {

    };

    class BundleResource : public Resource
    {
    public:
	    BundleResource( const BundleResourceParams& params );

        ~BundleResource();

    };

}

#endif // BundleResource_H