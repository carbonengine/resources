#include "Enums.h"

namespace CarbonResources
{
    bool resultToString( Result result, std::string& output )
    {

	    switch( result.type )
	    {
	    case ResultType::SUCCESS:
		    output = "Operation was successful.";
		    return true;

	    case ResultType::FAIL:
		    output = "Operation failed. This is an internal library error which shouldn't be encountered. If you encounter this error contact API addministrators.";
		    return true;

	    case ResultType::UNSUPPORTED_FILE_FORMAT:
		    output = "Operation encounted an unsupported file format. It is likely that supplied file format is unsupported.";
		    return true;

	    case ResultType::FAILED_TO_OPEN_FILE:
		    output = "A file failed to open during operation. Perhaps the file was not found, or is locked.";
		    return true;

	    case ResultType::MALFORMED_RESOURCE_INPUT:
		    output = "A malformed resource was encountered. This may be related to required missing fields required for document version.";
		    return true;

	    case ResultType::FILE_TYPE_MISMATCH:
		    output = "The document type being imported didn't match expected type for operation.";
		    return true;

	    case ResultType::DOCUMENT_VERSION_UNSUPPORTED:
		    output = "Document version supplied is not supported.";
		    return true;

	    case ResultType::REQUIRED_RESOURCE_PARAMETER_NOT_SET:
		    output = "Resource parameter that is required for the document version has not been set.";
		    return true;

	    case ResultType::FAILED_TO_OPEN_FILE_STREAM:
			output = "Failed to open file stream during operation.Perhaps the file was not found, or is locked.";
		    return true;

	    case ResultType::FAILED_TO_READ_FROM_STREAM:
			output = "An error was encountered while attempting to read from stream.";
		    return true;

	    case ResultType::FAILED_TO_WRITE_TO_STREAM:
			output = "An error was encountered while attempting to write to a stream.";
		    return true;

		case ResultType::FAILED_TO_DOWNLOAD_FILE:
			output = "A file failed to download during operation. Check supplied URLS are correct.";
		    return true;

	    case ResultType::FAILED_TO_CREATE_PATCH:
			output = "An error occurred during generation of a binary patch.";
		    return true;

	    case ResultType::FAILED_TO_SAVE_FILE:
			output = "Operation failed to save a file.";
		    return true;

	    case ResultType::FAILED_TO_GENERATE_CHECKSUM:
			output = "An error occurred during generation of a data checksum.";
		    return true;

	    case ResultType::FAILED_TO_GENERATE_RELATIVE_PATH_CHECKSUM:
			output = "An error occurred during generation of a relative path checksum.";
		    return true;

	    case ResultType::FAILED_TO_COMPRESS_DATA:
			output = "An error occurred during data compression.";
		    return true;

	    case ResultType::PATCH_RESOURCE_LIST_MISSMATCH:
		    output = "ResourceGroups provided to create patch do not match in type. Validate ResourceGroup inputs are correct.";
		    return true;

	    case ResultType::FAILED_TO_APPLY_PATCH:
			output = "An error occurred during application of a binary patch.";
		    return true;

	    case ResultType::UNEXPECTED_PATCH_CHECKSUM_RESULT:
		    output = "Patched resource doesn't match expected checksum upon patch completion.";
		    return true;

	    case ResultType::UNEXPECTED_PATCH_DIFF_ENCOUNTERED:
		    output = "ResourceGroup diff operation produced unexpected result. This is an internal library error which shouldn't be encountered. If you encounter this error contact API addministrators.";
		    return true;

	    case ResultType::FILE_NOT_FOUND:
		    output = "Required file not found. Validate parameter inputs.";
		    return true;

	    case ResultType::FAILED_TO_RETRIEVE_CHUNK_DATA:
		    output = "An error was encountered while attempting to read from a chunk data stream.";
		    return true;

	    case ResultType::RESOURCE_VALUE_NOT_SET:
		    output = "A request was made for a parameter which was not set. This is an internal library error which shouldn't be encountered. If you encounter this error contact API addministrators.";
		    return true;

	    case ResultType::UNEXPECTED_END_OF_CHUNKS:
		    output = "Chunk file finished before all expected chunks had reconstituted.";
		    return true;

	    case ResultType::UNEXPECTED_CHUNK_CHECKSUM_RESULT:
		    output = "Generated checksum didn't match expected result.";
		    return true;

	    case ResultType::FAILED_TO_SAVE_TO_STREAM:
			output = "An error was encountered while attempting to write to a stream.";
		    return true;

	    case ResultType::INPUT_DIRECTORY_DOESNT_EXIST:
		    output = "Supplied input directory doesn't exist. Validate parameter inputs.";
		    return true;

        case ResultType::RESOURCE_TYPE_MISSMATCH:
			output = "Resource parameters were attempted to be set by a resource of a different type. This is an internal library error which shouldn't be encountered. If you encounter this error contact API addministrators.";
			return true;

	    case ResultType::MALFORMED_RESOURCE_GROUP:
	    	output = "The resource group does not seem to contain the required parameters for this version.";
	    	return true;
	    }

	    output = "Error code unrecognised. This is an internal library error which shouldn't be encountered. If you encounter this error contact API addministrators.";

	    return false;
    }
}