/* 
	*************************************************************************

	PatchResourceGroup.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/

#pragma once
#ifndef PatchResourceGroup_H
#define PatchResourceGroup_H

#include "Exports.h"
#include "ResourceGroup.h"
#include "Enums.h"
#include <memory>
#include <string>

namespace CarbonResources
{

    struct API PatchApplyParams final
    {
	    std::string resourceInputPath = "";

	    std::string chunkOutputPath = "";

	    BundleResourceGroup* bundleResourceGroup = nullptr;
    };

	class PatchResourceGroupImpl;
	class ResourceGroupImpl;

    class API PatchResourceGroup final: public ResourceGroup
    {

    public:
		PatchResourceGroup( const std::string& relativePath );  // TODO struct input

	    ~PatchResourceGroup();

        Result Apply( const PatchApplyParams& params );

        Result SetResourceGroup( ResourceGroupImpl* resourceGroup );  // TODO get out of public API!!

    private:

		PatchResourceGroupImpl* m_impl;

    };

}

#endif // PatchResourceGroup_H