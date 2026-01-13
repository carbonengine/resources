// Copyright © 2025 CCP ehf.

#include <sstream>
#include <FilterResourcePathFileEntry.h>

namespace ResourceTools
{

FilterResourcePathFileEntry::FilterResourcePathFileEntry( std::string rawPathLine,
														  const FilterPrefixMap& parentPrefixMap,
														  const FilterResourceFilter& parentSectionFilter ) :
	m_rawPathLine( std::move( rawPathLine ) ),
	m_parentPrefixMap( parentPrefixMap ),
	m_parentSectionFilter( parentSectionFilter )
{
	ParseRawPathLine();
}


const FilterResourceFilter& FilterResourcePathFileEntry::GetEntryFilter() const
{
	return m_entryFilter;
}

const std::set<std::string>& FilterResourcePathFileEntry::GetResolvedPaths() const
{
	return m_resolvedPaths;
}

void FilterResourcePathFileEntry::ParseRawPathLine()
{
	// Split on whitespace: first token is pathPart, rest is (optional) filterPart
	std::string rawPathToken;
	std::string rawOptionalFilterPart;
	std::string combinedRawFilter;

	std::istringstream iss( m_rawPathLine );
	iss >> rawPathToken;
	if( !iss.eof() )
	{
		// There is an optional filter part. Construct it, will error out if wrong format
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
	m_entryFilter = FilterResourceFilter( combinedRawFilter );

	// Validate the rawPathToken
	size_t colon = rawPathToken.find( ':' );
	if( colon == std::string::npos )
	{
		// TODO: Change this to a defined error code/type
		throw std::invalid_argument( std::string( "Missing prefix in path for: " ) + m_rawPathLine );
	}
	std::string prefix = rawPathToken.substr( 0, colon );
	std::string rest = rawPathToken.substr( colon + 1 );

	const auto& prefixMapEntries = m_parentPrefixMap.GetMapEntries();
	auto it = prefixMapEntries.find( prefix );
	if( it == prefixMapEntries.end() )
	{
		// TODO: Change this to a defined error code/type
		throw std::invalid_argument( std::string( "Prefix '" ) + prefix + "' not present in prefixMap for line: " + m_rawPathLine );
	}

	// Each FilterPrefixMapEntry may have multiple paths, combine/resolve all of them
	const auto& prefixEntry = it->second;
	const auto& prefixPaths = prefixEntry.GetPaths();
	for( const auto& basePrefixPath : prefixPaths )
	{
		// Ensure only one '/' at the join point
		bool baseEndsWithSlash = !basePrefixPath.empty() && basePrefixPath.back() == '/';
		bool restStartsWithSlash = !rest.empty() && rest.front() == '/';
		std::string resolvedPath = basePrefixPath;
		if( baseEndsWithSlash && restStartsWithSlash )
		{
			resolvedPath += rest.substr( 1 );
		}
		else if( !baseEndsWithSlash && !restStartsWithSlash )
		{
			resolvedPath += '/' + rest;
		}
		else
		{
			resolvedPath = basePrefixPath + rest;
		}
		m_resolvedPaths.insert( resolvedPath );
	}
}

}