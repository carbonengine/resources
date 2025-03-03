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

    struct API PatchCreateParams
	{
		ResourceGroup* previousResourceGroup = nullptr;

		std::string outputDirectoryPath = "";

		PatchResourceGroup* patchResourceGroup = nullptr;

        std::string basePath = "";
	};

    struct API ResourceGroupImportFromFileParams
	{
		std::string inputFilename = "";
	};

    struct API ResourceGroupExportToFileParams
	{
		std::string outputFilename = "";

        Version outputDocumentVersion = S_DOCUMENT_VERSION;
	};

    class API ResourceGroup
    {
	protected:
		class ResourceGroupImpl;

        ResourceGroup( ResourceGroupImpl* impl );

		ResourceGroupImpl* m_impl;

    public:
	    ResourceGroup();

	    virtual ~ResourceGroup();

        Result CreateBundle( const BundleCreateParams& params ) const;

        Result CreatePatch( const PatchCreateParams& params ) const;

        Result ImportFromFile( const ResourceGroupImportFromFileParams& params ) const;

        Result ExportToFile( const ResourceGroupExportToFileParams& params ) const;

    };

}

#endif // ResourceGroup_H