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

#include "Exports.h"
#include "Resource.h"

namespace YAML
{
    class Emitter;
    class Node;
}

namespace CarbonResources
{
    class API BundleResourceParams : public ResourceParams
    {
	public:
        BundleResourceParams();

    };

    class API BundleResource final : public Resource
    {

    private:
	    class BundleResourceImpl;

    public:
        BundleResource( const BundleResourceParams& params );

	    ~BundleResource();

    private:
		BundleResourceImpl* m_impl;

    };

}

#endif // BundleResource_H