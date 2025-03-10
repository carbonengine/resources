/* 
	*************************************************************************

	ResourceGroup.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/

#pragma once
#ifndef ResourceGroup_H
#define ResourceGroup_H

#include "Exports.h"
#include "Enums.h"
#include <memory>
#include <string>
#include <filesystem>

namespace CarbonResources
{
    class ResourceGroup;
    class PatchResourceGroup;
    class BundleResourceGroup;
	enum class Result;

    struct API BundleCreateParams
    {
        std::filesystem::path resourceInputPath = "";

        std::filesystem::path chunkOutputPath = "";

        BundleResourceGroup* bundleResourceGroup = nullptr;
    };

    struct API ResourceSourceSettings
	{
		std::filesystem::path developmentLocalBasePath = "";

		std::filesystem::path productionLocalBasePath = "";

		std::string productionRemoteBaseUrl = "";
	};

	struct API ResourceDestinationSettings
	{
		std::filesystem::path developmentLocalBasePath = ""; // TODO rename, it's not just developmentLocal now

		std::filesystem::path productionLocalBasePath = "";
	};

    struct API PatchCreateParams
	{
		ResourceGroup* previousResourceGroup;

        std::filesystem::path resourceGroupRelativePath;

        std::filesystem::path resourceGroupPatchRelativePath;

		ResourceSourceSettings resourceSourceSettingsFrom;

		ResourceSourceSettings resourceSourceSettingsTo;

        ResourceDestinationSettings resourcePatchBinaryDestinationSettings;

        ResourceDestinationSettings resourcePatchResourceGroupDestinationSettings;
	};

    struct API ResourceGroupImportFromFileParams
	{
		std::filesystem::path filename;
	};

    struct API ResourceGroupExportToFileParams
	{
		std::filesystem::path filename;

        Version outputDocumentVersion = S_DOCUMENT_VERSION;
	};

    class ResourceGroupImpl;    // TODO remove these from public API

    // TODO lock down things like copy constructors for these public classes
    class API ResourceGroup
    {
	protected:
        ResourceGroup( ResourceGroupImpl* impl );

		ResourceGroupImpl* m_impl;

    public:
	    ResourceGroup();

	    virtual ~ResourceGroup();

        Result CreateBundle( const BundleCreateParams& params ) const;

        Result CreatePatch( PatchCreateParams& params ) const;

        Result ImportFromFile( ResourceGroupImportFromFileParams& params ) const;

        Result ExportToFile( const ResourceGroupExportToFileParams& params ) const;

        friend class ResourceGroupImpl;
    };

}

#endif // ResourceGroup_H