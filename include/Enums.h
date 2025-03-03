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

#define ApiExport __declspec( dllexport )

namespace CarbonResources
{
    struct Version
    {
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
		REQUIRED_RESOURCE_PARAMETER_NOT_SET
    };

    static const Version S_LIBRARY_VERSION = { 1, 0, 0 };

    static const Version S_DOCUMENT_VERSION = { 0, 1, 0 };

    #define BINARY_GUARD_RETURN( v_major, v_minor, v_patch ) \
		const Version v = { v_major, v_minor, v_patch };     \
		if( S_LIBRARY_VERSION < v )                          \
		{                                                    \
			return Result::SUCCESS;                          \
		}

}

#endif // Enums_H