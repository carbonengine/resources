// Copyright © 2025 CCP ehf.

#include "FilterResourceFilter.h"

#include <cctype>
#include <sstream>
#include <stdexcept>

namespace ResourceTools
{

// -------------------------------------------------------------
// Description:
//   Construct a FilterResourceFilter object by parsing the given raw filter string.
// Arguments:
//   rawFilter - the raw filter string to parse (e.g: "[ .yaml .txt ] ![ .exe ]")
//   isTopLevelFilter - indicates whether the level of the filter:
//     - true  = filter is from the "filter" attribute of a [NamedSection]
//     - false = (default value) filter is an inline filter of a respaths/resfile line entry.
// -------------------------------------------------------------
FilterResourceFilter::FilterResourceFilter( const std::string& rawFilter, bool isToplevelFilter /* = false */ ) :
	m_rawFilter( rawFilter ),
	m_isToplevelFilter( isToplevelFilter )
{
	ParseFilters();
}

// -------------------------------------------------------------
// Description:
//   Gets the raw filter string attribute from the .ini file,
//   e.g: "[ .yaml .txt ] ![ .exe ]", either topLevel or inLine.
// Return Value:
//   The raw string representation of the filter.
// Note:
//   This function is needed as input to easily construct combined
//   filters for "topLevel parent" and respaths/resfile attribute
//   line filter entries, from both of their raw representation.
// -------------------------------------------------------------
const std::string& FilterResourceFilter::GetRawFilter() const
{
	return m_rawFilter;
}

// -------------------------------------------------------------
// Description:
//   Gets the parsed include filter vector.
// Return Value:
//   Vector of strings representing the include filter tokens, e.g: { ".yaml", ".txt" }.
// -------------------------------------------------------------
const std::vector<std::string>& FilterResourceFilter::GetIncludeFilter() const
{
	return m_includeFilter;
}

// -------------------------------------------------------------
// Description:
//   Gets the parsed exclude filter vector.
// Return Value:
//   Vector of strings representing the exclude filter tokens, e.g: { ".exclude" }.
// -------------------------------------------------------------
const std::vector<std::string>& FilterResourceFilter::GetExcludeFilter() const
{
	return m_excludeFilter;
}

// -------------------------------------------------------------
// Description:
//   Parses the raw filter string into the include and exclude
//   filter vectors and places it in the correct vector.
//   The raw filter string is expected to be in the format of one or more sections of the form:
// -------------------------------------------------------------
void FilterResourceFilter::ParseFilters()
{
	std::string s = m_rawFilter;
	size_t pos = 0;
	while( pos < s.size() )
	{
		// Skip whitespaces
		while( pos < s.size() && std::isspace( static_cast<unsigned char>( s[pos] ) ) )
		{
			++pos;
		}
		if( pos >= s.size() )
		{
			break;
		}

		// Check for exclude filter marker '!'
		bool isExclude = false;
		if( s[pos] == '!' )
		{
			// We have an exclude filter, advance the position by one and skip whitespace(s)
			isExclude = true;
			++pos;
			while( pos < s.size() && std::isspace( static_cast<unsigned char>( s[pos] ) ) )
			{
				++pos;
			}
			if( pos >= s.size() )
			{
				throw std::invalid_argument( "Invalid filter format: exclude filter marker found without a [ token ] section" );
			}
		}

		if( pos >= s.size() || s[pos] != '[' )
		{
			throw std::invalid_argument( "Invalid filter format: missing '['" );
		}
		++pos; // skip '['

		size_t endBracket = s.find( ']', pos );
		size_t nextStartBracket = s.find( '[', pos );
		if( nextStartBracket != std::string::npos && nextStartBracket < endBracket )
		{
			throw std::invalid_argument( "Invalid filter format: matching end bracket ']' not present before the next start bracket '['" );
		}

		if( endBracket == std::string::npos )
		{
			throw std::invalid_argument( "Invalid filter format: missing ']'" );
		}

		std::string entries = s.substr( pos, endBracket - pos );
		std::istringstream iss( entries );
		std::string token;
		while( iss >> token )
		{
			// Trim whitespace from token
			size_t start = token.find_first_not_of( " \t\r\n" );
			size_t end = token.find_last_not_of( " \t\r\n" );
			if( start == std::string::npos || end == std::string::npos )
			{
				continue;
			}
			token = token.substr( start, end - start + 1 );

			if( token.empty() )
			{
				continue;
			}

			PlaceTokenInCorrectVector( token,
									   isExclude ? m_includeFilter : m_excludeFilter,
									   isExclude ? m_excludeFilter : m_includeFilter );
		}
		pos = endBracket + 1;
	}

	// Make sure that we have a wild-card ("*") in the TOP-LEVEL include filter if the include filter is empty
	if( m_isToplevelFilter && m_includeFilter.empty() )
	{
		m_includeFilter.push_back( "*" );

		// Also make sure we add the wild-card include to the raw filter (in case filters are concatenated later)
		if( !m_rawFilter.empty() )
		{
			m_rawFilter += " ";
		}
		m_rawFilter += "[ * ]";
	}
}

// -------------------------------------------------------------
// Description:
//   Static helper function placing filter tokens in the correct include/exclude vector.
// Arguments:
//   token - the filter token to place in the correct vector (e.g: ".yaml")
//   fromVector - the vector to remove the token from (if present)
//   toVector - the vector to add the token to (if not already present in it)
// -------------------------------------------------------------
void FilterResourceFilter::PlaceTokenInCorrectVector( const std::string& token, std::vector<std::string>& fromVector, std::vector<std::string>& toVector )
{
	// Remove token from the fromVector if present
	auto it = std::find( fromVector.begin(), fromVector.end(), token );
	if( it != fromVector.end() )
	{
		fromVector.erase( it );
	}

	// Add token to the toVector if not already present in it.
	if( std::find( toVector.begin(), toVector.end(), token ) == toVector.end() )
	{
		toVector.push_back( token );
	}
}

}
