// Copyright © 2025 CCP ehf.

#include "ResourceGroupImpl.h"

#include <sstream>
#include <yaml-cpp/yaml.h>
#include <ResourceTools.h>
#include <BundleStreamOut.h>
#include <FileDataStreamIn.h>
#include <CompressedFileDataStreamOut.h>
#include <Md5ChecksumStream.h>
#include <GzipCompressionStream.h>
#include <cctype>
#include <algorithm>

#include "ResourceInfo/PatchResourceGroupInfo.h"
#include "ResourceInfo/BundleResourceGroupInfo.h"
#include "ResourceInfo/ResourceGroupInfo.h"
#include "ResourceInfo/PatchResourceInfo.h"
#include "Patching.h"
#include "PatchResourceGroupImpl.h"
#include "BundleResourceGroupImpl.h"
#include "ChunkIndex.h"
#include "ResourceGroupFactory.h"
#include "ResourceFilter.h"
#include "ThreadPool.h"

namespace CarbonResources
{


ResourceGroup::ResourceGroupImpl::ResourceGroupImpl()
{
	m_versionParameter = VersionInternal( S_DOCUMENT_VERSION );

	m_type = TypeId();

	m_numberOfResources = 0;

	m_totalResourcesSizeCompressed = 0;

	m_totalResourcesSizeUncompressed = 0;
}

ResourceGroup::ResourceGroupImpl::~ResourceGroupImpl()
{
	for( ResourceInfo* resourceInfo : m_resourcesParameter )
	{
		delete resourceInfo;
	}
}

Result ResourceGroup::ResourceGroupImpl::CreateFromDirectory( const CreateResourceGroupFromDirectoryParams& params, StatusSettings& statusSettings )
{
	// Update status
	statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 0, 10, "Creating resource group from directory: " + params.directory.string() );

	if( !std::filesystem::exists( params.directory ) )
	{
		return Result{ ResultType::INPUT_DIRECTORY_DOESNT_EXIST };
	}

	// Ensure document version is valid
	VersionInternal documentVersion( params.outputDocumentVersion );

	if( !documentVersion.isVersionValid() )
	{
		return Result{ ResultType::DOCUMENT_VERSION_UNSUPPORTED };
	}

	// Walk directory and create a resource from each file using data
	auto recursiveDirectoryIter = std::filesystem::recursive_directory_iterator( params.directory );

    {
		StatusSettings fileProcessingInnerStatusSettings;
		statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 10, 90, "Processing Files", &fileProcessingInnerStatusSettings );

		for( const std::filesystem::directory_entry& entry : recursiveDirectoryIter )
		{
			if( entry.is_regular_file() )
			{
				// Update status
				fileProcessingInnerStatusSettings.Update( CarbonResources::StatusProgressType::UNBOUNDED, 0, 0, "Processing File: " + entry.path().string() );

				// Create resource
				auto fileSize = entry.file_size();

				if( fileSize < params.resourceStreamThreshold )
				{
					// Create resource from data
					ResourceInfoParams resourceParams;

					resourceParams.relativePath = std::filesystem::relative( entry.path(), params.directory );

					resourceParams.binaryOperation = ResourceTools::CalculateBinaryOperation( entry.path() );

					resourceParams.prefix = params.resourcePrefix;

					ResourceInfo* resource = new ResourceInfo( resourceParams );

					std::string resourceData;

					ResourceGetDataParams resourceGetDataParams;

					resourceGetDataParams.resourceSourceSettings.basePaths = { params.directory };

					resourceGetDataParams.resourceSourceSettings.sourceType = ResourceSourceType::LOCAL_RELATIVE;

					resourceGetDataParams.data = &resourceData;

					Result getResourceDataResult = resource->GetData( resourceGetDataParams );

					if( getResourceDataResult.type != ResultType::SUCCESS )
					{
						return getResourceDataResult;
					}

					Result setParametersFromDataResult = resource->SetParametersFromData( resourceData, params.calculateCompressions );

					if( setParametersFromDataResult.type != ResultType::SUCCESS )
					{
						return setParametersFromDataResult;
					}

					Result addResourceResult = AddResource( resource );

					if( addResourceResult.type != ResultType::SUCCESS )
					{
						return addResourceResult;
					}

					// If resources are set to be exported, then export as specified
					if( params.exportResources )
					{
						ResourcePutDataParams putDataParams;

						putDataParams.resourceDestinationSettings = params.exportResourcesDestinationSettings;

						putDataParams.data = &resourceData;

						Result putDataResult = resource->PutData( putDataParams );

						if( putDataResult.type != ResultType::SUCCESS )
						{
							return putDataResult;
						}
					}
				}
				else
				{
					// Process data via stream
					ResourceTools::Md5ChecksumStream checksumStream;
					std::string compressedData;

					ResourceTools::GzipCompressionStream compressionStream( &compressedData );

					ResourceTools::FileDataStreamIn fileStreamIn( params.fileStreamChunkSize );

					if( params.calculateCompressions )
					{
						if( !compressionStream.Start() )
						{
							return Result{ ResultType::FAILED_TO_COMPRESS_DATA };
						}
					}

					if( !fileStreamIn.StartRead( entry.path() ) )
					{
						return Result{ ResultType::FAILED_TO_OPEN_FILE_STREAM };
					}

					uintmax_t compressedDataSize = 0;

					while( !fileStreamIn.IsFinished() )
					{
						// Update status
						if( fileProcessingInnerStatusSettings.RequiresStatusUpdates() )
						{
							float step = static_cast<float>( 100.0 / fileStreamIn.Size() );
							float percentage = static_cast<float>( fileStreamIn.GetCurrentPosition() * step );
							fileProcessingInnerStatusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, percentage, step, "Percentage Update" );
						}

						std::string fileData;

						if( !( fileStreamIn >> fileData ) )
						{
							return Result{ ResultType::FAILED_TO_READ_FROM_STREAM };
						}

						if( !( checksumStream << fileData ) )
						{
							return Result{ ResultType::FAILED_TO_GENERATE_CHECKSUM };
						}

						if( params.calculateCompressions )
						{
							if( !( compressionStream << &fileData ) )
							{
								return Result{ ResultType::FAILED_TO_COMPRESS_DATA };
							}
						}

						compressedDataSize += compressedData.size();
						compressedData.clear();
					}

					if( params.calculateCompressions )
					{
						if( !compressionStream.Finish() )
						{
							return Result{ ResultType::FAILED_TO_COMPRESS_DATA };
						}

						compressedDataSize += compressedData.size();
						compressedData.clear();
					}

					std::string checksum;

					if( !checksumStream.Retrieve( checksum ) )
					{
						return Result{ ResultType::FAILED_TO_GENERATE_CHECKSUM };
					}

					// Create resource from parameters
					ResourceInfoParams resourceParams;

					resourceParams.relativePath = std::filesystem::relative( entry.path(), params.directory );

					resourceParams.uncompressedSize = fileSize;

					resourceParams.compressedSize = compressedDataSize;

					resourceParams.checksum = checksum;

					resourceParams.binaryOperation = ResourceTools::CalculateBinaryOperation( entry.path() );

					Location l;

					Result calculateLocationResult = l.SetFromRelativePathAndDataChecksum( resourceParams.relativePath, resourceParams.checksum );

					if( calculateLocationResult.type != ResultType::SUCCESS )
					{
						return calculateLocationResult;
					}

					resourceParams.location = l.ToString();

					ResourceInfo* resource = new ResourceInfo( resourceParams );

					Result addResourceResult = AddResource( resource );

					if( addResourceResult.type != ResultType::SUCCESS )
					{
						return addResourceResult;
					}

					// If resources are set to be exported, then export as specified.
					// This is slow with large files as each need to be streamed again
					// The problem is that checksum of the whole file needs to be calculated first
					// in order to get the correct destination CDN path
					// If compression is not skipped and REMOTE_CDN is chosen as destination then
					// compression will also be calculated twice.
					// This can be improved with a refactor but currently this code path not
					// likely to be relied upon often
					if( params.exportResources )
					{
						ResourcePutDataStreamParams putDataStreamParams;

						// Create the correct file data streaming for the desination
						std::unique_ptr<ResourceTools::FileDataStreamOut> resourceDataStreamOut;

						if( params.exportResourcesDestinationSettings.destinationType == ResourceDestinationType::REMOTE_CDN )
						{
							// REMOTE_CDN requires compression
							resourceDataStreamOut = std::make_unique<ResourceTools::CompressedFileDataStreamOut>();
						}
						else
						{
							// Else just stream out uncompressed
							resourceDataStreamOut = std::make_unique<ResourceTools::FileDataStreamOut>();
						}

						putDataStreamParams.resourceDestinationSettings = params.exportResourcesDestinationSettings;

						putDataStreamParams.dataStream = resourceDataStreamOut.get();

						Result putDataStreamResult = resource->PutDataStream( putDataStreamParams );

						if( putDataStreamResult.type != ResultType::SUCCESS )
						{
							return putDataStreamResult;
						}

						// Export resource using streaming
						ResourceTools::FileDataStreamIn fileStreamIn( params.resourceStreamThreshold );

						if( !fileStreamIn.StartRead( entry.path() ) )
						{
							return Result{ ResultType::FAILED_TO_OPEN_FILE_STREAM };
						}

						while( !fileStreamIn.IsFinished() )
						{
							std::string data = "";

							if( !( fileStreamIn >> data ) )
							{
								return Result{ ResultType::FAILED_TO_READ_FROM_STREAM };
							}

							if( !( resourceDataStreamOut->operator<<( data ) ) )
							{
								return Result{ ResultType::FAILED_TO_SAVE_TO_STREAM };
							}
						}

						if( !resourceDataStreamOut->Finish() )
						{
							return Result{ ResultType::FAILED_TO_SAVE_TO_STREAM };
						}
					}
				}
			}
		}

		if( !params.calculateCompressions )
		{
			m_totalResourcesSizeCompressed.Reset();
		}
	}

	return Result{ ResultType::SUCCESS };
}

// Thread structs for filtering
struct ResourceGroupFromFilterPoolArguments
{
	bool backoff = false;

	std::mutex backoffMutex;

	std::condition_variable backoffConditional;

	std::mutex resourceGroupMutex;

    std::mutex checkedRelativePathsMutex;

    std::map<std::string, int> checkedRelativePaths;

	std::vector<std::shared_ptr<FilterGroup>> filterGroups;

	bool ignoreCase;

	const CreateResourceGroupFromFilterParams* params;

    Result status = Result{ ResultType::SUCCESS };

    bool checkForDuplicateRelativePaths = false;

    int totalNumberOfResources = 0;

    std::atomic<int> numberOfResourcesProcessed = 0;

    std::atomic<int> numberOfResourcesSkipped = 0;

    std::atomic<int> numberOfResourcesCompressed = 0;

    std::mutex statusUpdateMutex;

    StatusSettings statusSettings;

    void RunStatusUpdate()
    {
        if (statusSettings.RequiresStatusUpdates())
        {
			{
				std::unique_lock<std::mutex> lock( statusUpdateMutex );
                if ((numberOfResourcesProcessed % 1000) == 0)
                {
				
				        float step = static_cast<float>( 100.0 / totalNumberOfResources );
				        float progress = step * numberOfResourcesProcessed;

                        std::stringstream ss;

                        ss << "Processed: " << std::to_string( numberOfResourcesProcessed )
				           << ", Skipped: " << std::to_string( numberOfResourcesSkipped )
				           << ", Compressed:" << std::to_string( numberOfResourcesCompressed );

                        statusSettings.Update( StatusProgressType::PERCENTAGE, progress, step, ss.str() );
				}
            }
        }
    }

    void ShowStatusWarning(const std::string& warningString)
    {
		{
			std::unique_lock<std::mutex> lock( statusUpdateMutex );

            statusSettings.Update( StatusProgressType::WARNING, 0, 0, warningString );
		}
    }
};

struct ResourceGroupFromFilterThreadArguments
{
	std::filesystem::path inputDirectory;

	std::vector<std::filesystem::directory_entry> entryPaths;

	std::filesystem::path searchPath;

    ResourceTools::Downloader downloader;
};

void CreateResourceGroupsFromFilterWorker( std::shared_ptr<ResourceGroupFromFilterPoolArguments> commonArguments, std::shared_ptr<ResourceGroupFromFilterThreadArguments> threadArguments )
{
	ResourceTools::FileDataStreamIn fileStreamIn( commonArguments->params->fileStreamChunkSize );

	ResourceTools::Md5ChecksumStream checksumStream;

	for( auto entry : threadArguments->entryPaths )
	{

		commonArguments->RunStatusUpdate();

		std::filesystem::path entryPath = entry.path();

		uintmax_t entrySize = entry.file_size();

        commonArguments->numberOfResourcesProcessed++;

		// Process file
		// Check each filter group
		std::vector<std::shared_ptr<FilterGroup>> matchedFilters;

		std::filesystem::path filePathRelativeToInputDirectory = std::filesystem::relative( entryPath, commonArguments->params->filterSettings.prefixMapBasePath );

		std::string normalisedRelativePath = filePathRelativeToInputDirectory.generic_string();

		// Filtering is not case sensitive
		if( commonArguments->ignoreCase )
		{
			std::transform( normalisedRelativePath.begin(), normalisedRelativePath.end(), normalisedRelativePath.begin(), []( unsigned char c ) { return std::tolower( c ); } );
		}

		for( auto& filterGroup : commonArguments->filterGroups )
		{

			// Check each filter in filter group
			for( auto& filter : filterGroup->filters )
			{

				// Check that the current search path is relevant to the current filter
				const std::vector<std::filesystem::path>& filterSearchPaths = filter->GetPrefixPaths();

				bool searchPathInFilter = false;

				for( auto& filterPath : filterSearchPaths )
				{
					if( threadArguments->searchPath == filterPath )
					{
						searchPathInFilter = true;
						break;
					}
				}

				if( !searchPathInFilter )
				{
					// The current search path isn't one that is present in the current filter
					continue;
				}

				if( filter->CheckPath( normalisedRelativePath ) )
				{
					matchedFilters.push_back( filterGroup );

					break;
				}
			}
		}

		// Check if any filters were matched
		if( matchedFilters.empty() )
		{
			//File doesn't meet any of the filters
            commonArguments->numberOfResourcesSkipped++;

            continue;
		}

		// Process the file data via streaming
		checksumStream.Start();

		std::filesystem::path resourceRelativePath = std::filesystem::relative( entryPath, threadArguments->inputDirectory );

		std::string resourceRelativePathString = resourceRelativePath.generic_string();

		if( commonArguments->ignoreCase )
		{
			std::transform( resourceRelativePathString.begin(), resourceRelativePathString.end(), resourceRelativePathString.begin(), []( unsigned char c ) { return std::tolower( c ); } );
		}

        // Check to ensure path hasn't been checked before
        bool duplicateFound = false;

        if (commonArguments->checkForDuplicateRelativePaths)
        {
			{
				std::unique_lock<std::mutex> lock( commonArguments->checkedRelativePathsMutex );
				duplicateFound = commonArguments->checkedRelativePaths.find( resourceRelativePathString ) != commonArguments->checkedRelativePaths.end();
			}
        }

        if (duplicateFound)
        {
			commonArguments->numberOfResourcesSkipped++;
			continue;
        }

        // Add path to checked paths
		{
			std::unique_lock<std::mutex> lock( commonArguments->checkedRelativePathsMutex );
			commonArguments->checkedRelativePaths[resourceRelativePathString] = 1;
		}

		if( !fileStreamIn.StartRead( entryPath ) )
		{
			{
				std::unique_lock<std::mutex> lock( commonArguments->resourceGroupMutex );
				commonArguments->status = Result{ ResultType::FAILED_TO_OPEN_FILE_STREAM };
			}
			return;
		}

		// Calculate without compression to get checksum
		while( !fileStreamIn.IsFinished() )
		{
			std::string fileData;

			if( !( fileStreamIn >> fileData ) )
			{
				{
					std::unique_lock<std::mutex> lock( commonArguments->resourceGroupMutex );
					commonArguments->status = Result{ ResultType::FAILED_TO_READ_FROM_STREAM };
				}
				return;
			}

			if( !( checksumStream << fileData ) )
			{
				{
					std::unique_lock<std::mutex> lock( commonArguments->resourceGroupMutex );
					commonArguments->status = Result{ ResultType::FAILED_TO_GENERATE_CHECKSUM };
				}
				return;
			}
		}

		// Get Checksum
		std::string checksum;

		if( !checksumStream.Retrieve( checksum ) )
		{
			{
				std::unique_lock<std::mutex> lock( commonArguments->resourceGroupMutex );
				commonArguments->status = Result{ ResultType::FAILED_TO_GENERATE_CHECKSUM };
			}
			return;
		}

		// Create resource from parameters
		ResourceInfoParams resourceParams;

		resourceParams.relativePath = resourceRelativePathString;

		resourceParams.prefix = commonArguments->params->resourcePrefix;

		resourceParams.uncompressedSize = entrySize;

		resourceParams.compressedSize = 0; // If required this will be calculated later

		resourceParams.checksum = checksum;

		if( commonArguments->params->calculateBinaryOperation )
		{
			resourceParams.binaryOperation = ResourceTools::CalculateBinaryOperation( entryPath );
		}

		Location location;

		Result calculateLocationResult = location.SetFromRelativePathAndDataChecksum( resourceParams.relativePath, resourceParams.checksum, resourceParams.prefix );

		if( calculateLocationResult.type != ResultType::SUCCESS )
		{
			{
				std::unique_lock<std::mutex> lock( commonArguments->resourceGroupMutex );
				commonArguments->status = calculateLocationResult;
			}
			return;
		}

		resourceParams.location = location.ToString();

		ResourceInfo resource( resourceParams );

		// Setup export
		std::filesystem::path resourceDestinationPath;

		Result constructDestinationPathResult = resource.GetDestinationPath( commonArguments->params->exportSettings.destinationSettings, resourceDestinationPath );

		if( constructDestinationPathResult.type != ResultType::SUCCESS )
		{
			{
				std::unique_lock<std::mutex> lock( commonArguments->resourceGroupMutex );
				commonArguments->status = constructDestinationPathResult;
			}
			return;
		}

		// If further calculation is required on the data then calculate here
		if( commonArguments->params->compressionCalculationSettings.calculateCompressions || commonArguments->params->exportSettings.enabled )
		{
			if( !fileStreamIn.StartRead( entryPath ) )
			{
				{
					std::unique_lock<std::mutex> lock( commonArguments->resourceGroupMutex );
					commonArguments->status = Result{ ResultType::FAILED_TO_OPEN_FILE_STREAM };
				}
				return;
			}

			ResourceTools::FileDataStreamOut exportResourceDataStreamOut;

			std::filesystem::path resourceDestinationPath;

			Result getDestionationResult = resource.GetDestinationPath( commonArguments->params->exportSettings.destinationSettings, resourceDestinationPath );

			if( getDestionationResult.type != ResultType::SUCCESS )
			{
				{
					std::unique_lock<std::mutex> lock( commonArguments->resourceGroupMutex );
					commonArguments->status = getDestionationResult;
				}
				return;
			}

			std::string compressedData;

			uintmax_t compressedDataSize = 0;

			ResourceTools::GzipCompressionStream compressionStream( &compressedData );

			// Compression will need to be calculated if specified or an exported resource requires it
			bool calculateCompressions = commonArguments->params->compressionCalculationSettings.calculateCompressions;
			bool exportResource = commonArguments->params->exportSettings.enabled;

			// Check url for compression information if supplied
			// If argument set to skip if file already exists in destination then
			// Check for file and if to skip
			if( commonArguments->params->compressionCalculationSettings.remoteUrlToAttemptToGetCompression != "" )
			{

				std::string resourceUrl = commonArguments->params->compressionCalculationSettings.remoteUrlToAttemptToGetCompression.string() + "/" + resourceParams.location;

				// Check if file exists in remote location
				std::string response;

				ResourceTools::Response downloadResponse = ResourceTools::Response::NONE;

                {
					std::unique_lock<std::mutex> lock( commonArguments->backoffMutex );

					commonArguments->backoffConditional.wait( lock, [commonArguments] {
						return !commonArguments->backoff;
					} );
				}

                downloadResponse = threadArguments->downloader.GetHeader( resourceUrl, response );

                auto retryCount = commonArguments->params->compressionCalculationSettings.downloadSettings.retryCount;
				auto retrySeconds = commonArguments->params->compressionCalculationSettings.downloadSettings.retrySeconds;

				if( downloadResponse == ResourceTools::Response::DOWNLOAD_ERROR )
				{
                    {
						std::unique_lock<std::mutex> lock( commonArguments->backoffMutex );

                        std::string warningMessage = "Download Info: Network error, requests backing off.";

						commonArguments->ShowStatusWarning( warningMessage );

                        commonArguments->backoff = true;

						for( int i = 0; i < retryCount; i++ )
						{

							downloadResponse = threadArguments->downloader.GetHeader( resourceUrl, response );

                            std::chrono::seconds backoffTime = retrySeconds * ( i + 1 );

							if( downloadResponse != ResourceTools::Response::DOWNLOAD_ERROR )
							{
								std::string warningMessage = "Download Info: Network requests resuming.";

								commonArguments->ShowStatusWarning( warningMessage );

								commonArguments->backoff = false;

                                commonArguments->backoffConditional.notify_all();

								break;
							}
                            else
                            {
								std::stringstream ss;

                                ss << "Download Info: Network error, retry ";
								ss << i << " failed. requests backing off for ";
								ss << backoffTime.count();
								ss << " seconds.";

								std::string warningMessage = ss.str();

								commonArguments->ShowStatusWarning( warningMessage );
                            }

							std::this_thread::sleep_for( backoffTime );

						}

                        // No matter what unblock other threads anyway to continue execution.
                        // The download may have still errored at this point
                        commonArguments->backoff = false;

                        commonArguments->backoffConditional.notify_all();

					}
				}
                
				if( downloadResponse == ResourceTools::Response::SUCCESS )
				{
					// Parse the header for the content size
					std::string contentLengthStr;

					if( ResourceTools::Downloader::GetAttributeValueFromHeader( response, "Content-Length", contentLengthStr ) )
					{
						// Parse content length
						try
						{
							unsigned long in = std::stoul( contentLengthStr );
							if( in > std::numeric_limits<uint32_t>::max() )
							{
								std::string warningMessage = "Invalid compression data from header information, compression will be calculated." + resourceUrl;
								
                                commonArguments->ShowStatusWarning( warningMessage );
							}
							else
							{
								// Compression is from the existing file rather than new one.
								compressedDataSize = static_cast<uint32_t>( in );
								
                                calculateCompressions = false;
								
                                exportResource = false;
							}
						}
						catch( std::invalid_argument& )
						{
							std::string warningMessage = "Invalid compression data from header information, compression will be calculated." + resourceUrl;
							
                            commonArguments->ShowStatusWarning( warningMessage );
                        }
						catch( std::out_of_range& )
						{
							std::string warningMessage = "Invalid compression data from header information, compression will be calculated." + resourceUrl;
							
                            commonArguments->ShowStatusWarning( warningMessage );
                        }
					}
				}
				else if( downloadResponse == ResourceTools::Response::DOWNLOAD_ERROR )
				{
                    // If download error persists after set retries then the resource
                    // is treated as if new and computation continues

					std::stringstream ss;

					ss << "Error while downloading header from: "
					   << resourceUrl
					   << " After " << retryCount << " retries. Continuing and treating resource as new.";

					std::string warningMessage = ss.str();

					commonArguments->ShowStatusWarning( warningMessage );

				}
			}
			

			if( exportResource )
			{
				if( !exportResourceDataStreamOut.StartWrite( resourceDestinationPath ) )
				{
					{
						std::unique_lock<std::mutex> lock( commonArguments->resourceGroupMutex );
						commonArguments->status = Result{ ResultType::FAILED_TO_OPEN_FILE_STREAM };
					}
					return;
				}
			}

			if( ( exportResource && commonArguments->params->exportSettings.destinationSettings.destinationType == ResourceDestinationType::REMOTE_CDN ) )
			{
				calculateCompressions = true;
			}

			if( calculateCompressions )
			{
				commonArguments->numberOfResourcesCompressed++;

				if( !compressionStream.Start() )
				{
					{
						std::unique_lock<std::mutex> lock( commonArguments->resourceGroupMutex );
						commonArguments->status = Result{ ResultType::FAILED_TO_COMPRESS_DATA };
					}
					return;
				}
			}

			if( calculateCompressions || exportResource )
			{
				while( !fileStreamIn.IsFinished() )
				{


					std::string fileData;

					if( !( fileStreamIn >> fileData ) )
					{
						{
							std::unique_lock<std::mutex> lock( commonArguments->resourceGroupMutex );
							commonArguments->status = Result{ ResultType::FAILED_TO_READ_FROM_STREAM };
						}
						return;
					}

					if( calculateCompressions )
					{
						if( !( compressionStream << &fileData ) )
						{
							{
								std::unique_lock<std::mutex> lock( commonArguments->resourceGroupMutex );
								commonArguments->status = Result{ ResultType::FAILED_TO_COMPRESS_DATA };
							}
							return;
						}
					}

					// If exporting resource then export the correct data
					if( exportResource )
					{
						if( commonArguments->params->exportSettings.destinationSettings.destinationType == ResourceDestinationType::LOCAL_CDN || commonArguments->params->exportSettings.destinationSettings.destinationType == ResourceDestinationType::LOCAL_RELATIVE )
						{
							// Output Uncompressed Data
							if( !( exportResourceDataStreamOut << fileData ) )
							{
								{
									std::unique_lock<std::mutex> lock( commonArguments->resourceGroupMutex );
									commonArguments->status = Result{ ResultType::FAILED_TO_WRITE_TO_STREAM };
								}
								return;
							}
						}
						else
						{
							// Output Compressed Data
							if( !( exportResourceDataStreamOut << compressedData ) )
							{
								{
									std::unique_lock<std::mutex> lock( commonArguments->resourceGroupMutex );
									commonArguments->status = Result{ ResultType::FAILED_TO_WRITE_TO_STREAM };
								}
								return;
							}
						}
					}

					compressedDataSize += compressedData.size();
					compressedData.clear();
				}
			}

			// Finish stream read in
			fileStreamIn.Finish();

			if( calculateCompressions )
			{
				if( !compressionStream.Finish() )
				{
					{
						std::unique_lock<std::mutex> lock( commonArguments->resourceGroupMutex );
						commonArguments->status = Result{ ResultType::FAILED_TO_WRITE_TO_STREAM };
					}
					return;
				}

				// Add remaining compression data
				compressedDataSize += compressedData.size();
			}

			if( commonArguments->params->compressionCalculationSettings.calculateCompressions )
			{
				// Set compressed size only if compression is calculated
				// If export requires compression but calculation is skipped
				// Then the calculations are not set as this may be unexpected
				resource.SetCompressedSize( compressedDataSize );
			}

			if( exportResource )
			{
				if( commonArguments->params->exportSettings.destinationSettings.destinationType == ResourceDestinationType::REMOTE_CDN )
				{
					// Add remaining compression data
					if( !( exportResourceDataStreamOut << compressedData ) )
					{
						{
							std::unique_lock<std::mutex> lock( commonArguments->resourceGroupMutex );
							commonArguments->status = Result{ ResultType::FAILED_TO_WRITE_TO_STREAM };
						}
						return;
					}
				}

				if( !exportResourceDataStreamOut.Finish() )
				{
					{
						std::unique_lock<std::mutex> lock( commonArguments->resourceGroupMutex );
						commonArguments->status = Result{ ResultType::FAILED_TO_WRITE_TO_STREAM };
					}
					return;
				}
			}
		}

		{
			std::unique_lock<std::mutex> lock( commonArguments->resourceGroupMutex );

			// Add entry to all relevant resource groups
			for( auto& filterGroup : matchedFilters )
			{
				// Resource group takes ownership of resource info
				filterGroup->resourceGroup->AddResource( new ResourceInfo( resource ) );
			}
		}

	}

	return;
}

Result ResourceGroup::ResourceGroupImpl::CreateFromFilter( const CreateResourceGroupFromFilterParams& params, StatusSettings& statusSettings )
{
	ResourceTools::Downloader downloader;

    // Update status
	statusSettings.Update( StatusProgressType::PERCENTAGE, 0, 5, "Creating resource group from filters" );

    std::shared_ptr<ResourceGroupFromFilterPoolArguments> poolArguments = std::make_shared<ResourceGroupFromFilterPoolArguments>();
	poolArguments->ignoreCase = false;
	poolArguments->params = &params;

	// Ensure document version is valid
	VersionInternal documentVersion( params.outputDocumentVersion );

	if( documentVersion == VersionInternal( S_CSV_DOCUMENT_VERSION ) )
	{
		poolArguments->ignoreCase = true;
	}

	if( !documentVersion.isVersionValid() )
	{
		return Result{ ResultType::DOCUMENT_VERSION_UNSUPPORTED };
	}

	statusSettings.Update( StatusProgressType::PERCENTAGE, 5, 5, "Loading filter files" );

	// Initialise Resource Filters
	for( auto& filter : params.filterSettings.filters )
	{
		if( filter->filterFilePaths.empty() )
		{
			std::string errorMsg = "No filter files provided.";
			return Result{ ResultType::FAILED_TO_INITIALIZE_RESOURCE_FILTER, errorMsg };
		}

		std::shared_ptr<FilterGroup> filterGroup = std::make_shared<FilterGroup>();

		filterGroup->resourceGroup = filter->outputResourceGroup->m_impl;

		for( auto filterPath : filter->filterFilePaths )
		{
			// Get filter file data
			std::string filterData;

			if( !ResourceTools::GetLocalFileData( filterPath, filterData ) )
			{
				std::string errorMsg = "Failed to open filter file: " + filterPath.string();
				return Result{ ResultType::FAILED_TO_OPEN_FILE, errorMsg };
			}

			ResourceTools::FilterFile fileData;

			try
			{
				ResourceTools::FilterFileReader::LoadFromIniFileData( filterData.data(), filterData.size(), fileData, poolArguments->ignoreCase );
			}
			catch( const std::exception& e )
			{
				std::string errorMsg = "Failed to read filter file: " + std::string( e.what() );

				return Result{ ResultType::FAILED_TO_INITIALIZE_RESOURCE_FILTER, errorMsg };
			}

			std::unique_ptr<ResourceTools::ResourceFilter> filter = std::make_unique<ResourceTools::ResourceFilter>();

			if( !filter->SetFromFilterFileData( fileData ) )
			{
				return Result{ ResultType::FAILED_TO_INITIALIZE_RESOURCE_FILTER };
			}

			filterGroup->filters.push_back( std::move( filter ) );
		}

		poolArguments->filterGroups.push_back( std::move( filterGroup ) );
	}

	// Populate all search paths
	std::vector<std::filesystem::path> searchPaths;

	for( auto& filterGroup : poolArguments->filterGroups )
	{
		for( auto& filter : filterGroup->filters )
		{
			const std::vector<std::filesystem::path>& filterPaths = filter->GetPrefixPaths();

			for( auto& filterPath : filterPaths )
			{
				if( std::find( searchPaths.begin(), searchPaths.end(), filterPath ) == searchPaths.end() )
				{
					searchPaths.push_back( filterPath );
				}
			}
		}
	}

	// Process files in search paths
	{
		StatusSettings directoryStatusSettings;
		statusSettings.Update( StatusProgressType::PERCENTAGE, 10, 90, "Creating resource group from directories", &directoryStatusSettings );

		// num threads must at least be 1
		// If zero threads are passed via async settings then
		// the job is done on the main thread
		int numThreads = params.asyncSettings.numberOfThreads + 1;

		// numThreads-1 as the value passed is the number of threads to create
		// If numThreads-1==0 then the tasks for the pool will all execute on this thread
		ThreadPool<std::shared_ptr<ResourceGroupFromFilterPoolArguments>, std::shared_ptr<ResourceGroupFromFilterThreadArguments>> threadPool( numThreads - 1 );

		threadPool.Start();

		int searchPathIndex = 0;

		float statusProgressStep = static_cast<float>(100.0 / searchPaths.size());

		for( auto searchPath : searchPaths )
		{
			std::filesystem::path inputDirectory = params.filterSettings.prefixMapBasePath / searchPath;

            if (directoryStatusSettings.RequiresStatusUpdates())
            {
				float progress = statusProgressStep * searchPathIndex;
				searchPathIndex++;
				directoryStatusSettings.Update( StatusProgressType::PERCENTAGE, progress, statusProgressStep, "Checking directory: " + inputDirectory.lexically_normal().generic_string(), &poolArguments->statusSettings );
            }

			if( !std::filesystem::exists( inputDirectory ) )
			{
				if( params.skipNonExistentInputDirectories )
				{
					directoryStatusSettings.Update( StatusProgressType::WARNING, 0, 0, "Skipping input directory as it doesn't exist." );

					continue;
				}
				else
				{
					return Result{ ResultType::INPUT_DIRECTORY_DOESNT_EXIST, inputDirectory.string() };
				}
			}

			//Create thread structure
			std::vector<std::shared_ptr<ResourceGroupFromFilterThreadArguments>> threadArguments;

			for( int i = 0; i < numThreads; i++ )
			{
				std::shared_ptr<ResourceGroupFromFilterThreadArguments> arguments = std::make_shared<ResourceGroupFromFilterThreadArguments>();

				arguments->inputDirectory = inputDirectory;

				arguments->searchPath = searchPath;

				threadArguments.push_back( std::move( arguments ) );
			}

			int threadId = 0;

			// Walk directory
			auto recursiveDirectoryIter = std::filesystem::recursive_directory_iterator( inputDirectory );
			{

				// Walk entries
				for( const std::filesystem::directory_entry& entry : recursiveDirectoryIter )
				{

					if( !entry.is_regular_file() )
					{
						// Not a file
						continue;
					}

					std::filesystem::path entryPath = entry.path();

                    

					threadArguments.at( threadId )->entryPaths.push_back( entry );

                    poolArguments->totalNumberOfResources++;

					threadId++;

					if( threadId >= numThreads )
					{
						threadId = 0;
					}
				}

				for( int i = 0; i < numThreads; i++ )
				{
					threadPool.AddTask( CreateResourceGroupsFromFilterWorker, poolArguments, threadArguments.at( i ) );
				}

				threadPool.Join();

				if( poolArguments->status.type != ResultType::SUCCESS )
				{
					return poolArguments->status;
				}

				// Subsequent paths need to have an added check to ensure they have
				// not already been processed
				// this is not needed for the first path as files will never have been
				// encountered before so for efficiency is skipped.
				poolArguments->checkForDuplicateRelativePaths = true;
			}

            poolArguments->statusSettings.Update( StatusProgressType::END, 100, 0, "process complete" );
		}
	}

	return Result{ ResultType::SUCCESS };
}

Result ResourceGroup::ResourceGroupImpl::ImportFromData( const std::string& data, StatusSettings& statusSettings, DocumentType documentType /* = DocumentType::YAML */ )
{
	switch( documentType )
	{
	case DocumentType::CSV:
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4996 ) // Suppress deprecation warning.
#elif __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
		return ImportFromCSV( data, statusSettings );
#ifdef _MSC_VER
#pragma warning( pop )
#elif __APPLE__
#pragma clang diagnostic pop
#endif
	case DocumentType::YAML:
		return ImportFromYamlString( data, statusSettings );
	default:
		return Result{ ResultType::UNSUPPORTED_FILE_FORMAT };
	}

	return Result{ ResultType::FAIL };
}

Result ResourceGroup::ResourceGroupImpl::ImportFromFile( const ResourceGroupImportFromFileParams& params, StatusSettings& statusSettings )
{
	statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 0, 20, "Importing Resource Group from file." );

	if( params.filename.empty() )
	{
		return Result{ ResultType::FILE_NOT_FOUND };
	}

	std::string data;

	if( !ResourceTools::GetLocalFileData( params.filename, data ) )
	{
		return Result{ ResultType::FAILED_TO_OPEN_FILE };
	}

	// VERSION NEEDS TO BE CHECKED TO ENSURE ITS SUPPORTED ON IMPORT
	std::filesystem::path filename = params.filename;

	std::string extension = filename.extension().string();

	Result importResult;

    {
		StatusSettings nestedStatusSettings;
		statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 20, 80, "Importing Resource Group from file.", &nestedStatusSettings );

		if( extension == ".txt" )
		{
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4996 ) // Suppress deprecation warning.
#elif __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
			importResult = ImportFromCSV( data, nestedStatusSettings );
#ifdef _MSC_VER
#pragma warning( pop )
#elif __APPLE__
#pragma clang diagnostic pop
#endif
		}
		else if( extension == ".yml" || extension == ".yaml" || extension.empty() )
		{
			importResult = ImportFromYamlString( data, nestedStatusSettings );
		}
		else
		{
			return Result{ ResultType::UNSUPPORTED_FILE_FORMAT };
		}

		if( importResult.type != ResultType::SUCCESS )
		{
			return importResult;
		}
    }

    return importResult;
}

Result ResourceGroup::ResourceGroupImpl::ExportToFile( const ResourceGroupExportToFileParams& params, StatusSettings& statusSettings ) const
{
	// Update status
	statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 0, 10, "Exporting Resource Group to file: " + params.filename.string() );

	std::string data = "";

	if( params.outputDocumentVersion.major == 0 && params.outputDocumentVersion.minor == 0 )
	{
		{
			StatusSettings exportCsvstatusSettings;
			statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 10, 90, "Exporting Resource Group to file: " + params.filename.string(), &exportCsvstatusSettings );


			Result exportCsvResult = ExportCsv( params.outputDocumentVersion, data, exportCsvstatusSettings );
			if( exportCsvResult.type != ResultType::SUCCESS )
			{
				return exportCsvResult;
			}
		}
	}
	else
	{
		{
			StatusSettings exportYamlstatusSettings;
			statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 10, 90, "Exporting Resource Group to file: " + params.filename.string(), &exportYamlstatusSettings );

			Result exportYamlResult = ExportYaml( params.outputDocumentVersion, data, exportYamlstatusSettings );

			if( exportYamlResult.type != ResultType::SUCCESS )
			{
				return exportYamlResult;
			}
		}
	}

	if( !ResourceTools::SaveFile( params.filename, data ) )
	{
		return Result{ ResultType::FAILED_TO_SAVE_FILE };
	}

	return Result{ ResultType::SUCCESS };
}

Result ResourceGroup::ResourceGroupImpl::ExportToData( std::string& data, StatusSettings& statusSettings, VersionInternal outputDocumentVersion /* = S_DOCUMENT_VERSION*/ ) const
{
	Result exportYamlResult = ExportYaml( outputDocumentVersion, data, statusSettings );

	if( exportYamlResult.type != ResultType::SUCCESS )
	{
		return exportYamlResult;
	}

	return Result{ ResultType::SUCCESS };
}

Result ResourceGroup::ResourceGroupImpl::ImportFromCSV( const std::string& data, StatusSettings& statusSettings )
{
	// Status update
	statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 0, 10, "Importing Resource Group from CSV file." );

	std::stringstream inputStream;

	inputStream << data;

	std::string stringIn;

    {
		StatusSettings detailStatusSettings;
		statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 10, 90, "Importing Resource Group from CSV file.", &detailStatusSettings );

		while( !inputStream.eof() )
		{
			std::getline( inputStream, stringIn );

			if( stringIn == "" )
			{
				continue;
			}

			std::stringstream ss( stringIn );

			std::string value;

			char delimiter = ',';

			ResourceInfoParams resourceParams;

			if( !std::getline( ss, value, delimiter ) )
			{
				return Result{ ResultType::MALFORMED_RESOURCE_INPUT };
			}

			// Split filename and prefix
			std::string resourcePrefixDelimiter = ":/";
			std::string filename = value.substr( value.find( resourcePrefixDelimiter ) + resourcePrefixDelimiter.size() );
			std::string resourcePrefix = value.substr( 0, value.find( ":" ) );

			resourceParams.relativePath = filename;

			resourceParams.prefix = resourcePrefix;

			if( !std::getline( ss, value, delimiter ) )
			{
				return Result{ ResultType::MALFORMED_RESOURCE_INPUT };
			}

			resourceParams.location = value;

			if( !std::getline( ss, value, delimiter ) )
			{
				return Result{ ResultType::MALFORMED_RESOURCE_INPUT };
			}

			resourceParams.checksum = value;

			if( !std::getline( ss, value, delimiter ) )
			{
				return Result{ ResultType::MALFORMED_RESOURCE_INPUT };
			}

			try
			{
				resourceParams.uncompressedSize = std::stoull( value.c_str() );
			}
			catch( std::invalid_argument& )
			{
				return Result{ ResultType::MALFORMED_RESOURCE_INPUT };
			}
			catch( std::out_of_range& )
			{
				return Result{ ResultType::MALFORMED_RESOURCE_INPUT };
			}

			if( !std::getline( ss, value, delimiter ) )
			{
				return Result{ ResultType::MALFORMED_RESOURCE_INPUT };
			}

			try
			{
				resourceParams.compressedSize = std::stoull( value.c_str() );
			}
			catch( std::invalid_argument& )
			{
				return Result{ ResultType::MALFORMED_RESOURCE_INPUT };
			}
			catch( std::out_of_range& )
			{
				return Result{ ResultType::MALFORMED_RESOURCE_INPUT };
			}

			if( !std::getline( ss, value, delimiter ) )
			{
				resourceParams.binaryOperation = 0;
			}
			else
			{
				try
				{
					unsigned long long valueToULongLong = std::stoull( value.c_str() );

					if( valueToULongLong > std::numeric_limits<uint32_t>::max() )
					{
						return Result{ ResultType::MALFORMED_RESOURCE_INPUT };
					}

					resourceParams.binaryOperation = static_cast<uint32_t>( valueToULongLong );
				}
				catch( std::invalid_argument& )
				{
					return Result{ ResultType::MALFORMED_RESOURCE_INPUT };
				}
				catch( std::out_of_range& )
				{
					return Result{ ResultType::MALFORMED_RESOURCE_INPUT };
				}
			}

			// ResourceGroup gets upgraded to 0.1.0
			m_versionParameter = VersionInternal{ 0, 1, 0 };

			// Create a Resource
			ResourceInfo* resource = new ResourceInfo( resourceParams );

			Result addResourceResult = AddResource( resource );

			if( addResourceResult.type != ResultType::SUCCESS )
			{
				return addResourceResult;
			}

			detailStatusSettings.Update( CarbonResources::StatusProgressType::UNBOUNDED, 0, 0, "Imported resource: " + resourceParams.relativePath.string() );
		}
	}

	return Result{ ResultType::SUCCESS };
}

Result ResourceGroup::ResourceGroupImpl::CreateResourceFromResource( const ResourceInfo& resourceIn, ResourceInfo*& resourceOut ) const
{
	resourceOut = nullptr;

	std::string resourceType = "";

	Result getResourceTypeResult = resourceIn.GetType( resourceType );

	if( getResourceTypeResult.type != ResultType::SUCCESS )
	{
		return getResourceTypeResult;
	}

	if( resourceType == ResourceInfo::TypeId() )
	{
		resourceOut = new ResourceInfo( {} );

		Result setParametersFromResourceResult = resourceOut->SetParametersFromResource( &resourceIn, m_versionParameter.GetValue() );

		if( setParametersFromResourceResult.type != ResultType::SUCCESS )
		{
			return setParametersFromResourceResult;
		}
	}
	else if( resourceType == PatchResourceInfo::TypeId() )
	{
		PatchResourceInfo* patchResourceInfo = new PatchResourceInfo( {} );

		Result setParametersFromResourceResult = patchResourceInfo->SetParametersFromResource( &resourceIn, m_versionParameter.GetValue() );

		if( setParametersFromResourceResult.type != ResultType::SUCCESS )
		{
			return setParametersFromResourceResult;
		}

		resourceOut = patchResourceInfo;
	}
	else if( resourceType == BundleResourceInfo::TypeId() )
	{
		BundleResourceInfo* bundleResourceInfo = new BundleResourceInfo( {} );

		Result setParametersFromResourceResult = bundleResourceInfo->SetParametersFromResource( &resourceIn, m_versionParameter.GetValue() );

		if( setParametersFromResourceResult.type != ResultType::SUCCESS )
		{
			return setParametersFromResourceResult;
		}

		resourceOut = bundleResourceInfo;
	}
	else if( resourceType == BundleResourceInfo::TypeId() )
	{
		ResourceInfo* binaryResourceInfo = new ResourceInfo( {} );

		Result setParametersFromResourceResult = binaryResourceInfo->SetParametersFromResource( &resourceIn, m_versionParameter.GetValue() );

		if( setParametersFromResourceResult.type != ResultType::SUCCESS )
		{
			return setParametersFromResourceResult;
		}

		resourceOut = binaryResourceInfo;
	}

	return Result{ ResultType::SUCCESS };
}

Result ResourceGroup::ResourceGroupImpl::CreateResourceFromYaml( YAML::Node& resource, ResourceInfo*& resourceOut )
{
	std::unique_ptr<ResourceInfo> resourceInfo;

	Result createResourceInfoResult = CreateResourceInfoFromYamlNode( resource, resourceInfo, m_versionParameter.GetValue() );

	if( createResourceInfoResult.type != ResultType::SUCCESS )
	{

		return createResourceInfoResult;
	}
	else
	{
		resourceOut = resourceInfo.release();

		return Result{ ResultType::SUCCESS };
	}
}

Result ResourceGroup::ResourceGroupImpl::ImportFromYamlString( const std::string& data, StatusSettings& statusSettings )
{
	YAML::Node resourceGroupFile;
	try
	{
		resourceGroupFile = YAML::Load( data );
	}
	catch( YAML::ParserException& )
	{
		return Result{ ResultType::FAILED_TO_PARSE_YAML };
	}
	return ImportFromYaml( resourceGroupFile, statusSettings );
}

Result ResourceGroup::ResourceGroupImpl::ImportFromYaml( YAML::Node& resourceGroupFile, StatusSettings& statusSettings )
{
	statusSettings.Update( StatusProgressType::PERCENTAGE, 0, 30, "Importing from Yaml file." );

	YAML::Node typeNode = resourceGroupFile[m_type.GetTag()];
	if( !typeNode.IsDefined() )
	{
		return Result{ ResultType::MALFORMED_RESOURCE_GROUP };
	}
	m_type = typeNode.as<std::string>();
	if( m_type.GetValue() != GetType() )
	{
		return Result{ ResultType::FILE_TYPE_MISMATCH };
	}

	YAML::Node resourceGroupVersionNode = resourceGroupFile[m_versionParameter.GetTag()];
	if( !resourceGroupVersionNode.IsDefined() )
	{
		return Result{ ResultType::MALFORMED_RESOURCE_GROUP };
	}
	std::string versionStr = resourceGroupVersionNode.as<std::string>(); //version stringID needs to be in one place

	VersionInternal version;
	version.FromString( versionStr );
	m_versionParameter = version;

	if( m_versionParameter.GetValue().getMajor() > S_DOCUMENT_VERSION.major )
	{
		return Result{ ResultType::DOCUMENT_VERSION_UNSUPPORTED };
	}

	// If version is greater than the max version supported at compile then ceil to that
	if( version > S_DOCUMENT_VERSION )
	{
		statusSettings.Update( StatusProgressType::WARNING, 0, 0, "Supplied resource group version greater than resources build max version. Some data may be lost during import." );

		version = S_DOCUMENT_VERSION;
	}

	YAML::Node numberOfResourcesNode = resourceGroupFile[m_numberOfResources.GetTag()];
	if( !numberOfResourcesNode.IsDefined() )
	{
		return Result{ ResultType::MALFORMED_RESOURCE_GROUP };
	}

	YAML::Node totalResourceSizeCompressedNode = resourceGroupFile[m_totalResourcesSizeCompressed.GetTag()];
	if( !totalResourceSizeCompressedNode.IsDefined() )
	{
		m_totalResourcesSizeCompressed.Reset();
	}

	YAML::Node totalResourceSizeUncompressedNode = resourceGroupFile[m_totalResourcesSizeUncompressed.GetTag()];
	if( !totalResourceSizeUncompressedNode.IsDefined() )
	{
		return Result{ ResultType::MALFORMED_RESOURCE_GROUP };
	}

	Result res = ImportGroupSpecialisedYaml( resourceGroupFile );

	if( res.type != ResultType::SUCCESS )
	{
		return res;
	}

	YAML::Node resources = resourceGroupFile[m_resourcesParameter.GetTag()];
	if( !resources.IsDefined() )
	{
		return Result{ ResultType::MALFORMED_RESOURCE_GROUP };
	}

    {
		StatusSettings resourcesStatusSettings;
		statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 30, 70, "Processing Resources.", &resourcesStatusSettings );

		int i = 0;

		for( auto iter = resources.begin(); iter != resources.end(); iter++ )
		{
			// This bit is a sequence
			YAML::Node resourceNode = ( *iter );

			ResourceInfo* resource = nullptr;

			Result createResourceFromYamlResult = CreateResourceFromYaml( resourceNode, resource );

			if( createResourceFromYamlResult.type != ResultType::SUCCESS )
			{
				return createResourceFromYamlResult;
			}

            if (resourcesStatusSettings.RequiresStatusUpdates())
            {
				float step = static_cast<float>( 100.0 / resources.size() );
				float progress = static_cast<float>( i * step );

                std::filesystem::path resourcePath;
				Result getResourcePathResult = resource->GetRelativePath( resourcePath );

                if( getResourcePathResult.type != ResultType::SUCCESS )
                {
					return getResourcePathResult;
                }

				resourcesStatusSettings.Update( StatusProgressType::PERCENTAGE, progress, step, "Adding resource: " + resourcePath.string() );
            }

			Result addResourceResult = AddResource( resource );

			if( addResourceResult.type != ResultType::SUCCESS )
			{
				return addResourceResult;
			}

			i++;
		}
    }

	return Result{ ResultType::SUCCESS };
}

std::string ResourceGroup::ResourceGroupImpl::GetType() const
{
	return TypeId();
}

std::string ResourceGroup::ResourceGroupImpl::TypeId()
{
	return "ResourceGroup";
}

Result ResourceGroup::ResourceGroupImpl::ImportGroupSpecialisedYaml( YAML::Node& resourceGroupFile )
{
	return Result{ ResultType::SUCCESS };
}

Result ResourceGroup::ResourceGroupImpl::ExportGroupSpecialisedYaml( YAML::Emitter& out, VersionInternal outputDocumentVersion ) const
{
	return Result{ ResultType::SUCCESS };
}

Result ResourceGroup::ResourceGroupImpl::ExportYaml( const VersionInternal& outputDocumentVersion, std::string& data, StatusSettings& statusSettings ) const
{
	statusSettings.Update( StatusProgressType::PERCENTAGE, 0, 20, "Exporting Yaml" );

	YAML::Emitter out;

	// Output header information
	out << YAML::BeginMap;

	// It is possible to export a different version that the imported version
	// The version must be less than the version of the document and also no higher than supported by the binary at compile
	// Ensure document version is valid
	if( !outputDocumentVersion.isVersionValid() )
	{
		return Result{ ResultType::DOCUMENT_VERSION_UNSUPPORTED };
	}

	VersionInternal sanitisedOutputDocumentVersion = outputDocumentVersion;
	const VersionInternal documentCurrentVersion = m_versionParameter.GetValue();

	if( sanitisedOutputDocumentVersion > documentCurrentVersion )
	{
		sanitisedOutputDocumentVersion = documentCurrentVersion;
	}

	if( sanitisedOutputDocumentVersion > S_DOCUMENT_VERSION )
	{
		sanitisedOutputDocumentVersion = S_DOCUMENT_VERSION;
	}

	//Export document parameters
	out << YAML::Key << m_versionParameter.GetTag();
	out << YAML::Value << sanitisedOutputDocumentVersion.ToString();

	out << YAML::Key << m_type.GetTag();
	out << YAML::Value << m_type.GetValue();

	out << YAML::Key << m_numberOfResources.GetTag();
	out << YAML::Value << m_numberOfResources.GetValue();

    if (m_totalResourcesSizeCompressed.HasValue())
    {
		uintmax_t compressedSize = m_totalResourcesSizeCompressed.GetValue();

		out << YAML::Key << m_totalResourcesSizeCompressed.GetTag();
		out << YAML::Value << compressedSize;
		
    }

	out << YAML::Key << m_totalResourcesSizeUncompressed.GetTag();
	out << YAML::Value << m_totalResourcesSizeUncompressed.GetValue();

	Result res = ExportGroupSpecialisedYaml( out, sanitisedOutputDocumentVersion );

	if( res.type != ResultType::SUCCESS )
	{
		return res;
	}

	out << YAML::Key << m_resourcesParameter.GetTag();

	out << YAML::Value << YAML::BeginSeq;

	int i = 0;

    {
		StatusSettings detailExportingStatusSettings;
		statusSettings.Update( StatusProgressType::PERCENTAGE, 20, 80, "Exporting Yaml", &detailExportingStatusSettings );

		for( ResourceInfo* r : m_resourcesParameter )
		{
			// Update status
			if( detailExportingStatusSettings.RequiresStatusUpdates() )
			{
				std::filesystem::path relativePath;

				Result getRelativePathResult = r->GetRelativePath( relativePath );

				if( getRelativePathResult.type != ResultType::SUCCESS )
				{
					return Result{ ResultType::FAIL };
				}

				float step = static_cast<float>( 100.0 / m_resourcesParameter.GetSize() );
				float percentage = static_cast<float>( step * i );

				std::string message = "Exporting: " + relativePath.string();

				detailExportingStatusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, percentage, step, message );

				i++;
			}

			out << YAML::BeginMap;

			Result resourceExportResult = r->ExportToYaml( out, sanitisedOutputDocumentVersion );

			if( resourceExportResult.type != ResultType::SUCCESS )
			{
				return resourceExportResult;
			}

			out << YAML::EndMap;
		}

		out << YAML::EndSeq;

		out << YAML::EndMap;

		data = out.c_str();
    }
    

	return Result{ ResultType::SUCCESS };
}

Result ResourceGroup::ResourceGroupImpl::ExportCsv( const VersionInternal& outputDocumentVersion, std::string& data, StatusSettings& statusSettings ) const
{
	statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 0, 10, "Exporting  to CSV" );

	if( outputDocumentVersion.getMajor() > 0 || outputDocumentVersion.getMinor() > 0 || outputDocumentVersion.getPatch() > 0 )
	{
		return Result{ ResultType::UNSUPPORTED_FILE_FORMAT };
	}

	std::string out;


	auto resourceInfos = m_resourcesParameter.GetValue();
	std::sort(
		resourceInfos->begin(),
		resourceInfos->end(),
		[]( ResourceInfo* a, ResourceInfo* b ) {
			std::filesystem::path ap;
			std::filesystem::path bp;
			a->GetRelativePath( ap );
			b->GetRelativePath( bp );
			return ap < bp;
		} );

	int i{ 0 };

    {
		StatusSettings detailExportStatusSettings;
		statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 10, 90, "Exporting  to CSV", &detailExportStatusSettings );

		for( ResourceInfo* r : m_resourcesParameter )
		{
			// Update status
			if( detailExportStatusSettings.RequiresStatusUpdates() )
			{
				float step = static_cast<float>( 100.0 / m_resourcesParameter.GetValue()->size() );
				float percentage = static_cast<float>( step * i );
				detailExportStatusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, percentage, step, "Percentage Update" );
				i++;
			}

			Result resourceExportResult = r->ExportToCsv( out, m_versionParameter.GetValue() );
			if( resourceExportResult.type != ResultType::SUCCESS )
			{
				return resourceExportResult;
			}
			data += out + "\n";
		}
    }

	return Result{ ResultType::SUCCESS };
}

Result ResourceGroup::ResourceGroupImpl::ProcessChunk( ResourceTools::GetChunk& chunkFile, const std::filesystem::path& chunkRelativePath, BundleResourceGroup::BundleResourceGroupImpl& bundleResourceGroup, const ResourceDestinationSettings& chunkDestinationSettings ) const
{
	// Create resource from Patch Data
	BundleResourceInfo* chunkResource = new BundleResourceInfo( { chunkRelativePath } );

	// md5 Checksum
	ResourceTools::Md5ChecksumStream checksumStream;

	std::string checksum;
	{
		std::string chunk;

		while( *chunkFile.uncompressedChunkIn >> chunk )
		{
			checksumStream << chunk;
		}
	}

	if( !checksumStream.Retrieve( checksum ) )
	{
		return Result( { ResultType::FAILED_TO_GENERATE_CHECKSUM } );
	}

	chunkResource->SetDataChecksum( checksum );

	// Compressed Size
	chunkResource->SetCompressedSize( chunkFile.compressedChunkIn->Size() );

	// Uncompressed Size
	chunkResource->SetUncompressedSize( chunkFile.uncompressedChunkIn->Size() );

	// Export chunk file
	std::filesystem::path sourceFile;

	if( chunkDestinationSettings.destinationType == ResourceDestinationType::REMOTE_CDN )
	{
		sourceFile = chunkFile.compressedChunkIn->GetPath();
	}
	else
	{
		sourceFile = chunkFile.uncompressedChunkIn->GetPath();
	}

	std::filesystem::path targetFile;

	if( chunkDestinationSettings.destinationType == ResourceDestinationType::LOCAL_RELATIVE )
	{
		std::filesystem::path relativePath;

		chunkResource->GetRelativePath( relativePath );

		targetFile = chunkDestinationSettings.basePath / relativePath;
	}
	else
	{
		std::string location;

		chunkResource->GetLocation( location );

		targetFile = chunkDestinationSettings.basePath / location;
	}

	if( std::filesystem::exists( targetFile ) )
	{
		std::filesystem::remove( targetFile );
	}

	if( !std::filesystem::exists( targetFile.parent_path() ) )
	{
		std::filesystem::create_directories( targetFile.parent_path() );
	}

	try
	{
		std::filesystem::copy_file( sourceFile, targetFile );
	}
	catch( std::filesystem::filesystem_error& e )
	{
		return Result( { ResultType::FAILED_TO_SAVE_FILE, e.what() } );
	}

	// Add the chunk resource to the bundleResourceGroup
	Result addResourceResult = bundleResourceGroup.AddResource( chunkResource );

	if( addResourceResult.type != ResultType::SUCCESS )
	{
		delete chunkResource;

		return addResourceResult;
	}

	return Result{ ResultType::SUCCESS };
}

Result ResourceGroup::ResourceGroupImpl::CreateBundle( const BundleCreateParams& params, StatusSettings& statusSettings ) const
{
	// Update status
	statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 0, 5, "Creating Bundle" );

	uintmax_t numberOfChunks = 0;

	std::string chunkBaseName = params.resourceGroupRelativePath.filename().replace_extension().string();

	BundleResourceGroup::BundleResourceGroupImpl bundleResourceGroup;

	Result setChunkSizeResult = bundleResourceGroup.SetChunkSize( params.chunkSize );

	if( setChunkSizeResult.type != ResultType::SUCCESS )
	{
		return setChunkSizeResult;
	}

	ResourceTools::BundleStreamOut bundleStream( params.chunkSize, params.chunkDestinationSettings.basePath, params.chunkDestinationSettings.destinationType == ResourceDestinationType::REMOTE_CDN );


	// Update status
	statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 5, 40, "Generating Chunks" );

	int i = 0;

	std::vector<ResourceInfo*> toBundle;

	std::copy( m_resourcesParameter.begin(), m_resourcesParameter.end(), std::back_inserter( toBundle ) );

	Result getGroupSpecificResourcesToBundleResult = GetGroupSpecificResourcesToBundle( toBundle );

	if( getGroupSpecificResourcesToBundleResult.type != ResultType::SUCCESS )
	{
		return getGroupSpecificResourcesToBundleResult;
	}

	// Loop through all resources and send data for chunking
	{
		StatusSettings fileProcessingDetailStatusSettings;
		statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 45, 35, "Generating Chunks", &fileProcessingDetailStatusSettings );


		for( ResourceInfo* resource : toBundle )
		{
			std::string location;

			Result getLocationResult = resource->GetLocation( location );

			if( getLocationResult.type != ResultType::SUCCESS )
			{
				return getLocationResult;
			}

			if( fileProcessingDetailStatusSettings.RequiresStatusUpdates() )
			{
				std::filesystem::path relativePath;

				Result getRelativePathResult = resource->GetRelativePath( relativePath );

				if( getRelativePathResult.type != ResultType::SUCCESS )
				{
					return getRelativePathResult;
				}

				std::string message;

				if( location.empty() )
				{
					message = "No file to process: " + relativePath.string();
				}
				else
				{
					message = "Processing: " + relativePath.string();
				}

				float step = static_cast<float>( 100.0 / toBundle.size() );
				float percentComplete = static_cast<float>( step * i );

				i++;

				fileProcessingDetailStatusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, percentComplete, step, message );
			}
			if( location.empty() )
			{
				continue;
			}

			auto resourceDataStream = std::make_shared<ResourceTools::FileDataStreamIn>( params.fileReadChunkSize );

			ResourceGetDataStreamParams resourceGetDataParams;

			resourceGetDataParams.resourceSourceSettings = params.resourceSourceSettings;

			resourceGetDataParams.dataStream = resourceDataStream;

			resourceGetDataParams.downloadSettings = params.downloadSettings;

			Result resourceGetDataResult = resource->GetDataStream( resourceGetDataParams );

            while( !resourceDataStream->IsFinished() )
			{
                if (!bundleStream.operator<<(resourceDataStream))
                {
					return Result( { ResultType::FAILED_TO_READ_FROM_STREAM } );
                }
			}

		}

        // Finish last chunk
		if( !bundleStream.Finish() )
		{
			return Result( { ResultType::FAILED_TO_READ_FROM_STREAM } );
		}

		// Add to BundleResourceGroup

		// Loop through possible created chunks
		ResourceTools::GetChunk chunkFile;

		chunkFile.clearCache = false;

		bool bundleReadOk{ true };

		while( ( bundleReadOk = bundleStream >> chunkFile ) && !chunkFile.outOfChunks )
		{
			std::stringstream ss;
			ss << chunkBaseName << numberOfChunks << ".chunk";
			std::string chunkName = ss.str();

			std::filesystem::path chunkPath = params.chunkDestinationSettings.basePath / ss.str();

			Result processChunkResult = ProcessChunk( chunkFile, chunkPath, bundleResourceGroup, params.chunkDestinationSettings );

			if( processChunkResult.type != ResultType::SUCCESS )
			{
				return processChunkResult;
			}

			numberOfChunks++;
		}
		if( !bundleReadOk )
		{
			return Result( { ResultType::FAILED_TO_READ_FROM_STREAM } );
		}
	}

	// Export this resource list
	{
		StatusSettings exportStatusSettings;
		statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 80, 10, "Exporting ResourceGroups", &exportStatusSettings );

		std::string resourceGroupData;

		Result exportToDataResult = ExportToData( resourceGroupData, exportStatusSettings );

		if( exportToDataResult.type != ResultType::SUCCESS )
		{
			return exportToDataResult;
		}


		ResourceGroupInfo resourceGroupInfo( { params.resourceGroupRelativePath } );

		Result setParametersFromDataResult = resourceGroupInfo.SetParametersFromData( resourceGroupData );

		if( setParametersFromDataResult.type != ResultType::SUCCESS )
		{
			return setParametersFromDataResult;
		}

		ResourcePutDataParams putDataParams;

		putDataParams.resourceDestinationSettings = params.chunkDestinationSettings;

		putDataParams.data = &resourceGroupData;

		Result subtractionResourcePutResult = resourceGroupInfo.PutData( putDataParams );

		if( subtractionResourcePutResult.type != ResultType::SUCCESS )
		{
			return subtractionResourcePutResult;
		}

		// Export the bundleGroup
		Result setResourceGroupResult = bundleResourceGroup.SetResourceGroup( resourceGroupInfo );

		if( setResourceGroupResult.type != ResultType::SUCCESS )
		{
			return setResourceGroupResult;
		}
	}

    {
		std::string bundleResourceGroupData;

		StatusSettings exportToDataStatusSettings;
		statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 90, 10, "Exporting ResourceGroups", &exportToDataStatusSettings );

		Result exportBundleResourceGroupToDataResult = bundleResourceGroup.ExportToData( bundleResourceGroupData, exportToDataStatusSettings );

		if( exportBundleResourceGroupToDataResult.type != ResultType::SUCCESS )
		{
			return exportBundleResourceGroupToDataResult;
		}

		BundleResourceGroupInfo bundleResourceGroupInfo( { params.resourceGroupBundleRelativePath } );

		Result setBundleParametersFromDataResult = bundleResourceGroupInfo.SetParametersFromData( bundleResourceGroupData );

		if( setBundleParametersFromDataResult.type != ResultType::SUCCESS )
		{
			return setBundleParametersFromDataResult;
		}

		ResourcePutDataParams bundlePutDataParams;

		bundlePutDataParams.resourceDestinationSettings = params.resourceBundleResourceGroupDestinationSettings;

		bundlePutDataParams.data = &bundleResourceGroupData;

		Result bundleResourceGroupPutResult = bundleResourceGroupInfo.PutData( bundlePutDataParams );

		if( bundleResourceGroupPutResult.type != ResultType::SUCCESS )
		{
			return bundleResourceGroupPutResult;
		}
    }

	return Result{ ResultType::SUCCESS };
}

Result ResourceGroup::ResourceGroupImpl::ConstructPatchResourceInfo( const PatchCreateParams& params, int patchId, uintmax_t dataOffset, uint64_t patchSourceOffset, ResourceInfo* resourceNext, PatchResourceInfo*& patchResource ) const
{
	// Create a resource from patch data
	std::filesystem::path resourceLatestRelativePath;
	Result getResourceLatestRelativePathResult = resourceNext->GetRelativePath( resourceLatestRelativePath );
	if( getResourceLatestRelativePathResult.type != ResultType::SUCCESS )
	{
		return getResourceLatestRelativePathResult;
	}

	PatchResourceInfoParams patchResourceInfoParams;
	std::string patchFilename = params.patchFileRelativePathPrefix.string() + "." + std::to_string( patchId );
	patchResourceInfoParams.relativePath = patchFilename;
	patchResourceInfoParams.targetResourceRelativePath = resourceLatestRelativePath;
	patchResourceInfoParams.dataOffset = dataOffset;
	patchResourceInfoParams.sourceOffset = patchSourceOffset;
	patchResource = new PatchResourceInfo( patchResourceInfoParams );

	return Result{ ResultType::SUCCESS };
}

Result ResourceGroup::ResourceGroupImpl::CreatePatch( const PatchCreateParams& params, StatusSettings& statusSettings ) const
{
	// Update status
	statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 0, 20, "Creating Patch" );

	std::string previousGroupType = params.previousResourceGroup->m_impl->GetType();

	std::string nextGroupType = GetType();

	if( params.previousResourceGroup->m_impl->GetType() != GetType() )
	{
		return Result{ ResultType::PATCH_RESOURCE_LIST_MISSMATCH };
	}

	PatchResourceGroup::PatchResourceGroupImpl patchResourceGroup;

	Result setMaxInputChunkSizeResult = patchResourceGroup.SetMaxInputChunkSize( params.maxInputFileChunkSize );

	if( setMaxInputChunkSizeResult.type != ResultType::SUCCESS )
	{
		return setMaxInputChunkSizeResult;
	}

	// Created resource groups

	std::shared_ptr<ResourceGroupImpl> resourceGroupSubtractionPrevious;

	Result createPreviousResourceGroupResult = CreateResourceGroupFromString( previousGroupType, resourceGroupSubtractionPrevious );

	if( createPreviousResourceGroupResult.type != ResultType::SUCCESS )
	{
		return createPreviousResourceGroupResult;
	}

	std::shared_ptr<ResourceGroupImpl> resourceGroupSubtractionNext;

	Result createNextResourceGroupResult = CreateResourceGroupFromString( nextGroupType, resourceGroupSubtractionNext );

	if( createNextResourceGroupResult.type != ResultType::SUCCESS )
	{
		return createNextResourceGroupResult;
	}


	ResourceGroupSubtractionParams resourceGroupSubtractionParams;

	resourceGroupSubtractionParams.subtractResourceGroup = params.previousResourceGroup->m_impl;

	resourceGroupSubtractionParams.result1 = resourceGroupSubtractionPrevious.get();

	resourceGroupSubtractionParams.result2 = resourceGroupSubtractionNext.get();

    {
		StatusSettings diffStatusSettings;
		statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 20, 20, "Creating Patch", &diffStatusSettings );

		Result subtractionResult = Diff( resourceGroupSubtractionParams, diffStatusSettings );

		if( subtractionResult.type != ResultType::SUCCESS )
		{
			return subtractionResult;
		}
	}
    

	// Ensure that the diff results have the same number of members
	if( resourceGroupSubtractionPrevious->m_resourcesParameter.GetSize() != resourceGroupSubtractionNext->m_resourcesParameter.GetSize() )
	{
		return Result{ ResultType::UNEXPECTED_PATCH_DIFF_ENCOUNTERED };
	}

	int patchId = 0;

	// Update status
    {
		StatusSettings resourceStatusSettings;
		statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 40, 20, "Generating Patches", &resourceStatusSettings );

		for( int i = 0; i < resourceGroupSubtractionNext->m_resourcesParameter.GetSize(); i++ )
		{

			ResourceInfo* resourcePrevious = resourceGroupSubtractionPrevious->m_resourcesParameter.At( i );

			ResourceInfo* resourceNext = resourceGroupSubtractionNext->m_resourcesParameter.At( i );

			if( resourceStatusSettings.RequiresStatusUpdates() )
			{
				float step = static_cast<float>( 100.0 / resourceGroupSubtractionNext->m_resourcesParameter.GetSize() );
				float percentageComplete = static_cast<float>( step * i );

				std::filesystem::path relativePath;

				Result getRelativePathResult = resourcePrevious->GetRelativePath( relativePath );

				if( getRelativePathResult.type != ResultType::SUCCESS )
				{
					return getRelativePathResult;
				}

				std::string message = "Creating patch for: " + relativePath.string();

				resourceStatusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, percentageComplete, step, message );
			}

			size_t patchSourceOffset{ 0 };
			uint64_t patchSourceOffsetDelta{ 0 };

			// Check to see if previous entry contains dummy information
			// Suggesting that this is a new entry in latest
			// In which case there is no reason to create a patch
			// The new entry will be stored with the ResourceGroup related to the PatchResourceGroup
			uintmax_t previousUncompressedSize;

			Result getResourcePreviousCompressedSizeResult = resourcePrevious->GetUncompressedSize( previousUncompressedSize );

			if( getResourcePreviousCompressedSizeResult.type != ResultType::SUCCESS )
			{
				return getResourcePreviousCompressedSizeResult;
			}



			uintmax_t nextUncompressedSize;

			Result getResourceNextCompressedSizeResult = resourceNext->GetUncompressedSize( nextUncompressedSize );

			if( getResourceNextCompressedSizeResult.type != ResultType::SUCCESS )
			{
				return getResourceNextCompressedSizeResult;
			}


			// If previous size is 0 this suggests that this is a new entry in latest
			// In which case there is no reason to create a patch
			if( previousUncompressedSize != 0 )
			{
				// Get resource data previous
				auto previousFileDataStream = std::make_shared<ResourceTools::FileDataStreamIn>( params.maxInputFileChunkSize );

				ResourceGetDataStreamParams previousResourceGetDataStreamParams;

				previousResourceGetDataStreamParams.resourceSourceSettings = params.resourceSourceSettingsPrevious;

				previousResourceGetDataStreamParams.downloadSettings = params.downloadSettings;

				previousResourceGetDataStreamParams.dataStream = previousFileDataStream;

				Result getPreviousDataStreamResult = resourcePrevious->GetDataStream( previousResourceGetDataStreamParams );

				if( getPreviousDataStreamResult.type != ResultType::SUCCESS )
				{
					return getPreviousDataStreamResult;
				}

				// Get resource data next
				auto nextFileDataStream = std::make_shared<ResourceTools::FileDataStreamIn>( params.maxInputFileChunkSize );

				ResourceGetDataStreamParams nextResourceGetDataStreamParams;

				nextResourceGetDataStreamParams.resourceSourceSettings = params.resourceSourceSettingsNext;

				nextResourceGetDataStreamParams.dataStream = nextFileDataStream;

                nextResourceGetDataStreamParams.downloadSettings = params.downloadSettings;

				Result getNextDataStreamResult = resourceNext->GetDataStream( nextResourceGetDataStreamParams );

				if( getNextDataStreamResult.type != ResultType::SUCCESS )
				{
					return getNextDataStreamResult;
				}

				std::filesystem::path relativePath;
				Result getRelativePathResult = resourcePrevious->GetRelativePath( relativePath );
				if( getRelativePathResult.type != ResultType::SUCCESS )
				{
					return getRelativePathResult;
				}

				ResourceTools::ChunkIndex index( previousFileDataStream->GetPath(), params.maxInputFileChunkSize, params.indexFolder );

				index.GenerateChecksumFilter( nextFileDataStream->GetPath() );

				if( !index.Generate() )
				{
					std::string message = "Index generation failed for " + relativePath.string();
					resourceStatusSettings.Update( StatusProgressType::WARNING, 0, 0, message );
				}

				// Process one chunk at a time
				for( uintmax_t dataOffset = 0; dataOffset < nextUncompressedSize; dataOffset += params.maxInputFileChunkSize )
				{
					std::string previousFileData = "";

					if( previousFileDataStream->IsFinished() )
					{
						if( previousFileDataStream->Size() > nextFileDataStream->GetCurrentPosition() )
						{
							// We ran out of data because we found a chunk match later in the file,
							// but we can rewind back to where the read stream is in hopes
							// of getting a good diff, rather than just treating it as new data.
							previousFileDataStream->StartRead( previousFileDataStream->GetPath() );
						}
					}

					// Handling if previous file is smaller than next file
					// If so then previousFileData will be nothing and
					// All next data will be used for the patch
					if( !previousFileDataStream->IsFinished() )
					{
						if( !( *previousFileDataStream >> previousFileData ) )
						{
							return Result{ ResultType::FAILED_TO_RETRIEVE_CHUNK_DATA };
						}
					}

					size_t nextStreamPosition = nextFileDataStream->GetCurrentPosition();
					// Note: in the case that the next file is smaller than previous
					// nothing is stored, application of the patch will chop off the extra file data
					std::string nextFileData;

					if( !nextFileDataStream->IsFinished() )
					{
						if( !( *nextFileDataStream >> nextFileData ) )
						{
							return Result{ ResultType::FAILED_TO_RETRIEVE_CHUNK_DATA };
						}
					}

					// Create a patch
					// Create a patch from the data
					std::string patchData;

					bool chunkMatchFound{ false };
					size_t matchCount{ 0 };


					if( previousFileData != "" )
					{
						// Here's how this should work:
						// We find a matching chunk if it exists. If the chunk exists we make a patch with no data, because we'll get
						// the data from the source file using the patch info. Consecutive patches should be collapsed into one big one.
						// If we can't find a matching chunk, we will base the current diff off the chunk in the source starting after the final byte
						// in the chunk from the source file that we last used.
						// These should keep our patches pretty minimal, even if lots of data gets added early in the file causing offsets.
						// It should also handle small changes in moved parts of the file pretty well.
						chunkMatchFound = index.FindMatchingChunk( nextFileData, patchSourceOffset );

						if( chunkMatchFound )
						{
							matchCount = 1;
							matchCount += ResourceTools::CountMatchingChunks(
								nextFileDataStream->GetPath(),
								nextFileDataStream->GetCurrentPosition(),
								previousFileDataStream->GetPath(),
								patchSourceOffset + params.maxInputFileChunkSize,
								params.maxInputFileChunkSize );

							size_t matchSize = std::min( params.maxInputFileChunkSize * matchCount, previousFileDataStream->Size() - patchSourceOffset );

							PatchResourceInfo* patchResource{ nullptr };

							ConstructPatchResourceInfo( params, patchId, dataOffset, patchSourceOffset, resourceNext, patchResource );

							if( previousFileDataStream->IsFinished() )
							{
								previousFileDataStream->StartRead( previousFileDataStream->GetPath() );
							}

							previousFileDataStream->Seek( patchSourceOffset );

							patchResource->SetParametersFromSourceStream( *previousFileDataStream, matchSize );

							// Advance the first stream by the size of the matching data,
							// but move the point we generate patches from for the previous
							// file data stream to the end of the match.
							// It's hard to tell if it would be smarter to simply advance
							// the destination data by the same amount of the source data,
							// or perhaps even not to move it at all.
							nextFileDataStream->Seek( std::min( nextFileDataStream->Size(), nextStreamPosition + matchSize ) );

							previousFileDataStream->Seek( std::min( previousFileDataStream->Size(), patchSourceOffset + matchSize ) );

							dataOffset += matchSize - params.maxInputFileChunkSize;

							patchSourceOffset += matchSize;

							if( nextStreamPosition == 0 && patchSourceOffset == 0 )
							{
								// This is the beginning of the file and it matches.
								// There is no need to write patch data.
								continue;
							}

							// Add the patch resource to the patchResourceGroup
							Result addResourceResult = patchResourceGroup.AddResource( patchResource );

							if( addResourceResult.type != ResultType::SUCCESS )
							{
								delete patchResource;

								return addResourceResult;
							}

							patchId++;

							continue;
						}
						else
						{
							// Previous and next data chunk are different, create a patch
							if( !ResourceTools::CreatePatch( previousFileData, nextFileData, patchData ) )
							{
								return Result{ ResultType::FAILED_TO_CREATE_PATCH };
							}
							patchSourceOffsetDelta = previousFileData.size();
						}
					}
					else
					{
						// If there is no previous data then just store the data straight from the file
						// All this data is new
						if( !ResourceTools::CreatePatch( "", nextFileData, patchData ) )
						{
							return Result{ ResultType::FAILED_TO_CREATE_PATCH };
						}
						patchSourceOffsetDelta = nextFileData.size();
					}

					PatchResourceInfo* patchResource{ nullptr };
					ConstructPatchResourceInfo( params, patchId, dataOffset, patchSourceOffset, resourceNext, patchResource );
					patchSourceOffset += patchSourceOffsetDelta;
					if( !patchData.empty() )
					{
						Result setParametersFromDataResult = patchResource->SetParametersFromData( patchData, params.calculateCompressions );

						if( setParametersFromDataResult.type != ResultType::SUCCESS )
						{
							return setParametersFromDataResult;
						}

						// Export patch file
						ResourcePutDataParams resourcePutDataParams;

						resourcePutDataParams.resourceDestinationSettings = params.resourcePatchBinaryDestinationSettings;

						resourcePutDataParams.data = &patchData;

						Result putPatchDataResult = patchResource->PutData( resourcePutDataParams );

						if( putPatchDataResult.type != ResultType::SUCCESS )
						{
							delete patchResource;

							return putPatchDataResult;
						}
					}

					// Add the patch resource to the patchResourceGroup
					Result addResourceResult = patchResourceGroup.AddResource( patchResource );

					if( addResourceResult.type != ResultType::SUCCESS )
					{
						delete patchResource;

						return addResourceResult;
					}

					patchId++;
				}
			}
		}
    }
	

	patchResourceGroup.SetRemovedResourceRelativePaths( resourceGroupSubtractionParams.removedResources );

	// Update status
    {
		StatusSettings exportToDataStatusSettings;
		statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 60, 20, "Export ResourceGroups.", &exportToDataStatusSettings );

		// Export the subtraction ResourceGroup
		std::string resourceGroupData;


		Result exportResourceGroupSubtractionLatestResult = resourceGroupSubtractionNext->ExportToData( resourceGroupData, exportToDataStatusSettings );

		if( exportResourceGroupSubtractionLatestResult.type != ResultType::SUCCESS )
		{
			return exportResourceGroupSubtractionLatestResult;
		}

		ResourceGroupInfo subtractionResourceGroupInfo( { params.resourceGroupRelativePath } );

		Result setParametersFromDataResult = subtractionResourceGroupInfo.SetParametersFromData( resourceGroupData );

		if( setParametersFromDataResult.type != ResultType::SUCCESS )
		{
			return setParametersFromDataResult;
		}

		ResourcePutDataParams putDataParams;

		putDataParams.resourceDestinationSettings = params.resourcePatchBinaryDestinationSettings;

		putDataParams.data = &resourceGroupData;

		Result subtractionResourcePutResult = subtractionResourceGroupInfo.PutData( putDataParams );

		if( subtractionResourcePutResult.type != ResultType::SUCCESS )
		{
			return subtractionResourcePutResult;
		}

		// Export the patchGroup
		Result setResourceGroupResult = patchResourceGroup.SetResourceGroup( subtractionResourceGroupInfo );

		if( setResourceGroupResult.type != ResultType::SUCCESS )
		{
			return setResourceGroupResult;
		}
    }
	
	std::string patchResourceGroupData;
    {
		StatusSettings exportPatchResourceGroupStatusSettings;
		statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 80, 20, "Export ResourceGroups.", &exportPatchResourceGroupStatusSettings );


		Result exportToDataResult = patchResourceGroup.ExportToData( patchResourceGroupData, exportPatchResourceGroupStatusSettings );

		if( exportToDataResult.type != ResultType::SUCCESS )
		{
			return exportToDataResult;
		}

		PatchResourceGroupInfo patchResourceGroupInfo( { params.resourceGroupPatchRelativePath } );

		Result setPatchParametersFromDataResult = patchResourceGroupInfo.SetParametersFromData( patchResourceGroupData );

		if( setPatchParametersFromDataResult.type != ResultType::SUCCESS )
		{
			return setPatchParametersFromDataResult;
		}

		ResourcePutDataParams patchPutDataParams;

		patchPutDataParams.resourceDestinationSettings = params.resourcePatchResourceGroupDestinationSettings;

		patchPutDataParams.data = &patchResourceGroupData;

		Result patchResourceGroupPutResult = patchResourceGroupInfo.PutData( patchPutDataParams );

		if( patchResourceGroupPutResult.type != ResultType::SUCCESS )
		{
			return patchResourceGroupPutResult;
		}
    }

	return Result{ ResultType::SUCCESS };
}


Result ResourceGroup::ResourceGroupImpl::AddResource( ResourceInfo* resource )
{
	m_resourcesParameter.PushBack( resource );

	m_numberOfResources = m_numberOfResources.GetValue() + 1;

	uintmax_t resourceUncompressedSize;

	Result resourceGetUncompressedSizeResult = resource->GetUncompressedSize( resourceUncompressedSize );

	if( resourceGetUncompressedSizeResult.type != ResultType::SUCCESS )
	{
		return resourceGetUncompressedSizeResult;
	}

	m_totalResourcesSizeUncompressed = m_totalResourcesSizeUncompressed.GetValue() + resourceUncompressedSize;

	uintmax_t resourceCompressedSize;

	Result resourceGetCompressedSizeResult = resource->GetCompressedSize( resourceCompressedSize );

	if( resourceGetCompressedSizeResult.type == ResultType::SUCCESS )
	{
		m_totalResourcesSizeCompressed = m_totalResourcesSizeCompressed.GetValue() + resourceCompressedSize;
	}

	return Result{ ResultType::SUCCESS };
}

Result ResourceGroup::ResourceGroupImpl::RemoveResources( const ResourceGroupRemoveResourcesParams& params, StatusSettings& statusSettings )
{
	// Update status
	statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 0, 5, "Removing resources from Resource Group" );

	if( !params.resourcesToRemove )
	{
		return Result{ ResultType::RESOURCE_LIST_NOT_SET };
	}

    {
		StatusSettings nestedStatusSettings;
		statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 5, 95, "Removing resources from Resource Group", &nestedStatusSettings );

        int i = 0;
		for( std::filesystem::path& relativePath : *params.resourcesToRemove )
		{
            if (nestedStatusSettings.RequiresStatusUpdates())
            {
				float step = static_cast<float>( 100.0 / params.resourcesToRemove->size() );
				float percentComplete = static_cast<float>( step * i );
                nestedStatusSettings.Update( StatusProgressType::PERCENTAGE, percentComplete, step, "Removing resource: " + relativePath.string() );
				i++;
            }

			// Construct a ResourceInfo from relativePath
			ResourceInfoParams resourceInfoParams;

			resourceInfoParams.relativePath = relativePath;

			ResourceInfo resource( resourceInfoParams );

			Result removeResourceResult = RemoveResource( resource );

			if( removeResourceResult.type != ResultType::SUCCESS )
			{
				if( removeResourceResult.type != ResultType::RESOURCE_NOT_FOUND )
				{
					return removeResourceResult;
				}
				else
				{
					if( params.errorIfResourceNotFound == true )
					{
						return removeResourceResult;
					}
				}
			}
		}
    }

	return Result{ ResultType::SUCCESS };
}

Result ResourceGroup::ResourceGroupImpl::RemoveResource( ResourceInfo& resource )
{

	auto iter = m_resourcesParameter.Find( &resource );

	if( iter == m_resourcesParameter.end() )
	{
		return Result{ ResultType::RESOURCE_NOT_FOUND };
	}

	ResourceInfo* foundResource = *iter;

	// Update counters
	m_numberOfResources = m_numberOfResources.GetValue() - 1;

	uintmax_t resourceUncompressedSize;

	Result resourceGetUncompressedSizeResult = foundResource->GetUncompressedSize( resourceUncompressedSize );

	if( resourceGetUncompressedSizeResult.type != ResultType::SUCCESS )
	{
		return resourceGetUncompressedSizeResult;
	}

	uintmax_t resourceCompressedSize;

	Result resourceGetCompressedSizeResult = foundResource->GetCompressedSize( resourceCompressedSize );

	if( resourceGetCompressedSizeResult.type != ResultType::SUCCESS )
	{
		return resourceGetCompressedSizeResult;
	}

	// Update sizes
	m_totalResourcesSizeUncompressed = m_totalResourcesSizeUncompressed.GetValue() - resourceUncompressedSize;

	m_totalResourcesSizeCompressed = m_totalResourcesSizeCompressed.GetValue() - resourceCompressedSize;

	// Remove from ResourceGroup
	m_resourcesParameter.Remove( iter );

	return Result{ ResultType::SUCCESS };
}

Result ResourceGroup::ResourceGroupImpl::Merge( const ResourceGroupMergeParams& params, StatusSettings& statusSettings )
{
	statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 0, 20, "Merging resource groups." );

	if( params.mergedResourceGroup == nullptr )
	{
		return Result{ ResultType::RESOURCE_GROUP_NOT_SET };
	}

	if( params.resourceGroupToMerge == nullptr )
	{
		return Result{ ResultType::RESOURCE_GROUP_NOT_SET };
	}

	DocumentParameterCollection<ResourceInfo*> mergeResources = params.resourceGroupToMerge->m_impl->m_resourcesParameter;

	std::vector<ResourceInfo*> sortedResourcesParameter( m_resourcesParameter.begin(), m_resourcesParameter.end() );
	std::vector<ResourceInfo*> sortedMergeResources( mergeResources.begin(), mergeResources.end() );

	std::sort( sortedResourcesParameter.begin(), sortedResourcesParameter.end(), []( const ResourceInfo* a, const ResourceInfo* b ) { return *a < *b; } );
	std::sort( sortedMergeResources.begin(), sortedMergeResources.end(), []( const ResourceInfo* a, const ResourceInfo* b ) { return *a < *b; } );

	std::vector<ResourceInfo*> unionResources;
	std::set_union(
		sortedMergeResources.begin(), sortedMergeResources.end(),
		sortedResourcesParameter.begin(), sortedResourcesParameter.end(),
		std::back_inserter( unionResources ),
		[]( const ResourceInfo* a, const ResourceInfo* b ) { return *a < *b; } );


    {
		StatusSettings nestedStatusSettings;
		statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 20, 80, "Merging resource groups.", &nestedStatusSettings );

		// Add result to merge ResourceGroup output
		int i = 0;
		for( auto resource : unionResources )
		{
            if (nestedStatusSettings.RequiresStatusUpdates())
            {
				float step = static_cast<float>( 100.0 / unionResources.size() );
				float percentComplete = static_cast<float>( step * i );
				nestedStatusSettings.Update( StatusProgressType::PERCENTAGE, percentComplete, step, "Merging Resource" );
				i++;
            }

			ResourceInfo* resourceCopy = nullptr;

			Result createResourceFromResourceResult = CreateResourceFromResource( *resource, resourceCopy );

			if( createResourceFromResourceResult.type != ResultType::SUCCESS )
			{
				return createResourceFromResourceResult;
			}

			Result addResourceResult = params.mergedResourceGroup->m_impl->AddResource( resourceCopy );

			if( addResourceResult.type != ResultType::SUCCESS )
			{
				return addResourceResult;
			}
		}
    }

	return Result{ ResultType::SUCCESS };
}

Result ResourceGroup::ResourceGroupImpl::DiffChangesAsLists( const ResourceGroupDiffAgainstGroupParams& params, StatusSettings& statusSettings ) const
{
	statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 0, 20, "Diffing changes as lists." );

	if( !params.resourceGroupToDiffAgainst )
	{
		return Result{ ResultType::RESOURCE_GROUP_NOT_SET };
	}

	if( ( !params.additions ) || ( !params.subtractions ) )
	{
		return Result{ ResultType::REQUIRED_INPUT_PARAMETER_NOT_SET };
	}

	ResourceGroupSubtractionParams subtractionParams;

	ResourceGroup result1;

	ResourceGroup result2;

	subtractionParams.subtractResourceGroup = params.resourceGroupToDiffAgainst->m_impl;

	subtractionParams.result1 = result1.m_impl;

	subtractionParams.result2 = result2.m_impl;

    {
		StatusSettings diffStatusSettings;
		statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 20, 60, "Diffing changes as lists.", &diffStatusSettings );

		Result diffResult = Diff( subtractionParams, diffStatusSettings );

		if( diffResult.type != ResultType::SUCCESS )
		{
			return diffResult;
		}
	}

    {
		StatusSettings subtractionsStatusSettings;
		statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 80, 10, "Collating subtractions.", &subtractionsStatusSettings );

        int i = 0;
		for( auto removedResource : subtractionParams.removedResources )
		{
			if( subtractionsStatusSettings.RequiresStatusUpdates() )
            {
				float step = static_cast<float>( 100.0 / subtractionParams.removedResources.size() );
				float percentage = static_cast<float>( i * step );
				subtractionsStatusSettings.Update( StatusProgressType::PERCENTAGE, percentage, step, removedResource.string() );
				i++;
            }
			
			params.subtractions->push_back( removedResource );
		}
	}
    
    {
		StatusSettings additionsStatusSettings;
		statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 90, 10, "Collating additions.", &additionsStatusSettings );

        int i = 0;
		for( auto resource : result1.m_impl->m_resourcesParameter )
		{
            
			std::filesystem::path relativePath;

			Result getRelativePathResult = resource->GetRelativePath( relativePath );

            if( additionsStatusSettings.RequiresStatusUpdates() )
			{
				float step = static_cast<float>( 100.0 / result1.m_impl->m_resourcesParameter.GetSize() );
				float percentage = static_cast<float>( i * step );
				additionsStatusSettings.Update( StatusProgressType::PERCENTAGE, percentage, step, relativePath.string() );
				i++;
			}

			if( getRelativePathResult.type != ResultType::SUCCESS )
			{
				return getRelativePathResult;
			}

			params.additions->push_back( relativePath );
		}
    }

	return Result{ ResultType::SUCCESS };
}

Result ResourceGroup::ResourceGroupImpl::Diff( ResourceGroupSubtractionParams& params, StatusSettings& statusSettings ) const
{
	statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 0, 20, "Calculating diff between two resource groups." );

	DocumentParameterCollection<ResourceInfo*> subtractionResources = params.subtractResourceGroup->m_resourcesParameter;
	// Iterate through all resources

	// Value only required for status updates
	int i = 0;
	std::vector<ResourceInfo*> sortedResourcesParameter( m_resourcesParameter.begin(), m_resourcesParameter.end() );
	std::vector<ResourceInfo*> sortedSubtractionResources( subtractionResources.begin(), subtractionResources.end() );
	std::sort( sortedResourcesParameter.begin(), sortedResourcesParameter.end(), []( const ResourceInfo* a, const ResourceInfo* b ) { return *a < *b; } );
	std::sort( sortedSubtractionResources.begin(), sortedSubtractionResources.end(), []( const ResourceInfo* a, const ResourceInfo* b ) { return *a < *b; } );

	std::vector<ResourceInfo*> addedResources;
	std::set_difference(
		sortedResourcesParameter.begin(), sortedResourcesParameter.end(),
		sortedSubtractionResources.begin(), sortedSubtractionResources.end(),
		std::back_inserter( addedResources ),
		[]( const ResourceInfo* a, const ResourceInfo* b ) { return *a < *b; } );

	std::vector<ResourceInfo*> removedResources;
	std::set_difference(
		sortedSubtractionResources.begin(), sortedSubtractionResources.end(),
		sortedResourcesParameter.begin(), sortedResourcesParameter.end(),
		std::back_inserter( removedResources ),
		[]( const ResourceInfo* a, const ResourceInfo* b ) { return *a < *b; } );

	std::vector<ResourceInfo*> potentiallyModifiedResources;
	std::set_intersection(
		sortedResourcesParameter.begin(), sortedResourcesParameter.end(),
		sortedSubtractionResources.begin(), sortedSubtractionResources.end(),
		std::back_inserter( potentiallyModifiedResources ),
		[]( const ResourceInfo* a, const ResourceInfo* b ) { return *a < *b; } );

    {
		StatusSettings processingResourcesStatus;
		statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 20, 20, "Calculating diff between two resource groups.", &processingResourcesStatus );


		for( ResourceInfo* resource : potentiallyModifiedResources )
		{
			if( processingResourcesStatus.RequiresStatusUpdates() )
			{
				std::filesystem::path relativePath;

				Result getRelativePathResult = resource->GetRelativePath( relativePath );

				if( getRelativePathResult.type != ResultType::SUCCESS )
				{
					return getRelativePathResult;
				}

				std::string message = "Processing: " + relativePath.string();

				float step = static_cast<float>( 100.0 / m_resourcesParameter.GetSize() );
				float percentComplete = static_cast<float>( step * i );

				processingResourcesStatus.Update( CarbonResources::StatusProgressType::PERCENTAGE, percentComplete, step, message );

				i++;
			}

			ResourceInfo* resource2 = *std::lower_bound(
				sortedSubtractionResources.begin(), sortedSubtractionResources.end(), resource, []( const ResourceInfo* a, const ResourceInfo* b ) { return *a < *b; } );

			std::string resource1Checksum;

			Result getResource1ChecksumResult = resource->GetChecksum( resource1Checksum );

			if( getResource1ChecksumResult.type != ResultType::SUCCESS )
			{
				return getResource1ChecksumResult;
			}

			std::string resource2Checksum;

			Result getResource2ChecksumResult = resource2->GetChecksum( resource2Checksum );

			if( getResource2ChecksumResult.type != ResultType::SUCCESS )
			{
				return getResource2ChecksumResult;
			}

			// Has this resource changed?
			if( resource1Checksum != resource2Checksum )
			{
				// The binary data has changed between versions, record an entry in both lists

				// Create a copy of the resource to result 2 (Latest)
				ResourceInfo* resourceCopy1 = nullptr;

				Result createResourceFromResource1Result = CreateResourceFromResource( *resource, resourceCopy1 );

				if( createResourceFromResource1Result.type != ResultType::SUCCESS )
				{
					return createResourceFromResource1Result;
				}

				params.result2->AddResource( resourceCopy1 );

				ResourceInfo* resourceCopy2 = nullptr;

				Result createResourceFromResource2Result = CreateResourceFromResource( *resource2, resourceCopy2 );

				if( createResourceFromResource2Result.type != ResultType::SUCCESS )
				{
					return createResourceFromResource2Result;
				}

				params.result1->AddResource( resourceCopy2 );
			}
		}
    }
    
    {
		StatusSettings addResourceStatusSettings;
		statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 40, 20, "Calculating diff between two resource groups.", &addResourceStatusSettings );
		i = 0;
		for( auto resource : addedResources )
		{
			// This is a new resource, add it to target
			// Note: Could be made optional, perhaps it is desirable to only include patch updates
			// Not new files, probably make as optional pass in setting
			if( addResourceStatusSettings.RequiresStatusUpdates() )
			{
				std::filesystem::path relativePath;
				Result getRelativePathResult = resource->GetRelativePath( relativePath );
				if( getRelativePathResult.type != ResultType::SUCCESS )
				{
					return getRelativePathResult;
				}
				std::string message = "Processing new resource: " + relativePath.string();
				float step = static_cast<float>( 100.0 / m_resourcesParameter.GetSize() );
				float percentComplete = static_cast<float>( step * i );
				addResourceStatusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, percentComplete, step, message );
				i++;
			}

			ResourceInfo* resourceCopy1 = nullptr;

			Result createResourceFromResourceResult = CreateResourceFromResource( *resource, resourceCopy1 );

			if( createResourceFromResourceResult.type != ResultType::SUCCESS )
			{
				return createResourceFromResourceResult;
			}

			params.result2->AddResource( resourceCopy1 );

			// Place in a dummy entry into result1 which shows that resource is new
			// This ensures that both lists stay the same size which makes it easier
			// To parse later
			std::filesystem::path resourceRelativePath;

			Result getResourceRelativepathResult = resource->GetRelativePath( resourceRelativePath );

			if( getResourceRelativepathResult.type != ResultType::SUCCESS )
			{
				return getResourceRelativepathResult;
			}

			ResourceInfoParams dummyResourceParams;
			dummyResourceParams.relativePath = resourceRelativePath;

			ResourceInfo* dummyResource = new ResourceInfo( dummyResourceParams );
			params.result1->AddResource( dummyResource );
		}
    }
    
    {
		StatusSettings removeResourceStatusSettings;
		statusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, 60, 40, "Calculating diff between two resource groups.", &removeResourceStatusSettings );

        i = 0;
		for( auto resource : removedResources )
		{
			if( removeResourceStatusSettings.RequiresStatusUpdates() )
			{
				std::filesystem::path relativePath;
				Result getRelativePathResult = resource->GetRelativePath( relativePath );
				if( getRelativePathResult.type != ResultType::SUCCESS )
				{
					return getRelativePathResult;
				}
				std::string message = "Processing removed resource: " + relativePath.string();
				float step = static_cast<float>( 100.0 / m_resourcesParameter.GetSize() );
				float percentComplete = static_cast<float>( step * i );
				removeResourceStatusSettings.Update( CarbonResources::StatusProgressType::PERCENTAGE, percentComplete, step, message );
				i++;
			}
			std::filesystem::path path;
			auto result = resource->GetRelativePath( path );
			if( result.type != ResultType::SUCCESS )
			{
				return result;
			}
			params.removedResources.push_back( path );
		}
    }

	return Result{ ResultType::SUCCESS };
}

std::vector<ResourceInfo*>::iterator ResourceGroup::ResourceGroupImpl::begin()
{
	return m_resourcesParameter.begin();
}

std::vector<ResourceInfo*>::const_iterator ResourceGroup::ResourceGroupImpl::begin() const
{
	return m_resourcesParameter.begin();
}

std::vector<ResourceInfo*>::const_iterator ResourceGroup::ResourceGroupImpl::cbegin()
{
	return m_resourcesParameter.begin();
}

std::vector<ResourceInfo*>::iterator ResourceGroup::ResourceGroupImpl::end()
{
	return m_resourcesParameter.end();
}

std::vector<ResourceInfo*>::const_iterator ResourceGroup::ResourceGroupImpl::end() const
{
	return m_resourcesParameter.end();
}

std::vector<ResourceInfo*>::const_iterator ResourceGroup::ResourceGroupImpl::cend()
{
	return m_resourcesParameter.end();
}

size_t ResourceGroup::ResourceGroupImpl::GetSize() const
{
	return m_resourcesParameter.GetSize();
}

Result ResourceGroup::ResourceGroupImpl::GetGroupSpecificResourcesToBundle( std::vector<ResourceInfo*>& toBundle ) const
{
	return Result{ ResultType::SUCCESS };
}


}