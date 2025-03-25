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
#include <filesystem>

namespace CarbonResources
{

    struct API PatchApplyParams final
    {
		ResourceSourceSettings newBuildResourcesSourceSettings;

		ResourceSourceSettings patchBinarySourceSettings;

        ResourceSourceSettings resourcesToPatchSourceSettings;

        ResourceDestinationSettings resourcesToPatchDestinationSettings;

        std::filesystem::path temporaryFilePath = "tempFile.resource";
    };

	class PatchResourceGroupImpl;

    /** @class PatchResourceGroup
    *  @brief This class blah blah blah...
    */
    class API PatchResourceGroup final: public ResourceGroup
    {

    public:
		PatchResourceGroup( );

	    ~PatchResourceGroup();

        Result Apply( const PatchApplyParams& params );

    private:

		PatchResourceGroupImpl* m_impl;

    };

}

#endif // PatchResourceGroup_H