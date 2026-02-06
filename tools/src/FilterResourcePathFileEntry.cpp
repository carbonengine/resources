// Copyright © 2025 CCP ehf.

#include "FilterResourcePathFileEntry.h"

#include <sstream>
#include <stdexcept>

namespace ResourceTools
{

// -------------------------------------------------------------
// Description:
//   Construct a FilterResourcePathFileEntry object for a single resfile/respaths attribute line entry.
// Arguments:
//   rawPathLine - the raw string for this line entry from the .ini file
//       e.g: "prefix1:/pathA/*  [ .txt ] ![ .yaml]
//   parentPrefixMap - the FilterPrefixMap from the [DEFAULT] section,
//       used to resolve prefixes in this line entry to actual paths.
//   parentSectionFilter - the FilterResourceFilter from the same
//       [namedSection] as this resfile/respaths attribute.
//       Needed to create actual filter (with optional inline one) for this line entry.
// -------------------------------------------------------------
FilterResourcePathFileEntry::FilterResourcePathFileEntry( const std::string& rawPathLine,
														  const FilterPrefixMap& parentPrefixMap,
														  const FilterResourceFilter& parentSectionFilter ) :
	m_parentPrefixMap( parentPrefixMap ),
	m_parentSectionFilter( parentSectionFilter )
{
	ParseRawPathLine( rawPathLine );
}

// -------------------------------------------------------------
// Description:
//   Gets the (possibly combined) filter for this resfile/respaths attribute line entry.
// Return Value:
//   FilterResourceFilter object representing the combined
//   include/exclude filters for this line entry.
// -------------------------------------------------------------
const FilterResourceFilter& FilterResourcePathFileEntry::GetEntryFilter() const
{
	return m_entryFilter;
}

// -------------------------------------------------------------
// Description:
//   Get resolved paths set for this resfile/respaths attribute line entry.
// Return Value:
//   Set of strings representing the resolved relative paths for this line entry.
// -------------------------------------------------------------
const std::set<std::string>& FilterResourcePathFileEntry::GetResolvedPaths() const
{
	return m_resolvedPaths;
}

// -------------------------------------------------------------
// Description:
//   Parses the rawPathLine and constructs/populates the
//   m_entryFilter and m_resolvedPaths members.
// Arguments:
//   rawPathLine - the raw string for this line entry
//       e.g: "prefix1:/pathA/*  [ .txt ] ![ .yaml]"
// -------------------------------------------------------------
void FilterResourcePathFileEntry::ParseRawPathLine( const std::string& rawPathLine )
{
	std::string rawPrefixPathToken;
	std::string combinedRawFilter;

	// Split on whitespace:
	// - first token is the prefix:pathPart,
	// - rest would be the (optional) filterPart (to be read at later stage)
	std::istringstream iss( rawPathLine );
	iss >> rawPrefixPathToken;

	// Validate that the rawPathToken is of the correct format "prefix:pathPart"
	size_t colon = rawPrefixPathToken.find( ':' );
	if( colon == std::string::npos )
	{
		throw std::invalid_argument( std::string( "Missing prefix in path for: " ) + rawPathLine );
	}
	std::string prefixPart = rawPrefixPathToken.substr( 0, colon );
	std::string pathPart = rawPrefixPathToken.substr( colon + 1 );

	// Now figure out which filter to use (inline or parent)
	if( !iss.eof() )
	{
		// There is more data, i.e. an optional filter exists.
		// Construct it from the rest of the "line", will error out if filter format is wrong.
		std::string rawOptionalFilterPart;
		std::getline( iss, rawOptionalFilterPart );
		FilterResourceFilter inlineFilter = FilterResourceFilter( rawOptionalFilterPart );

		// Combine parent filter and inline filter
		combinedRawFilter = m_parentSectionFilter.GetRawFilter() + " " + inlineFilter.GetRawFilter();
	}
	else
	{
		// No inline filter, use parent section filter as is
		combinedRawFilter = m_parentSectionFilter.GetRawFilter();
	}

	// Construct the parsed (potentially) combined filter for this line entry.
	m_entryFilter = FilterResourceFilter( combinedRawFilter );

	// Check that the prefix from the rawPathToken exists in the parent [DEFAULT] section prefix map.
	const auto& prefixMapEntries = m_parentPrefixMap.GetMapEntries();
	auto foundPrefixMapEntry = prefixMapEntries.find( prefixPart );
	if( foundPrefixMapEntry == prefixMapEntries.end() )
	{
		throw std::invalid_argument( std::string( "Prefix '" ) + prefixPart + "' not present in prefixMap for line: " + rawPathLine );
	}

	// Each FilterPrefixMapEntry may have multiple paths, combine/resolve for all of those paths
	const auto& prefixEntry = foundPrefixMapEntry->second;
	const auto& prefixPathsSet = prefixEntry.GetPaths();
	for( const auto& basePrefixMapPath : prefixPathsSet )
	{
		// Ensure only one '/' at the join point
		bool baseEndsWithSlash = !basePrefixMapPath.empty() && basePrefixMapPath.back() == '/';
		bool restStartsWithSlash = !pathPart.empty() && pathPart.front() == '/';

		std::string resolvedPath = basePrefixMapPath;
		if( baseEndsWithSlash && restStartsWithSlash )
		{
			resolvedPath += pathPart.substr( 1 );
		}
		else if( !baseEndsWithSlash && !restStartsWithSlash )
		{
			resolvedPath += '/' + pathPart;
		}
		else
		{
			resolvedPath = basePrefixMapPath + pathPart;
		}
		m_resolvedPaths.insert( resolvedPath );
	}
}

}