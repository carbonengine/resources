// Copyright © 2025 CCP ehf.

#include <stdexcept>
#include <sstream>
#include <FilterResourcePathFile.h>


namespace ResourceTools
{

FilterResourcePathFile::FilterResourcePathFile( std::string rawPathFileAttrib,
												const FilterPrefixMap& parentPrefixMap,
												const FilterResourceFilter& parentSectionFilter ) :
	m_rawPathFileAttrib( std::move( rawPathFileAttrib ) ),
	m_parentPrefixMap( parentPrefixMap ),
	m_parentSectionFilter( parentSectionFilter )
{
	m_resolvedPathMap.clear();

	ParseRawPathFileAttribute();
}

const std::map<std::string, FilterResourceFilter>& FilterResourcePathFile::GetResolvedPathMap() const
{
	return m_resolvedPathMap;
}

void FilterResourcePathFile::ParseRawPathFileAttribute()
{
	// Split m_rawPathFileAttrib into lines (in case of multiline attribute)
	std::istringstream stream( m_rawPathFileAttrib );
	std::string line;
	while( std::getline( stream, line ) )
	{
		// Trim whitespace from both ends
		size_t first = line.find_first_not_of( " \t\r" );
		if( first == std::string::npos )
			continue; // skip if empty line

		size_t last = line.find_last_not_of( " \t\r" );
		std::string rawPathLine = line.substr( first, last - first + 1 );

		// Skip commented out lines (in case there is "inline" comment within the .ini file attribute value)
		if( rawPathLine.empty() || rawPathLine[0] == '#' || rawPathLine[0] == ';' )
			continue;

		// Add entries to the resolved path map
		auto lineEntry = FilterResourcePathFileEntry( rawPathLine, m_parentPrefixMap, m_parentSectionFilter );
		const auto resolvedPaths = lineEntry.GetResolvedPaths();
		const auto entryFilter = lineEntry.GetEntryFilter();
		for( const auto& path : resolvedPaths )
		{
			m_resolvedPathMap.insert_or_assign( path, entryFilter );
		}
	}
}

}
