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

#include "Exports.h"
#include "Resource.h"

namespace YAML
{
    class Emitter;
    class Node;
}

namespace CarbonResources
{
    class API PatchResourceParams : public ResourceParams
    {
	public:
        PatchResourceParams();

    };

    class API PatchResource final : public Resource
    {

    private:
	    class PatchResourceImpl;

    public:
        PatchResource( const PatchResourceParams& params );

	    ~PatchResource();

    private:
		PatchResourceImpl* m_impl;

    };

}

#endif // PatchResource_H