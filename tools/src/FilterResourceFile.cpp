// Copyright © 2025 CCP ehf.

#include "FilterResourceFile.h"

#include <stdexcept>

#include "INIReader.h"

namespace ResourceTools
{

// -------------------------------------------------------------
// Description:
//   Construct a FilterResourceFile object by parsing the supplied resource .ini file.
// Arguments:
//   iniFilePath - the file path to the resource .ini file to parse.
// -------------------------------------------------------------
FilterResourceFile::FilterResourceFile( const std::filesystem::path& iniFilePath )
{
	ParseIniFile( iniFilePath );
	PopulateIniFileResolvedPathMap();
}

// -------------------------------------------------------------
// Description:
//   Returns the fully resolved PathMaps for all named sections within this .ini file.
// Return Value:
//   Map of resolved paths to their associated filters:
//   - Key = "resolved path"
//   - Value = associated include/exclude filters
// -------------------------------------------------------------
const std::map<std::string, FilterResourceFilter>& FilterResourceFile::GetIniFileResolvedPathMap() const
{
	return m_iniFileResolvedPathMap;
}

// -------------------------------------------------------------
// Description:
//   Parses the resource .ini file and populates the m_defaultSection and
//   m_namedSections members from the [DEFAULT] and [NamedSection(s)] respectively.
// Arguments:
//   iniFilePath - the file path to the resource .ini file to parse.
// Return Value:
//   None (void).
// -------------------------------------------------------------
void FilterResourceFile::ParseIniFile( const std::filesystem::path& iniFilePath )
{
	// Open, read and parse the resource INI file.
	INIReader reader( iniFilePath.generic_string() );
	if( reader.ParseError() != 0 )
	{
		throw std::runtime_error( "Failed to parse INI file: " + iniFilePath.generic_string() + " - " + reader.ParseErrorMessage() );
	}

	// Parse the [DEFAULT] section
	if( !reader.HasSection( "DEFAULT" ) )
	{
		throw std::invalid_argument( "Missing [DEFAULT] section in INI file: " + iniFilePath.generic_string() );
	}
	m_defaultSection = FilterDefaultSection( reader.Get( "DEFAULT", "prefixmap", "" ) );


	// Validate that non-DEFAULT section(s) exist
	std::vector<std::string> allSections = reader.Sections();
	if( allSections.size() <= 1 )
	{
		// No namedSections defined
		throw std::invalid_argument( "No [namedSection] defined in INI file: " + iniFilePath.generic_string() );
	}

	// Parse all other named sections
	for( const auto& sectionName : reader.Sections() )
	{
		if( sectionName == "default" || sectionName == "DEFAULT" )
		{
			continue; // Already loaded, skip it
		}

		std::string filter = reader.Get( sectionName, "filter", "" );
		std::string respaths = reader.Get( sectionName, "respaths", "" );
		std::string resfile = reader.Get( sectionName, "resfile", "" );

		if( respaths.empty() )
		{
			throw std::invalid_argument( "Respaths attribute is empty for section: " + sectionName );
		}

		FilterNamedSection namedSection( sectionName, filter, respaths, resfile, m_defaultSection.GetPrefixMap() );
		m_namedSections.push_back( namedSection );
	}
}

// -------------------------------------------------------------
// Description:
//   Populate the fully resolved PathMaps for all [namedSections]
//   in this .ini file.
// Return Value:
//   None (void).
// -------------------------------------------------------------
void FilterResourceFile::PopulateIniFileResolvedPathMap()
{
	if( m_iniFileResolvedPathMap.empty() )
	{
		// Populate the full resolved path map from all named sections in this INI file
		for( auto& namedSection : m_namedSections )
		{
			auto& sectionPathMap = namedSection.GetCombinedResolvedPathMap();
			for( const auto& kv : sectionPathMap )
			{
				// Combine filters if the same path already exists
				auto it = m_iniFileResolvedPathMap.find( kv.first );
				if( it != m_iniFileResolvedPathMap.end() )
				{
					// Combine the filters (using raw filter strings)
					std::string combinedRawFilter = it->second.GetRawFilter() + " " + kv.second.GetRawFilter();
					FilterResourceFilter combinedFilter( combinedRawFilter );
					m_iniFileResolvedPathMap.insert_or_assign( kv.first, combinedFilter );
				}
				else
				{
					m_iniFileResolvedPathMap.insert( { kv.first, kv.second } );
				}
			}
		}
	}
}

}
