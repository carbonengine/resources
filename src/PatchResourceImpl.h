/* 
	*************************************************************************

	PatchResourceImpl.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/
#pragma once
#ifndef PatchResourceImpl_H
#define PatchResourceImpl_H

#include "PatchResource.h"

#include "ResourceImpl.h"

namespace CarbonResources
{
    class ResourceGroup;

    class PatchResourceImpl : public ResourceImpl
    {
    public:
	    PatchResourceImpl( const PatchResourceParams& params );

	    ~PatchResourceImpl();
    };


}

#endif // PatchResourceImpl_H