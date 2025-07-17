// Copyright © 2025 CCP ehf.

#include "VersionInternal.h"
#include "Enums.h"

namespace CarbonResources
{
    VersionInternal::VersionInternal()
    {
    }

    VersionInternal::VersionInternal( const Version& v ) :
		m_major(v.major),
		m_minor(v.minor),
		m_patch(v.patch)
    {
        
    }

    VersionInternal::VersionInternal( unsigned int major, unsigned int minor, unsigned int patch ) :
	    m_major( major ),
	    m_minor( minor ),
	    m_patch( patch )
    {
    }

    bool VersionInternal::operator>( const VersionInternal& value ) const
    {
	    if( m_major > value.m_major )
	    {
		    return true;
	    }
	    else
	    {
		    if( m_minor > value.m_minor )
		    {
			    return true;
		    }
		    else
		    {
			    return m_patch > value.m_patch;
		    }
	    }
    }

    bool VersionInternal::operator<( const VersionInternal& value ) const
    {
	    if( m_major < value.m_major )
	    {
		    return true;
	    }
	    else
	    {
		    if( m_minor < value.m_minor )
		    {
			    return true;
		    }
		    else
		    {
			    return m_patch < value.m_patch;
		    }
	    }
    }

    bool VersionInternal::operator>=( const VersionInternal& value ) const
    {

	    if( m_major > value.m_major )
	    {
		    return true;
	    }
	    else if( m_major == value.m_major )
	    {
		    if( m_minor > value.m_minor )
		    {
			    return true;
		    }
		    else if( m_minor == value.m_minor )
		    {
			    if( m_patch >= value.m_patch )
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

    bool VersionInternal::operator == ( const VersionInternal& value ) const
    {
        if (value.m_major != m_major)
        {
			return false;
        }
        else if (value.m_minor != m_minor)
        {
			return false;
        }
        else if (value.m_patch != m_patch)
        {
			return false;
        }
        else
        {
			return true;
        }
    }

    bool VersionInternal::operator<=( const VersionInternal& value ) const
    {
	    return !(*this > value);
    }

    std::string VersionInternal::ToString() const
    {
	    std::stringstream ss;
	    ss << m_major << "." << m_minor << "." << m_patch;
	    return ss.str();
    }

    bool VersionInternal::FromString( std::string versionString )
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

		    m_major = std::stoi( majorStr );

		    std::string minorStr = "";

		    if( !std::getline( ss, minorStr, delimiter ) )
		    {
			    return false;
		    }

		    m_minor = std::stoi( minorStr );

		    std::string patchStr = "";

		    if( !std::getline( ss, patchStr, delimiter ) )
		    {
			    return false;
		    }

		    m_patch = std::stoi( patchStr );

		    return true;
	    }
	    catch( ... )
	    {
		    return false;
	    }
    }

    unsigned int VersionInternal::getMajor() const
    {
		return m_major;
    }

    unsigned int VersionInternal::getMinor() const
    {
		return m_minor;
    }

    unsigned int VersionInternal::getPatch() const
    {
		return m_patch;
    }

    bool VersionInternal::isVersionValid() const
    {
        for (Version v : S_VALID_DOCUMENT_VERSIONS)
        {
			VersionInternal internalVersion( v );

            if (this->operator==(internalVersion))
            {
				return true;
            }
        }

        return false;
    }
}