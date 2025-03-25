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

        unsigned long fileReadChunkSize = 10000000;

        ResourceDestinationSettings resourceBundleResourceGroupDestinationSettings;
	};

    struct API PatchCreateParams
	{
		unsigned int size = sizeof( PatchCreateParams );

        unsigned long maxInputFileSize = -1;

		ResourceGroup* previousResourceGroup = nullptr;

        std::filesystem::path resourceGroupRelativePath;

        std::filesystem::path resourceGroupPatchRelativePath;

        std::filesystem::path patchFileRelativePathPrefix;

		ResourceSourceSettings resourceSourceSettingsFrom;

		ResourceSourceSettings resourceSourceSettingsTo;

        ResourceDestinationSettings resourcePatchBinaryDestinationSettings;

        ResourceDestinationSettings resourcePatchResourceGroupDestinationSettings;
	};

    struct API ResourceGroupImportFromFileParams
	{
		std::filesystem::path filename;
	};


    /** @struct ResourceGroupExportToFileParams
    *  @brief This structure blah blah blah...
    *  @var ResourceGroupExportToFileParams::size
    *  This is the size
    *  @var ResourceGroupExportToFileParams::filename
    *  filename
    *  @var ResourceGroupExportToFileParams::outputDocumentVersion
    *  outputDocumentVersion
    *  @var ResourceGroupExportToFileParams::statusCallback
    *  statusCallback
    */
	struct API ResourceGroupExportToFileParams
	{
		const unsigned int size = sizeof( ResourceGroupExportToFileParams );

		std::filesystem::path filename = "";

		Version outputDocumentVersion = S_DOCUMENT_VERSION;

        std::function<void( int, const std::string& )> statusCallback = nullptr;
	};


	/** @struct CreateResourceGroupFromDirectoryParams
    *  @brief This structure blah blah blah...
    *  @var CreateResourceGroupFromDirectoryParams::size
    *  This is the size
    *  @var CreateResourceGroupFromDirectoryParams::directory
    *  directory
    *  @var CreateResourceGroupFromDirectoryParams::resourceStreamThreshold
    *  resourceStreamThreshold
    *  @var CreateResourceGroupFromDirectoryParams::outputDocumentVersion
    *  outputDocumentVersion
    *  @var CreateResourceGroupFromDirectoryParams::statusCallback
    *  statusCallback
    */
	struct API CreateResourceGroupFromDirectoryParams
	{
		const unsigned int size = sizeof( CreateResourceGroupFromDirectoryParams );

		std::filesystem::path directory = "";

		unsigned long resourceStreamThreshold = 10000000;

		Version outputDocumentVersion = S_DOCUMENT_VERSION;

        std::function<void( int, const std::string& )> statusCallback = nullptr;
	};


    class ResourceGroupImpl;    // TODO remove these from public API

    /** @class ResourceGroup
    *  @brief This class blah blah blah...
    */
    // TODO lock down things like copy constructors for these public classes
    class API ResourceGroup
    {
	protected:
        ResourceGroup( ResourceGroupImpl* impl );

		ResourceGroupImpl* m_impl;

    public:
	    ResourceGroup();

	    virtual ~ResourceGroup();

        /// @brief Create bundle from resource group
		/// @param data data which the checksum will be based on
		/// @param data_size size of data passed in
		/// @param checksum will contain the resulting checksum on success
		/// @return true on success, false on failure
		/// @note will relinquish ownership of bundle resource group
        Result CreateBundle( const BundleCreateParams& params ) const;

        /// @brief Create patch from resource group
		/// @param data data which the checksum will be based on
		/// @param data_size size of data passed in
		/// @param checksum will contain the resulting checksum on success
		/// @return true on success, false on failure
		/// @note will relinquish ownership of patch resource group
        Result CreatePatch( const PatchCreateParams& params ) const;

        /// @brief Import resource group to file
		/// @param data data which the checksum will be based on
		/// @param data_size size of data passed in
		/// @param checksum will contain the resulting checksum on success
		/// @return true on success, false on failure
		/// @note will relinquish ownership of patch resource group
        Result ImportFromFile( const ResourceGroupImportFromFileParams& params ) const;

        /// @brief Export resource group to file
		/// @param data data which the checksum will be based on
		/// @param data_size size of data passed in
		/// @param checksum will contain the resulting checksum on success
		/// @return true on success, false on failure
		/// @note will relinquish ownership of patch resource group
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