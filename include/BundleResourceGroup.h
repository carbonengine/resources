/* 
	*************************************************************************

	BundleResourceGroup.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/

#pragma once
#ifndef BundleResourceGroup_H
#define BundleResourceGroup_H

#include "Exports.h"
#include "ResourceGroup.h"
#include "Enums.h"
#include <memory>
#include <string>

namespace CarbonResources
{

    struct API BundleUnpackParams final
    {
	    std::string outputDirectoryPath = "";
    };

    class BundleResourceGroupImpl;

    class API BundleResourceGroup final: public ResourceGroup
    {
    public:

	    BundleResourceGroup( const std::string& relativePath );

	    ~BundleResourceGroup();

        Result Unpack( const BundleUnpackParams& params );

    private:

		BundleResourceGroupImpl* m_impl;

    };

}

#endif // BundleResourceGroup_H