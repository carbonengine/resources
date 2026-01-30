// Copyright © 2025 CCP ehf.

#include <sstream>
#include <cctype>
#include <stdexcept>
#include <FilterResourceFilter.h>

namespace ResourceTools
{

FilterResourceFilter::FilterResourceFilter( const std::string& rawFilter, bool isToplevelFilter /* = false */ ) :
	m_rawFilter( rawFilter ),
	m_isToplevelFilter( isToplevelFilter )
{
	ParseFilters();
}

const std::string& FilterResourceFilter::GetRawFilter() const
{
	return m_rawFilter;
}

const std::vector<std::string>& FilterResourceFilter::GetIncludeFilter() const
{
	return m_includeFilter;
}

const std::vector<std::string>& FilterResourceFilter::GetExcludeFilter() const
{
	return m_excludeFilter;
}

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

void FilterResourceFilter::ParseFilters()
{
	m_includeFilter.clear();
	m_excludeFilter.clear();

	std::string s = m_rawFilter;
	size_t pos = 0;
	while( pos < s.size() )
	{
		// Skip whitespace
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

}
