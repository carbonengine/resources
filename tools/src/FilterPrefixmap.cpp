// Copyright © 2025 CCP ehf.

#include <stdexcept>
#include <cctype>
#include <FilterPrefixMap.h>
#include <FilterPrefixMapEntry.h>

namespace ResourceTools
{

FilterPrefixMap::FilterPrefixMap( const std::string& rawPrefixMap )
{
	m_prefixMapEntries.clear();

	ParsePrefixMap( rawPrefixMap );
}

const std::map<std::string, FilterPrefixMapEntry>& FilterPrefixMap::GetMapEntries() const
{
	return m_prefixMapEntries;
}

void FilterPrefixMap::ParsePrefixMap( const std::string& rawPrefixMap )
{
	std::size_t pos = 0;
	while( pos < rawPrefixMap.size() )
	{
		// Find the prefix (or error out if missing a colon)
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
		std::string rawPaths = ( nextSpace == std::string::npos ) ? rawPrefixMap.substr( pos ) : rawPrefixMap.substr( pos, nextSpace - pos );

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
			break;
		pos = nextSpace + 1;

		// There was a whitespace, skip any additional spaces as well
		while( pos < rawPrefixMap.size() && std::isspace( static_cast<unsigned char>( rawPrefixMap[pos] ) ) )
			++pos;
	}
}

}