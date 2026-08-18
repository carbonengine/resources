// Copyright © 2025 CCP ehf.

#pragma once
#ifndef ResourceGroup_H
#define ResourceGroup_H

#include "Exports.h"
#include "Enums.h"
#include <memory>
#include <string>
#include <filesystem>
#include <functional>
#include <thread>


namespace CarbonResources
{

class ResourceGroup;
class PatchResourceGroup;
class BundleResourceGroup;
struct Result;

/** 
 * @brief Default file stream size.
 * 
 * 20mb index aligned.
 */
const uintmax_t DEFAULT_FILE_STREAM_SIZE = 20971520;

/** 
 * @brief Default resource stream threshold size.
 * 
 * 500mb index aligned.
 */
const uintmax_t DEFAULT_STREAM_THRESHOLD_SIZE = 524288000;

/** @struct CallbackSettings
    *  @brief Parameters relating to status callback.
    *  @var CallbackSettings::statusCallback
    *  Optional status function callback. Callback is triggered at status update events.
    *  @var CallbackSettings::verbosityLevel
    *  Level of verbosity for status updates. Number represents nesting level of jobs. Use -1 to register for all updates.
    */
struct CallbackSettings
{
	StatusCallback statusCallback = nullptr;
	int verbosityLevel = -1;
};

/** Download Callback function signature.
    * @param totalSizeBytes Total size in bytes of file being downloaded.
    * @param currentlyDownloadedBytes Current size of data downloaded in bytes.
    * @param bytesPerSecond Current transfer rate in bytes per second.
    */
using DownloadCallback = std::function<void( uintmax_t totalSizeBytes, uintmax_t currentlyDownloadedBytes, double bytesPerSecond )>;

/** @struct DownloadSettings
    *  @brief Parameters relating downloading
    *  @var DownloadSettings::retrySeconds
    *  Delay before a failed download is retried (seconds)
    *  @var DownloadSettings::retryCount
    *  Number of times times a download is retried before failure. Note: a backoff is also applied before retry.
    *  @var DownloadSettings::downloadInfoCallback
    *  Optional callback to receive download information.
    */
struct DownloadSettings
{
	std::chrono::seconds retrySeconds{ 1 };

	uintmax_t retryCount = 3;

    DownloadCallback downloadInfoCallback = nullptr;
};

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

/** @struct AsyncSettings
    *  @brief Settings related to async processing
    *  @var AsyncSettings::numberOfThreads
    *  Number of threads to use for async processes, passing 0 will run all on main thread
    */
struct AsyncSettings
{
	uint32_t numberOfThreads = std::thread::hardware_concurrency();
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
    *  @see BundleCreateParams::fileReadChunkSize
    *  Target size of chunks to break files into. Note that chunk size may not exactly match value, the exact size is linked to BundleCreateParams::fileReadChunkSize. Value representation is in bytes
    *  @var BundleCreateParams::fileReadChunkSize
    *  Size of chunks to read files in.
    *  @var BundleCreateParams::resourceBundleResourceGroupDestinationSettings
    *  Where to save the resulting BundleResourceGroup
    *  @var BundleCreateParams::callbackSettings
    *  Settings relating to status callback messaging
    *  @var BundleCreateParams::downloadSettings
    *  Settings related to downloads
    *  @var BundleCreateParams::calculateCompressions
    *  Specifies if compression will be calculated for the generated bundle chunks
    *  @var BundleCreateParams::splitOnCompressedSize
    *  Chunks are split based on the compressed size rather than uncompressed, this gives the best chunk compression.
    *  @var BundleCreateParams::asyncSettings
    *  Settings related to async setup
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

	CallbackSettings callbackSettings;

	DownloadSettings downloadSettings;

    bool calculateCompressions = true;

    bool splitOnCompressedSize = true;

    AsyncSettings asyncSettings;
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
    *  @var PatchCreateParams::resourceGroupNewFilesRelativePath
    *  Relative path for output NewFilesResourceGroup which will contain all the new files produced.
    *  @var PatchCreateParams::patchFileRelativePathPrefix
    *  Relative path prefix for produced patch binaries. Default is "Patches/Patch" which will produce patches such as Patches/Patch.1 ...
    *  @var PatchCreateParams::resourceSourceSettingsPrevious
    *  Where resources for the previous build will be sourced.
    *  @var PatchCreateParams::resourceSourceSettingsNext
    *  Where resources for the current ResourceGroup build will be sourced.
    *  @var PatchCreateParams::resourcePatchBinaryDestinationSettings
    *  Where the produced binary patches will be saved.
    *  @var PatchCreateParams::resourcePatchResourceGroupDestinationSettings
    *  Where the produced PatchResourceGroup will be saved.
    *  @var PatchCreateParams::resourceNewFilesResourceGroupDestinationSettings
    *  Where the produced NewFilesResourceGroup will be saved.
    *  @var PatchCreateParams::callbackSettings
    *  Settings relating to status callback messaging
    *  @var PatchCreateParams::downloadSettings
    *  Settings related to downloads
    *  @var PatchCreateParams::indexFolder
    *  Directory to store index calculation files during patch creation.
    *  @var PatchCreateParams::calculateCompressions
    *  Specifies if compression will be calculated for the generated bundle chunks
    *  @var PatchCreateParams::maxTotalPatchSize
    *  The maximum size of total patch data, if exceeded the process will fail, a value of zero will be treated as unlimited.
    */
struct PatchCreateParams
{
	uint32_t maxInputFileChunkSize = 50000000;

	ResourceGroup* previousResourceGroup = nullptr;

	std::filesystem::path resourceGroupRelativePath = "ResourceGroup.yaml";

	std::filesystem::path resourceGroupPatchRelativePath = "PatchResourceGroup.yaml";

    std::filesystem::path resourceGroupNewFilesRelativePath = "NewFilesResourceGroup.yaml";

	std::filesystem::path patchFileRelativePathPrefix = "Patches/Patch";

	ResourceSourceSettings resourceSourceSettingsPrevious = { CarbonResources::ResourceSourceType::LOCAL_RELATIVE };

	ResourceSourceSettings resourceSourceSettingsNext = { CarbonResources::ResourceSourceType::LOCAL_RELATIVE };

	ResourceDestinationSettings resourcePatchBinaryDestinationSettings = { CarbonResources::ResourceDestinationType::LOCAL_CDN, "PatchOut/Patches/" };
	
	ResourceDestinationSettings resourcePatchResourceGroupDestinationSettings = { CarbonResources::ResourceDestinationType::LOCAL_RELATIVE, "PatchOut/" };

    ResourceDestinationSettings resourceNewFilesResourceGroupDestinationSettings = { CarbonResources::ResourceDestinationType::LOCAL_RELATIVE, "PatchOut/" };

	CallbackSettings callbackSettings;

	DownloadSettings downloadSettings;

	std::filesystem::path indexFolder = std::filesystem::temp_directory_path() / "carbonResources" / "chunkIndexes";

    bool calculateCompressions = true;

    uint32_t maxTotalPatchSize = 0;
};

/** @struct ResourceGroupImportFromFileParams
    *  @brief Function Parameters required for CarbonResources::ResourceGroup::ImportFromFile
    *  @var ResourceGroupImportFromFileParams::filename
    *  Full filename of input file.
    *  @var ResourceGroupImportFromFileParams::callbackSettings
    *  Settings relating to status callback messaging
    */
struct ResourceGroupImportFromFileParams
{
	std::filesystem::path filename;

	CallbackSettings callbackSettings;
};

/** @struct ResourceGroupExportToFileParams
    *  @brief Function Parameters required for CarbonResources::ResourceGroup::ExportToFile
    *  @var ResourceGroupExportToFileParams::filename
    *  Full filename of output file. If directory doesn't exist it will be created.
    *  @var ResourceGroupExportToFileParams::outputDocumentVersion
    *  Document version to output. By default this will be latest supported by the library.
    *  @var ResourceGroupExportToFileParams::callbackSettings
    *  Settings relating to status callback messaging
    */
struct ResourceGroupExportToFileParams
{
	std::filesystem::path filename = "ResourceGroup.yaml";

	Version outputDocumentVersion = S_DOCUMENT_VERSION;

    CallbackSettings callbackSettings;
};

/** @struct CreateResourceGroupFromDirectoryParams
    *  @brief Function Parameters required for CarbonResources::ResourceGroup::CreateFromDirectory
    *  @var CreateResourceGroupFromDirectoryParams::directory
    *  Input directory for which to find files.
    *  @var CreateResourceGroupFromDirectoryParams::resourceStreamThreshold
    *  Files encountered that are above this the threshold value will be streamed in. Value is in bytes, default: 10000000
    *  @var CreateResourceGroupFromDirectoryParams::outputDocumentVersion
    *  Document version to output. By default this will be latest supported by the library.
    *  @var CreateResourceGroupFromDirectoryParams::callbackSettings
    *  Settings relating to status callback messaging
    *  @var CreateResourceGroupFromDirectoryParams::resourcePrefix
    *  Resource prefix setting, e.g. res.
    *  @var CreateResourceGroupFromDirectoryParams::calculateCompressions
    *  Specifies if compression will be calculated for the generated bundle chunks
    *  @var CreateResourceGroupFromDirectoryParams::exportResources
    *  Specifies if resources will be exported
    *  @see CreateResourceGroupFromDirectoryParams::exportResourcesDestinationSettings
    *  @var CreateResourceGroupFromDirectoryParams::exportResourcesDestinationSettings
    *  If export resources is set, specifies where the produced PatchResourceGroup will be saved.
    *  @see CreateResourceGroupFromDirectoryParams::exportResources
    *  @var CreateResourceGroupFromDirectoryParams::fileStreamChunkSize
    *  Chunk size to stream in data
    *  @see CreateResourceGroupFromDirectoryParams::resourceStreamThreshold
    */
struct CreateResourceGroupFromDirectoryParams
{
	std::filesystem::path directory = "";

	uintmax_t resourceStreamThreshold = DEFAULT_STREAM_THRESHOLD_SIZE;

	Version outputDocumentVersion = S_DOCUMENT_VERSION;

	CallbackSettings callbackSettings;

	std::string resourcePrefix = "";

    bool calculateCompressions = true;

    bool exportResources = false;

    ResourceDestinationSettings exportResourcesDestinationSettings = { CarbonResources::ResourceDestinationType::LOCAL_CDN, "ExportedResources" };

    uintmax_t fileStreamChunkSize = DEFAULT_FILE_STREAM_SIZE;
};

/** @struct Filter
    *  @brief Function Parameters related to filters
    *  @var FilterSettings::filterFilePaths
    *  Paths to filter files to apply
    *  @var FilterSettings::outputResourceGroup
    *  Output resource group to populate with filtered resources
    */
struct Filter
{
	std::vector<std::filesystem::path> filterFilePaths = {};

	ResourceGroup* outputResourceGroup = nullptr;
};

/** @struct FilterSettings
    *  @brief Function Parameters related to filtering
    *  @var FilterSettings::filters
    *  Filters to apply
    *  @var FilterSettings::prefixMapBasePath
    *  Base directory used for prefixmap in filter files, refer to filter file spec.
    */
struct FilterSettings
{
	std::vector<std::unique_ptr<Filter>> filters = {};

	std::filesystem::path prefixMapBasePath = "";
};

/** @struct ExportResourceSettings
    *  @brief Function Parameters related to resource exporting
    *  @var ExportResourceSettings::enabled
    *  Specifies if export is turned on
    *  @var ExportResourceSettings::destinationSettings
    *  Destination settings related to exported resource
    */
struct ExportResourceSettings
{
	bool enabled = false;

	ResourceDestinationSettings destinationSettings = {
		CarbonResources::ResourceDestinationType::LOCAL_CDN,
		"ExportedResources"
	};
};

/** @struct CompressionCalculationSettings
    *  @brief Function Parameters related to resource compression calculation
    *  @var CompressionCalculationSettings::calculateCompressions
    *  Specifies if compression will be calculated for resoures
    *  @var CompressionCalculationSettings::remoteUrlToAttemptToGetCompression
    *  Path to remote location to attempt to get compression information
    *  @var CompressionCalculationSettings::downloadSettings
    *  Download settings used, related to CompresionCalculationSettings::remoteUrlToAttemptToGetCompression
    */
struct CompressionCalculationSettings
{
	bool calculateCompressions = true;

	std::filesystem::path remoteUrlToAttemptToGetCompression = "";

	DownloadSettings downloadSettings;
};

/** @struct CreateResourceGroupFromFilterParams
    *  @brief Function Parameters required for CarbonResources::ResourceGroup::CreateFromFilters
    *  @var CreateResourceGroupFromFilterParams::fileStreamChunkSize
    *  Chunk size to stream in data
    *  @var CreateResourceGroupFromFilterParams::outputDocumentVersion
    *  Document version to output. By default this will be latest supported by the library.
    *  @var CreateResourceGroupFromFilterParams::callbackSettings
    *  Settings relating to status callback messaging
    *  @var CreateResourceGroupFromFilterParams::resourcePrefix
    *  Resource prefix setting, e.g. res.
    *  @var CreateResourceGroupFromFilterParams::skipNonExistentInputDirectories
    *  If true will skip filter source directories that don't exist rather than error.
    *  @var CreateResourceGroupFromFilterParams::compressionCalculationSettings
    *  Settings related to compression calculations
    *  @var CreateResourceGroupFromFilterParams::exportSettings
    *  Settings related to exporting
    *  @var CreateResourceGroupFromFilterParams::filterSettings
    *  Settings related to filtering
    *  @var CreateResourceGroupFromFilterParams::calculateBinaryOperation
    *  Set true to include calculation of binary operation
    *  @var CreateResourceGroupFromFilterParams::asyncSettings
    *  Settings related to async setup
    */
struct CreateResourceGroupFromFilterParams
{
	uintmax_t fileStreamChunkSize = DEFAULT_FILE_STREAM_SIZE;

	Version outputDocumentVersion = S_DOCUMENT_VERSION;

	CallbackSettings callbackSettings;

	std::string resourcePrefix = "";

	bool skipNonExistentInputDirectories = false;

	CompressionCalculationSettings compressionCalculationSettings;

	ExportResourceSettings exportSettings;

	FilterSettings filterSettings;

    bool calculateBinaryOperation = true;

    AsyncSettings asyncSettings;
};

/** @struct ResourceGroupMergeParams
    *  @brief Function Parameters required for CarbonResources::ResourceGroup::Merge
    *  @var ResourceGroupMergeParams::resourceGroupToMerge
    *  ResourceGroup to merge
    *  @var ResourceGroupMergeParams::mergedResourceGroup
    *  Resulting ResourceGroup after merge
    *  @var ResourceGroupMergeParams::callbackSettings
    *  Settings relating to status callback messaging
    */
struct ResourceGroupMergeParams
{
	ResourceGroup* resourceGroupToMerge = nullptr;

	ResourceGroup* mergedResourceGroup = nullptr;

    CallbackSettings callbackSettings;
};

/** @struct ResourceGroupRemoveResourcesParams
    *  @brief Function Parameters required for CarbonResources::ResourceGroup::RemoveResources
    *  @var ResourceGroupRemoveResourcesParams::resourcesToRemove
    *  List of Resources to remove identified by RelativePath.
    *  @var ResourceGroupRemoveResourcesParams::errorIfResourceNotFound
    *  If true the function will return an error state if supplied Resource is not present in ResourceGroup
    *  @var ResourceGroupRemoveResourcesParams::callbackSettings
    *  Settings relating to status callback messaging
    */
struct ResourceGroupRemoveResourcesParams
{
	std::vector<std::filesystem::path>* resourcesToRemove = nullptr;

	bool errorIfResourceNotFound = true;

    CallbackSettings callbackSettings;
};

/** @struct ResourceGroupDiffAgainstGroupParams
    *  @brief Function Parameters required for CarbonResources::ResourceGroup::DiffAgainstGroup
    *  @var ResourceGroupDiffAgainstGroupParams::resourceGroupToDiffAgainst
    *  Resource group to diff against
    *  @var ResourceGroupDiffAgainstGroupParams::additions
    *  Output list of relative paths that have been added or modified on second group.
    *  @var ResourceGroupDiffAgainstGroupParams::subtractions
    *  Output list of relative paths that have been removed on second group.
    *  @var ResourceGroupDiffAgainstGroupParams::callbackSettings
    *  Settings relating to status callback messaging
    */
struct ResourceGroupDiffAgainstGroupParams
{
	ResourceGroup* resourceGroupToDiffAgainst = nullptr;

	std::vector<std::filesystem::path>* additions = nullptr;

	std::vector<std::filesystem::path>* subtractions = nullptr;

    CallbackSettings callbackSettings;
};

/** @class ResourceGroup
    *  @brief Contains a collection of Resources
    */
class API ResourceGroup
{
public:
	class ResourceGroupImpl;

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

    /// @brief Creates a filtered ResourceGroup.
	/// @param params input parameters, See CreateResourceGroupFromFilterParams for more details.
	/// @return Result see CarbonResources::Result for more details.
	static Result CreateFromFilter( const CreateResourceGroupFromFilterParams& params );

	/// @brief Merges a supplied ResourceGroup with this one. Merge performed on RelativePath, merge ResourceGroup takes precedent.
	/// @param params input parameters, See ResourceGroupMergeParams for more details.
	/// @return Result see CarbonResources::Result for more details.
	Result Merge( const ResourceGroupMergeParams& params ) const;

	/// @brief Diffs two supplied ResourceGroups.
	/// @param params input parameters, See ResourceGroupDiffAgainstGroupParams for more details.
	/// @return Result see CarbonResources::Result for more details.
	Result DiffAgainstGroup( const ResourceGroupDiffAgainstGroupParams& params ) const;

	/// @brief Removes Resources from ResourceGroup. Resources to remove are supplied are identified from a vector of RelativePaths.
	/// @param params input parameters, See CreateResourceGroupFromDirectoryParams for more details.
	/// @return Result see CarbonResources::Result for more details.
	/// @note No file filtering supported
	Result RemoveResources( const ResourceGroupRemoveResourcesParams& params ) const;
};

}

#endif // ResourceGroup_H