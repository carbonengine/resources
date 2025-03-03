/* 
	*************************************************************************

	BundleResourceImpl.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/
#pragma once
#ifndef BundleResourceImpl_H
#define BundleResourceImpl_H

#include "BundleResource.h"

#include "ResourceImpl.h"

class CarbonResources::BundleResource::BundleResourceImpl : public CarbonResources::Resource::ResourceImpl
{
public:
	BundleResourceImpl( const BundleResourceParams& params );

    ~BundleResourceImpl();

};

#endif // BundleResourceImpl_H