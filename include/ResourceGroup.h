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
#include <functional>




namespace CarbonResources
{
    
    class ResourceGroup;
    class PatchResourceGroup;
    class BundleResourceGroup;
	struct Result;
    
    /** @struct ResourceSourceSettings
    *  @brief Parameters to represent where and how a resource is sourced.
    *  @var ResourceSourceSettings::sourceType
    *  Specifies the type of resource location. See ResourceSourceType for more info.
    *  @var ResourceSourceSettings::basePaths
    *  The base paths to locate resources.
    */
    struct ResourceSourceSettings
	{
		ResourceSourceType sourceType = ResourceSourceType::LOCAL_CDN;

        std::vector<std::filesystem::path> basePaths;
	};

    /** @struct ResourceDestinationSettings
    *  @brief Parameters to represent where and how a resource is saved.
    *  @var ResourceDestinationSettings::destinationType
    *  Specifies the type of resource location. See ResourceDestinationType for more info.
    *  @var ResourceDestinationSettings::basePath
    *  The base path to save resources.
    */
	struct ResourceDestinationSettings
	{
		ResourceDestinationType destinationType = ResourceDestinationType::LOCAL_CDN;

        std::filesystem::path basePath = "";
	};

    /** @struct BundleCreateParams
    *  @brief Function Parameters required for CarbonResources::ResourceGroup::CreatePatch
    *  @var BundleCreateParams::resourceSourceSettings
    *  Where resources related to ResourceGroup are to be be sourced.
    *  @var BundleCreateParams::chunkDestinationSettings
    *  Where chunks created will be saved.
    *  @var BundleCreateParams::resourceGroupRelativePath
    *  Where to save a ResourceGroup the Bundle was based off. This ResourceGroup will match the original but can be saved somewhere else.
    *  @var BundleCreateParams::resourceGroupBundleRelativePath
    *  Relative path for use with the output BundleResourceGroup.
    *  @var BundleCreateParams::chunkSize
    *  Size of chunks to break files into. Value representation is in bytes, default is 10000000
    *  @var BundleCreateParams::fileReadChunkSize
    *  Size of chunks to read files in. Default is 10000000.
    *  @var BundleCreateParams::resourceBundleResourceGroupDestinationSettings
    *  Where to save the resulting BundleResourceGroup
    *  @var BundleCreateParams::statusCallback
    *  Optional status function callback. Callback is triggered at key status update events.
    */
    struct BundleCreateParams
	{
		ResourceSourceSettings resourceSourceSettings = { CarbonResources::ResourceSourceType::LOCAL_RELATIVE };

        ResourceDestinationSettings chunkDestinationSettings = { CarbonResources::ResourceDestinationType::LOCAL_CDN, "BundleOut/Chunks/" };

        std::filesystem::path resourceGroupRelativePath = "ResourceGroup.yaml";

		std::filesystem::path resourceGroupBundleRelativePath = "BundleResourceGroup.yaml";

        uintmax_t chunkSize = 50000000;

        uintmax_t fileReadChunkSize = 10000000;

        ResourceDestinationSettings resourceBundleResourceGroupDestinationSettings = { CarbonResources::ResourceDestinationType::LOCAL_RELATIVE, "BundleOut/" };
		
        StatusCallback statusCallback = nullptr;

    	std::chrono::seconds downloadRetrySeconds{120};
	};

    /** @struct PatchCreateParams
    *  @brief Function Parameters required for CarbonResources::ResourceGroup::CreatePatch
    *  @var PatchCreateParams::maxInputFileChunkSize
    *  Files are processed in chunks, maxInputFileChunkSize indicate the size of this chunk. Files smaller than chunk will be processed in one pass.
    *  @var PatchCreateParams::previousResourceGroup
    *  ResourceGroup containing resources from previous build.
    *  @var PatchCreateParams::resourceGroupRelativePath
    *  Relative path for output resourceGroup which will contain the diff between PatchCreateParams::previousResourceGroup and this ResourceGroup.
    *  @var PatchCreateParams::resourceGroupPatchRelativePath
    *  Relative path for output PatchResourceGroup which will contain all the patches produced.
    *  @var PatchCreateParams::patchFileRelativePathPrefix
    *  Relative path prefix for produced patch binaries. Default is "Patches/Patch" which will produce patches such as Patches/Patch.1 ...
    *  @var PatchCreateParams::resourceSourceSettingsFrom
    *  Where resources for the previous build will be sourced.
    *  @var PatchCreateParams::resourceSourceSettingsTo
    *  Where resources for the current ResourceGroup build will be sourced.
    *  @var PatchCreateParams::resourcePatchBinaryDestinationSettings
    *  Where the produced binary patches will be saved.
    *  @var PatchCreateParams::resourcePatchResourceGroupDestinationSettings
    *  Where the produced PatchResourceGroup will be saved.
    *  @var PatchCreateParams::statusCallback
    *  Optional status function callback. Callback is triggered at key status update events.
    */
    struct PatchCreateParams
	{
        uint32_t maxInputFileChunkSize = 50000000;

		ResourceGroup* previousResourceGroup = nullptr;

        std::filesystem::path resourceGroupRelativePath = "ResourceGroup.yaml";

        std::filesystem::path resourceGroupPatchRelativePath = "PatchResourceGroup.yaml";

        std::filesystem::path patchFileRelativePathPrefix = "Patches/Patch";

		ResourceSourceSettings resourceSourceSettingsPrevious = { CarbonResources::ResourceSourceType::LOCAL_RELATIVE };

		ResourceSourceSettings resourceSourceSettingsNext = { CarbonResources::ResourceSourceType::LOCAL_RELATIVE }; 

        ResourceDestinationSettings resourcePatchBinaryDestinationSettings = { CarbonResources::ResourceDestinationType::LOCAL_CDN, "PatchOut/Patches/" };
;
        ResourceDestinationSettings resourcePatchResourceGroupDestinationSettings = { CarbonResources::ResourceDestinationType::LOCAL_RELATIVE, "PatchOut/" };
	
        StatusCallback statusCallback = nullptr;

    	std::chrono::seconds downloadRetrySeconds{120};

    	std::filesystem::path indexFolder = std::filesystem::temp_directory_path() / "carbonResources" / "chunkIndexes";
    };

    /** @struct ResourceGroupImportFromFileParams
    *  @brief Function Parameters required for CarbonResources::ResourceGroup::ImportFromFile
    *  @var ResourceGroupImportFromFileParams::filename
    *  Full filename of input file.
    *  @var ResourceGroupImportFromFileParams::statusCallback
    *  Optional status function callback. Callback is triggered at key status update events.
    */
    struct ResourceGroupImportFromFileParams
	{
		std::filesystem::path filename;

        StatusCallback statusCallback = nullptr;
	};

    /** @struct ResourceGroupExportToFileParams
    *  @brief Function Parameters required for CarbonResources::ResourceGroup::ExportToFile
    *  @var ResourceGroupExportToFileParams::filename
    *  Full filename of output file. If directory doesn't exist it will be created.
    *  @var ResourceGroupExportToFileParams::outputDocumentVersion
    *  Document version to output. By default this will be latest supported by the library.
    *  @var ResourceGroupExportToFileParams::statusCallback
    *  Optional status function callback. Callback is triggered at key status update events.
    */
	struct ResourceGroupExportToFileParams
	{
		std::filesystem::path filename = "ResourceGroup.yaml";

		Version outputDocumentVersion = S_DOCUMENT_VERSION;

        StatusCallback statusCallback = nullptr;
	};

	/** @struct CreateResourceGroupFromDirectoryParams
    *  @brief Function Parameters required for CarbonResources::ResourceGroup::CreateFromDirectory
    *  @var CreateResourceGroupFromDirectoryParams::directory
    *  Input directory for which to find files.
    *  @var CreateResourceGroupFromDirectoryParams::resourceStreamThreshold
    *  Files encountered that are above this the threshold value will be streamed in. Value is in bytes, default: 10000000
    *  @var CreateResourceGroupFromDirectoryParams::outputDocumentVersion
    *  Document version to output. By default this will be latest supported by the library.
    *  @var CreateResourceGroupFromDirectoryParams::statusCallback
    *  Optional status function callback. Callback is triggered at key status update events.
    */
	struct CreateResourceGroupFromDirectoryParams
	{
		std::filesystem::path directory = "";

		uintmax_t resourceStreamThreshold = 10000000;

		Version outputDocumentVersion = S_DOCUMENT_VERSION;

        StatusCallback statusCallback = nullptr;

		std::string resourcePrefix;
	};

    class ResourceGroupImpl;    // TODO remove these from public API

    /** @class ResourceGroup
    *  @brief Contains a collection of Resources
    */
    class API ResourceGroup
    {
	protected:
        ResourceGroup( ResourceGroupImpl* impl );

        ResourceGroup( const ResourceGroup& ) = delete;

		ResourceGroupImpl* m_impl;

    public:
	    ResourceGroup();

	    virtual ~ResourceGroup();

        /// @brief Creates a Bundle from the ResourceGroup.
		/// @param params input parameters, See BundleCreateParams for more details.
        /// @see BundleResourceGroup::Unpack for information regarding bundle unpacking.
		/// @return Result see CarbonResources::Result for more details.
        Result CreateBundle( const BundleCreateParams& params ) const;

        /// @brief Creates a Patch from the ResourceGroup. This ResourceGroup is expected to be latest.
		/// @param params input parameters, See PatchCreateParams for more details.
        /// @see PatchResourceGroup::Apply for information regarding patch application.
		/// @return Result see CarbonResources::Result for more details.
        Result CreatePatch( const PatchCreateParams& params ) const;

        /// @brief Imports resource data from file.
		/// @param params input parameters, See ResourceGroupImportFromFileParams for more details.
		/// @return Result see CarbonResources::Result for more details.
        /// @note Legacy support for importing from resfileindex CSV files is included.
        Result ImportFromFile( const ResourceGroupImportFromFileParams& params ) const;

        /// @brief Exports ResourceGroup to file.
		/// @param params input parameters, See ResourceGroupExportToFileParams for more details.
		/// @return Result see CarbonResources::Result for more details.
        Result ExportToFile( const ResourceGroupExportToFileParams& params ) const;

        /// @brief Creates a ResourceGroup from a supplied directory.
		/// @param params input parameters, See CreateResourceGroupFromDirectoryParams for more details.
		/// @return Result see CarbonResources::Result for more details.
		/// @note No file filtering supported
        Result CreateFromDirectory( const CreateResourceGroupFromDirectoryParams& params );

        friend class ResourceGroupImpl;
    };

}

#endif // ResourceGroup_H