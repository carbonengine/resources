// Copyright © 2025 CCP ehf.

#include "FilterPrefixMapEntry.h"

#include <stdexcept>

namespace ResourceTools
{

// -------------------------------------------------------------
// Description:
//   Constructs a FilterPrefixMapEntry object for a given prefix
//   and the ";" semicolon separated rawPaths string.
// Arguments:
//   prefix - identifier of the prefix the paths belong to (e.g: "prefix1")
//   rawPaths - string of one or more paths separated by ";" (e.g: pathA;pathB)
// -------------------------------------------------------------
FilterPrefixMapEntry::FilterPrefixMapEntry( const std::string& prefix, const std::string& rawPaths ) :
	m_prefix( prefix )
{
	AppendPaths( prefix, rawPaths );
}

// -------------------------------------------------------------
// Description:
//   Parses rawPaths and appends individual path entries to the m_paths set
//   for this prefix.
// Arguments:
//   prefix - identifier of the prefix the paths belong to (e.g: "prefix1")
//   rawPaths - string of one or more paths separated by ";" (e.g: pathA;pathB)
// -------------------------------------------------------------
void FilterPrefixMapEntry::AppendPaths( const std::string& prefix, const std::string& rawPaths )
{
	if( prefix != m_prefix )
	{
		throw std::invalid_argument( "Prefix mismatch while appending path(s): " + prefix + " (incoming) != " + m_prefix + " (existing)" );
	}

	std::size_t pos = 0;
	// Loop through rawPaths and split by semicolons to extract individual paths (in case there are many)
	while( pos < rawPaths.size() )
	{
		// Split the string up by semicolons (in case of multiple paths)
		std::size_t semicolon = rawPaths.find( ';', pos );
		std::string path = ( semicolon == std::string::npos ) ?
			rawPaths.substr( pos ) :
			rawPaths.substr( pos, semicolon - pos );

		if( !path.empty() )
		{
			m_paths.insert( path );
		}

		if( semicolon == std::string::npos )
		{
			break;
		}
		pos = semicolon + 1;
	}

	if( m_paths.empty() )
	{
		throw std::invalid_argument( "Invalid prefixmap format: No paths appended for prefix: " + m_prefix );
	}
}

// -------------------------------------------------------------
// Description:
//   Gets the prefix identifier for this entry.
// Return Value:
//   String representation of the prefix identifier (e.g: "prefix1")
// -------------------------------------------------------------
const std::string& FilterPrefixMapEntry::GetPrefix() const
{
	return m_prefix;
}

// -------------------------------------------------------------
// Description:
//  Gets the ordered set of parsed paths for this entry.
// Return Value:
//   A set of strings representing the paths associated with
//   this prefix (e.g: {"pathA", "pathB"})
// -------------------------------------------------------------
const std::set<std::string>& FilterPrefixMapEntry::GetPaths() const
{
	return m_paths;
}

}
