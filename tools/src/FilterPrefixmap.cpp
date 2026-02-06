// Copyright © 2025 CCP ehf.

#include "FilterPrefixmap.h"

#include <cctype>
#include <stdexcept>

namespace ResourceTools
{

// -------------------------------------------------------------
// Description:
//   Constructs a FilterPrefixMap object from a raw prefixmap string
//   by calling the ParsePrefixMap() private function.
// Arguments:
//   rawPrefixMap - string representation of the raw prefixmap attribute
// -------------------------------------------------------------
FilterPrefixMap::FilterPrefixMap( const std::string& rawPrefixMap )
{
	ParsePrefixMap( rawPrefixMap );
}

// -------------------------------------------------------------
// Description:
//   Gets the map of keyed prefixes to FilterPrefixMapEntry objects.
//   The FilterPrefixMapEntry objects contain a set of parsed paths for each prefix.
// Return Value:
//   map of prefixes to FilterPrefixMapEntry objects
// -------------------------------------------------------------
const std::map<std::string, FilterPrefixMapEntry>& FilterPrefixMap::GetMapEntries() const
{
	return m_prefixMapEntries;
}

// -------------------------------------------------------------
// Description:
//   Parses the raw prefixmap string and populates m_prefixMapEntries
//   with the corresponding FilterPrefixMapEntry objects.
//   The raw prefixmap string is expected to be in the format:
//     "prefix:path1;path2 prefix2:path3"
//   The function throws std::invalid_argument if format is invalid.
// Arguments:
//   rawPrefixMap - string representation of the raw prefixmap attribute
// Return Value:
//   None (void)
// -------------------------------------------------------------
void FilterPrefixMap::ParsePrefixMap( const std::string& rawPrefixMap )
{
	std::size_t pos = 0;
	while( pos < rawPrefixMap.size() )
	{
		// Find the prefix (or error out if missing a colon ":")
		std::size_t colon = rawPrefixMap.find( ':', pos );
		if( colon == std::string::npos )
		{
			throw std::invalid_argument( "Invalid prefixmap format: missing ':'" );
		}

		std::string prefix = rawPrefixMap.substr( pos, colon - pos );
		if( prefix.empty() )
		{
			throw std::invalid_argument( "Invalid prefixmap format: empty prefix" );
		}

		// Move position past the colon
		pos = colon + 1;

		// Find end of paths (next whitespace or end of string)
		std::size_t nextSpace = rawPrefixMap.find_first_of( " \t\r\n", pos );
		std::string rawPaths = ( nextSpace == std::string::npos ) ?
			rawPrefixMap.substr( pos ) :
			rawPrefixMap.substr( pos, nextSpace - pos );

		if( rawPaths.empty() )
		{
			throw std::invalid_argument( "Invalid prefixmap format: No paths defined for prefix: " + prefix );
		}

		auto it = m_prefixMapEntries.find( prefix );
		if( it == m_prefixMapEntries.end() )
		{
			// Prefix doesn't exist, create a map entry for it.
			m_prefixMapEntries.insert_or_assign( prefix, FilterPrefixMapEntry( prefix, rawPaths ) );
		}
		else
		{
			// The same prefix has been found again, appending paths to the existing entry
			it->second.AppendPaths( prefix, rawPaths );
		}

		// Go to the next token in the rawPrefixMap (or break if at end)
		if( nextSpace == std::string::npos )
		{
			break;
		}
		pos = nextSpace + 1;

		// There was a whitespace, skip any additional spaces as well
		while( pos < rawPrefixMap.size() && std::isspace( static_cast<unsigned char>( rawPrefixMap[pos] ) ) )
		{
			++pos;
		}
	}
}

}