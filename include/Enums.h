/* 
	*************************************************************************

	Enums.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/

#pragma once
#ifndef Enums_H
#define Enums_H

#include <memory>
#include <string>
#include <sstream>
#include <functional>

#include "Exports.h"

namespace CarbonResources
{

    /** Status Callback function signature. */
    using StatusCallback = std::function<void( int, int, const std::string& )>;

    /**
    * @enum CarbonResources::ResultType
    * @brief Error return type codes.
    * @var SUCCESS
    * Operation was successful.
    * @var FAIL
    * Operation failed. This is an internal library error which shouldn't be encountered. If you encounter this error contact API addministrators.
	* @var UNSUPPORTED_FILE_FORMAT
    * Operation encounted an unsupported file format. It is likely that supplied file format is unsupported.
	* @var FAILED_TO_OPEN_FILE
    * A file failed to open during operation. Perhaps the file was not found, or is locked.
	* @var MALFORMED_RESOURCE_INPUT
    * A malformed resource was encountered. This may be related to required missing fields required for document version.
	* @var FILE_TYPE_MISMATCH
    * The document type being imported didn't match expected type for operation.
    * @var DOCUMENT_VERSION_UNSUPPORTED
    * Document major version is greater than library max version. Library needs updating.
	* @var REQUIRED_RESOURCE_PARAMETER_NOT_SET
    * Resource parameter that is required for the document version has not been set.
	* @var FAILED_TO_OPEN_FILE_STREAM
    * Failed to open file stream during operation.Perhaps the file was not found, or is locked.
    * @var FAILED_TO_READ_FROM_STREAM
    * An error was encountered while attempting to read from stream.
	* @var FAILED_TO_WRITE_TO_STREAM
    * An error was encountered while attempting to write to a stream.
	* @var FAILED_TO_DOWNLOAD_FILE
    * A file failed to download during operation. Check supplied URLS are correct.
    * @var FAILED_TO_CREATE_PATCH
    * An error occurred during generation of a binary patch.
	* @var FAILED_TO_SAVE_FILE
    * Operation failed to save a file.
	* @var FAILED_TO_GENERATE_CHECKSUM
    * An error occurred during generation of a data checksum.
	* @var FAILED_TO_GENERATE_RELATIVE_PATH_CHECKSUM
    * An error occurred during generation of a relative path checksum.
	* @var FAILED_TO_COMPRESS_DATA
    * An error occurred during data compression.
	* @var PATCH_RESOURCE_LIST_MISSMATCH
    * ResourceGroups provided to create patch do not match in type. Validate ResourceGroup inputs are correct.
	* @var FAILED_TO_APPLY_PATCH
    * An error occurred during application of a binary patch.
	* @var UNEXPECTED_PATCH_CHECKSUM_RESULT
    * Patched resource doesn't match expected checksum upon patch completion.
	* @var UNEXPECTED_PATCH_DIFF_ENCOUNTERED
    * ResourceGroup diff operation produced unexpected result. This is an internal library error.
	* @var FILE_NOT_FOUND
    * Required file not found. Validate parameter inputs.
	* @var FAILED_TO_RETRIEVE_CHUNK_DATA
    * An error was encountered while attempting to read from a chunk data stream.
	* @var RESOURCE_VALUE_NOT_SET
    * A request was made for a parameter which was not set. This is an internal library error.
	* @var UNEXPECTED_END_OF_CHUNKS
    * Chunk file finished before all expected chunks had reconstituted.
	* @var UNEXPECTED_CHUNK_CHECKSUM_RESULT
    * Generated checksum didn't match expected result.
	* @var FAILED_TO_SAVE_TO_STREAM
    * An error was encountered while attempting to write to a stream.
	* @var INPUT_DIRECTORY_DOESNT_EXIST
    * Supplied input directory doesn't exist. Validate parameter inputs.
	* @var RESOURCE_TYPE_MISSMATCH
    * Resource parameters were attempted to be set by a resource of a different type. This is an internal library error which shouldn't be encountered. If you encounter this error contact API addministrators.
    */
    enum class ResultType
    {
	    SUCCESS,
	    FAIL,
	    UNSUPPORTED_FILE_FORMAT,
	    FAILED_TO_OPEN_FILE,
	    MALFORMED_RESOURCE_INPUT,
	    FILE_TYPE_MISMATCH,
		DOCUMENT_VERSION_UNSUPPORTED,
		REQUIRED_RESOURCE_PARAMETER_NOT_SET,
		FAILED_TO_OPEN_FILE_STREAM,
        FAILED_TO_READ_FROM_STREAM,
		FAILED_TO_WRITE_TO_STREAM,
	    FAILED_TO_DOWNLOAD_FILE,
		FAILED_TO_CREATE_PATCH,
		FAILED_TO_SAVE_FILE,
		FAILED_TO_GENERATE_CHECKSUM,
		FAILED_TO_GENERATE_RELATIVE_PATH_CHECKSUM,
		FAILED_TO_COMPRESS_DATA,
		PATCH_RESOURCE_LIST_MISSMATCH,
		FAILED_TO_APPLY_PATCH,
		UNEXPECTED_PATCH_CHECKSUM_RESULT,
		UNEXPECTED_PATCH_DIFF_ENCOUNTERED,
		FILE_NOT_FOUND,
		FAILED_TO_RETRIEVE_CHUNK_DATA,
		RESOURCE_VALUE_NOT_SET,
		UNEXPECTED_END_OF_CHUNKS,
		UNEXPECTED_CHUNK_CHECKSUM_RESULT,
		FAILED_TO_SAVE_TO_STREAM,
		INPUT_DIRECTORY_DOESNT_EXIST,
	    RESOURCE_TYPE_MISSMATCH,
    	MALFORMED_RESOURCE_GROUP,
        //NOTE: if adding to this enum, a complimentary entry must be added to resultToString.
    };

    /** @struct Result
    *  @brief Represents Version information. Version follows semantic versioning paradigm.
    *  @var Result::type
    *  Type of result returned
    *  @var Version::info
    *  Optional further information on the return
    */
    struct API Result
    {
		ResultType type = ResultType::SUCCESS;

		std::string info = "";
    };
	
	bool API resultToString( Result result, std::string& output );
    
    /** @struct Version
    *  @brief Represents Version information. Version follows semantic versioning paradigm.
    *  @var Version::major
    *  Major version.
    *  @var Version::minor
    *  Minor version.
    *  @var Version::patch
    *  Patch version
    */
    struct Version
    {
		unsigned int major;
		unsigned int minor;
		unsigned int patch;
    };

    static const Version S_LIBRARY_VERSION = { 1, 0, 0 }; /*!< Current version of the carbon-resources */

    static const Version S_DOCUMENT_VERSION = { 0, 1, 0 }; /*!< Maximum document version supported by carbon-resources */

}

#endif // Enums_H