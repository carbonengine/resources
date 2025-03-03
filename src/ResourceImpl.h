/* 
	*************************************************************************

	ResourceImpl.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/
#pragma once
#ifndef ResourceImpl_H
#define ResourceImpl_H

#include "Resource.h"

class CarbonResources::Resource::ResourceImpl
{
public:
	ResourceImpl( const ResourceParams& params );

    ~ResourceImpl();

    ResourceParams& GetResourceParams();

    Result GetData( const ResourceGetDataParams& params );

private:

    ResourceParams m_resourceParameters;

};

#endif // ResourceImpl_H