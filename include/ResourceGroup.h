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

namespace CarbonResources
{
    class ResourceGroup;
    class PatchResourceGroup;
    class BundleResourceGroup;
	enum class Result;

    struct API BundleCreateParams
    {
        std::string resourceInputPath = "";

        std::string chunkOutputPath = "";

        BundleResourceGroup* bundleResourceGroup = nullptr;
    };

    struct API ResourceSourceSettings
	{
		std::string developmentLocalBasePath = "";

		std::string productionLocalBasePath = "";

		std::string productionRemoteBaseUrl = "";
	};

	struct API ResourceDestinationSettings
	{
		std::string developmentLocalBasePath = ""; // TODO needs to be supported and also needs a rename, it's not just developmentLocal now

		std::string productionLocalBasePath = "";
	};

    struct API ResourcePutDataParams
	{
		ResourceDestinationSettings resourceDestinationSettings;

		std::string data;
	};

	struct API ResourceGetDataParams
	{
		ResourceSourceSettings resourceSourceSettings;

		std::string data;
	};

    struct API PatchCreateParams
	{
		PatchResourceGroup* patchResourceGroup;

		ResourceSourceSettings resourceSourceSettingsFrom;

		ResourceSourceSettings resourceSourceSettingsTo;

        ResourceDestinationSettings resourceDestinationSettings;
	};

    struct API ResourceGroupImportFromFileParams
	{
		ResourceGetDataParams dataParams;
	};

    struct API ResourceGroupExportToFileParams
	{
		ResourceDestinationSettings resourceDetinationSettings;

        Version outputDocumentVersion = S_DOCUMENT_VERSION;
	};

    struct API ResourceGroupSubtractionParams
	{
		ResourceGroup* subtractResourceGroup = nullptr;

		ResourceGroup* result = nullptr;
	};

    class Resource;
    class ResourceGroupImpl;

    class API ResourceGroup
    {
	protected:
        ResourceGroup( ResourceGroupImpl* impl );

		ResourceGroupImpl* m_impl;

        Result AddResource( Resource* r );

    public:
	    ResourceGroup(const std::string& relativePath); // TODO should be an input struct

	    virtual ~ResourceGroup();

        Result CreateBundle( const BundleCreateParams& params ) const;

        Result CreatePatch( PatchCreateParams& params ) const;

        Result ImportFromFile( ResourceGroupImportFromFileParams& params ) const;

        Result ExportToFile( const ResourceGroupExportToFileParams& params ) const;

        Result Subtraction( ResourceGroupSubtractionParams& params ) const; // TODO not too thrilled about this being in public API

        friend class ResourceGroupImpl;
    };

}

#endif // ResourceGroup_H