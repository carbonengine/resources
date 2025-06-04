/* 
	*************************************************************************

	BundleResourceInfo.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/
#pragma once
#ifndef BundleResourceInfo_H
#define BundleResourceInfo_H

#include "ResourceInfo.h"

namespace CarbonResources
{
    struct BundleResourceInfoParams : public ResourceInfoParams
    {

    };

    class BundleResourceInfo : public ResourceInfo
    {
    public:
	    BundleResourceInfo( const BundleResourceInfoParams& params );

        ~BundleResourceInfo();

        static std::string TypeId( );

        virtual Result SetParametersFromResource( const ResourceInfo* other, const VersionInternal& documentVersion ) override;

    };

}

#endif // BundleResourceInfo_H