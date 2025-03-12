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
    
    enum class ResourceSourceType
    {
        LOCAL_RELATIVE,
        LOCAL_CDN,
        REMOTE_CDN,
    };

    struct API ResourceSourceSettings
	{
		unsigned int size = sizeof( ResourceSourceSettings );

		ResourceSourceType sourceType = ResourceSourceType::LOCAL_CDN;

        std::filesystem::path basePath = "";
	};

    enum class ResourceDestinationType
	{
		LOCAL_RELATIVE,
		LOCAL_CDN,
	};

	struct API ResourceDestinationSettings
	{
		unsigned int size = sizeof( ResourceDestinationSettings );

		ResourceDestinationType destinationType = ResourceDestinationType::LOCAL_CDN;

        std::filesystem::path basePath = "";
	};

    struct API BundleCreateParams
	{
		ResourceSourceSettings resourceSourceSettings;

        ResourceDestinationSettings chunkDestinationSettings;

        std::filesystem::path resourceGroupRelativePath;

		std::filesystem::path resourceGroupBundleRelativePath;

        unsigned long chunkSize = 10000000;

        ResourceDestinationSettings resourceBundleResourceGroupDestinationSettings;
	};

    struct API PatchCreateParams
	{
		unsigned int size = sizeof( PatchCreateParams );

		ResourceGroup* previousResourceGroup = nullptr;

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

        Result CreatePatch( const PatchCreateParams& params ) const;

        Result ImportFromFile( const ResourceGroupImportFromFileParams& params ) const;

        Result ExportToFile( const ResourceGroupExportToFileParams& params ) const;

        friend class ResourceGroupImpl;
    };

}

#endif // ResourceGroup_H