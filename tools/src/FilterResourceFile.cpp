// Copyright © 2025 CCP ehf.

#include <stdexcept>
#include <INIReader.h>
#include <FilterResourceFile.h>

namespace ResourceTools
{

FilterResourceFile::FilterResourceFile( const std::filesystem::path& iniFilePath ) :
	m_iniFilePath( iniFilePath )
{
	m_iniFileResolvedPathMap.clear();

	ParseIniFile();
}

const std::map<std::string, FilterResourceFilter>& FilterResourceFile::GetIniFileResolvedPathMap()
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
					m_iniFileResolvedPathMap.insert_or_assign( kv.first, kv.second );
				}
			}
		}
	}

	return m_iniFileResolvedPathMap;
}

void FilterResourceFile::ParseIniFile()
{
	// Open, read and parse the resource INI file.
	INIReader reader( m_iniFilePath.generic_string() );
	if( reader.ParseError() != 0 )
	{
		throw std::runtime_error( "Failed to parse INI file: " + m_iniFilePath.generic_string() + " - " + reader.ParseErrorMessage() );
	}

	// Parse the [DEFAULT] section
	if( !reader.HasSection( "DEFAULT" ) )
	{
		throw std::invalid_argument( "Missing [DEFAULT] section in INI file: " + m_iniFilePath.generic_string() );
	}
	m_defaultSection = FilterDefaultSection( reader.Get( "DEFAULT", "prefixmap", "" ) );


	// Validate that non-DEFAULT section(s) exist
	std::vector<std::string> allSections = reader.Sections();
	if( allSections.size() <= 1 )
	{
		// No namedSections defined
		throw std::invalid_argument( "No namedSections defined in INI file: " + m_iniFilePath.generic_string() );
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

}
