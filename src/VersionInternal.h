/* 
	*************************************************************************

	VersionInternal.h

	Author:    James Hawk
	Created:   March. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/

#pragma once
#ifndef VersionInternal_H
#define VersionInternal_H

#include <memory>
#include <string>
#include <sstream>
#include "Exports.h"


namespace CarbonResources
{
    struct Version;

    class VersionInternal
    {
	public:
		VersionInternal();

        VersionInternal( const Version& v );

        VersionInternal( unsigned int major, unsigned int minor, unsigned int patch );

		bool operator>( VersionInternal value ) const;

		bool operator>( VersionInternal& value ) const;

		bool operator<( VersionInternal value ) const;

        bool operator<( VersionInternal& value ) const;

        bool operator>=( VersionInternal& value ) const;

        bool operator>=( VersionInternal value ) const;

        std::string ToString() const;

		bool FromString( std::string versionString );

        unsigned int getMajor() const;

        unsigned int getMinor() const;

        unsigned int getPatch() const;

    private:

		unsigned int m_major;

        unsigned int m_minor;

        unsigned int m_patch;
        
    };

}

#endif // VersionInternal_H