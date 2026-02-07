// Copyright © 2025 CCP ehf.

#include "FilterResourcePathFile.h"

#include <sstream>

#include "FilterResourcePathFileEntry.h"

namespace ResourceTools
{

// -------------------------------------------------------------
// Description:
//   Constructs a FilterResourcePathFile object for a resfile/respaths attribute.
// Arguments:
//   rawPathFileAttrib - the raw string for this resfile/respaths attribute from the filter .ini file.
//     This can contain multiple lines of path entries, see FilterResourcePathFileEntry class.
//   parentPrefixMap - the FilterPrefixMap from the [DEFAULT] section, used to
//     resolve prefixes in the resfile/respaths attribute to actual paths.
//   parentSectionFilter - the FilterResourceFilter from the same [namedSection] as this
//     resfile/respaths attribute. Needed to create actual filters (some with optional inline)
//     for each line entry (FilterResourcePathFileEntry) of the resfile/respaths attribute.
// Note:
//   The parsing of the rawPathFileAttrib takes place in the ParseRawPathFileAttribute() function.
// -------------------------------------------------------------
FilterResourcePathFile::FilterResourcePathFile( const std::string& rawPathFileAttrib,
												const FilterPrefixMap& parentPrefixMap,
												const FilterResourceFilter& parentSectionFilter ) :
	m_parentPrefixMap( parentPrefixMap ),
	m_parentSectionFilter( parentSectionFilter )
{
	ParseRawPathFileAttribute( rawPathFileAttrib );
}

// -------------------------------------------------------------
// Description:
//   Gets a map of fully resolved relative paths and their
//   associated FilterResourceFilter include/exclude filters.
// Return Value:
//   Map with a key = resolved relative path and value of
//   the associated include/exclude filters.
// -------------------------------------------------------------
const std::map<std::string, FilterResourceFilter>& FilterResourcePathFile::GetResolvedPathMap() const
{
	return m_resolvedPathMap;
}

// -------------------------------------------------------------
// Description:
//   Parses the rawPathFileAttrib and populates the m_resolvedPathMap.
// Arguments:
//   rawPathFileAttrib - the raw string for this resfile/respaths attribute
//      from the filter .ini file. This can contain multiple lines of path
//      entries that is represented by the FilterResourcePathFileEntry class.
// -------------------------------------------------------------
void FilterResourcePathFile::ParseRawPathFileAttribute( const std::string& rawPathFileAttrib )
{
	// Split rawPathFileAttrib into lines (in case of multiline attribute)
	std::istringstream stream( rawPathFileAttrib );
	std::string line;

	while( std::getline( stream, line ) )
	{
		// Trim whitespace from both ends
		size_t first = line.find_first_not_of( " \t\r" );
		if( first == std::string::npos )
		{
			continue; // skip if empty line
		}

		size_t last = line.find_last_not_of( " \t\r" );
		std::string rawPathLine = line.substr( first, last - first + 1 );

		// Skip commented out lines (in case there is "inline" comment within the .ini file attribute value)
		if( rawPathLine.empty() || rawPathLine[0] == '#' || rawPathLine[0] == ';' )
		{
			continue;
		}

		// Add entries to the resolved path map
		auto lineEntry = FilterResourcePathFileEntry( rawPathLine, m_parentPrefixMap, m_parentSectionFilter );
		const auto entryFilter = lineEntry.GetEntryFilter();
		const auto resolvedPaths = lineEntry.GetResolvedPaths();

		for( const auto& path : resolvedPaths )
		{
			// Check if the path already exists in the map, to determine if se should combine filters or not.
			auto foundMapItem = m_resolvedPathMap.find( path );
			if( foundMapItem != m_resolvedPathMap.end() )
			{
				// Combine the raw filters from the previous and current entry
				const std::string& prevRawFilter = foundMapItem->second.GetRawFilter();
				const std::string& currRawFilter = entryFilter.GetRawFilter();
				std::string combinedRawFilter;

				if( !prevRawFilter.empty() && !currRawFilter.empty() )
				{
					combinedRawFilter = prevRawFilter + " " + currRawFilter;
				}
				else if( !prevRawFilter.empty() )
				{
					combinedRawFilter = prevRawFilter;
				}
				else
				{
					combinedRawFilter = currRawFilter;
				}
				// Update the map with the combined filter
				m_resolvedPathMap.insert_or_assign( path, FilterResourceFilter( combinedRawFilter ) );
			}
			else
			{
				// Path not present, just insert as is
				m_resolvedPathMap.insert( { path, entryFilter } );
			}
		}
	}
}

}
