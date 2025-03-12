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

namespace CarbonResources
{
    // TODO This is not an enum, move it out of here
    // It also shouldn't be in the API
    struct Version
    {
		Version() 
		{
		}

        Version(unsigned int majorIn, unsigned int minorIn, unsigned int patchIn):
			major(majorIn),
			minor(minorIn),
			patch(patchIn)
        {

        }

		bool operator>( Version value ) const
		{
			if( major > value.major )
			{
				return true;
			}
			else
			{
				if( minor > value.minor )
				{
					return true;
				}
				else
				{
					return patch > value.patch;
				}
			}
		}

		bool operator>( Version& value ) const
		{
			if( major > value.major )
			{
				return true;
			}
			else
			{
				if( minor > value.minor )
				{
					return true;
				}
				else
				{
					return patch > value.patch;
				}
			}

		}
		bool operator<( Version value ) const
		{
			if( major < value.major )
			{
				return true;
			}
			else
			{
				if( minor < value.minor )
				{
					return true;
				}
				else
				{
					return patch < value.patch;
				}
			}
		}

        bool operator<(Version& value) const
        {
            if (major < value.major)
            {
				return true;
            }
            else
            {
                if (minor < value.minor)
                {
					return true;
                }
                else
                {
					return patch < value.patch;
                }
            }
        }

        bool operator>=(Version& value) const
        {
			
            if (major > value.major)
            {
				return true;
            }
            else if (major == value.major)
            {
                if (minor > value.minor)
                {
					return true;
                }
                else if (minor == value.minor)
                {
                    if (patch >= value.patch)
                    {
						return true;
                    }
                    else
                    {
						return false;
                    }
                }
                else
                {
					return false;
                }
            }
            else
            {
				return false;
            }
        }

        // TODO duplicate logic above
        bool operator>=(Version value) const
        {
			
            if (major > value.major)
            {
				return true;
            }
            else if (major == value.major)
            {
                if (minor > value.minor)
                {
					return true;
                }
                else if (minor == value.minor)
                {
                    if (patch >= value.patch)
                    {
						return true;
                    }
                    else
                    {
						return false;
                    }
                }
                else
                {
					return false;
                }
            }
            else
            {
				return false;
            }
        }

		unsigned int major;

        unsigned int minor;

        unsigned int patch;

        std::string ToString() const
        {
			std::stringstream ss;
			ss << major << "." << minor << "." << patch;
			return ss.str();
        }

        bool FromString(std::string versionString)
        {
			try
			{
				std::stringstream ss( versionString );

				char delimiter = '.';

				std::string majorStr = "";

				if( !std::getline( ss, majorStr, delimiter ) )
				{
					return false;
				}

				major = std::stoi( majorStr );

				std::string minorStr = "";

				if( !std::getline( ss, minorStr, delimiter ) )
				{
					return false;
				}

				minor = std::stoi( minorStr );

				std::string patchStr = "";

				if( !std::getline( ss, patchStr, delimiter ) )
				{
					return false;
				}

				patch = std::stoi( patchStr );

                return true;
			}
            catch (...)
            {
				return false;
            }
        }
    };

    enum class Result
    {
	    SUCCESS,
	    FAIL,
	    UNSUPPORTED_FILE_FORMAT,
	    FAILED_TO_OPEN_FILE,
	    MALFORMED_RESOURCE_INPUT,
	    FILE_TYPE_MISMATCH,
		DOCUMENT_VERSION_UNSUPPORTED,
		REQUIRED_RESOURCE_PARAMETER_NOT_SET,
		FAILED_TO_OPEN_LOCAL_FILE,
        FAILED_TO_OPEN_REMOTE_FILE,
		INVALID_FILENAME,
		FAILED_TO_CREATE_PATCH,
		FAILED_TO_SAVE_FILE,
		FAILED_TO_GENERATE_CHECKSUM,
		FAILED_TO_GENERATE_RELATIVE_PATH_CHECKSUM,
		FAILED_TO_COMPRESS_DATA,
		PATCH_RESOURCE_LIST_MISSMATCH,
		DESTINATION_RESOURCE_NOT_IN_PATCH_RESOURCE_GROUP,
		FAILED_TO_APPLY_PATCH,
		UNEXPECTED_PATCH_CHECKSUM_RESULT,
		UNEXPECTED_PATCH_DIFF_ENCOUNTERED,
		FILE_NOT_FOUND,
		FAILED_TO_RETRIEVE_CHUNK_DATA,
		RESOURCE_VALUE_NOT_SET,
		UNEXPECTED_END_OF_CHUNKS,
		UNEXPECTED_CHUNK_CHECKSUM_RESULT
    };

    static const Version S_LIBRARY_VERSION = { 1, 0, 0 };

    static const Version S_DOCUMENT_VERSION = { 0, 1, 0 };

}

#endif // Enums_H