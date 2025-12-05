// Copyright © 2025 CCP ehf.

//#include <stdexcept>
//#include <regex>
#include <sstream>
#include <cctype>
#include "FilterResourceFilter.h"

namespace CarbonResources
{

FilterResourceFilter::FilterResourceFilter( const std::string& rawFilter )
	: m_rawFilter( rawFilter )
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

void FilterResourceFilter::ParseFilters()
{
	m_includeFilter.clear();
	m_excludeFilter.clear();

	std::string s = m_rawFilter;
	size_t pos = 0;
	while( pos < s.size() )
	{
		// Skip whitespace
		while( pos < s.size() && std::isspace(static_cast<unsigned char>(s[pos])) )
			++pos;
		if( pos >= s.size() )
			break;

		bool isExclude = false;
		if( s[pos] == '!' )
		{
			isExclude = true;
			++pos;
		}

		if( pos >= s.size() || s[pos] != '[' )
			throw std::invalid_argument( "Invalid filter format: missing '['" );

		++pos; // skip '['
		size_t endBracket = s.find( ']', pos );
		if( endBracket == std::string::npos )
			throw std::invalid_argument( "Invalid filter format: missing ']'" );

		std::string entries = s.substr( pos, endBracket - pos );
		std::istringstream iss( entries );
		std::string token;
		while( iss >> token )
		{
			// Trim whitespace from token
			size_t start = token.find_first_not_of( " \t\r\n" );
			size_t end = token.find_last_not_of( " \t\r\n" );
			if( start == std::string::npos || end == std::string::npos )
				continue;
			token = token.substr( start, end - start + 1 );

			if( token.empty() )
				continue;

			if( isExclude )
			{
				// Remove from include if present
				auto it = std::find( m_includeFilter.begin(), m_includeFilter.end(), token );
				if( it != m_includeFilter.end() )
					m_includeFilter.erase( it );
				// Add to exclude if not present
				if( std::find( m_excludeFilter.begin(), m_excludeFilter.end(), token ) == m_excludeFilter.end() )
					m_excludeFilter.push_back( token );
			}
			else
			{
				// Remove from exclude if present
				auto it = std::find( m_excludeFilter.begin(), m_excludeFilter.end(), token );
				if( it != m_excludeFilter.end() )
					m_excludeFilter.erase( it );
				// Add to include if not present
				if( std::find( m_includeFilter.begin(), m_includeFilter.end(), token ) == m_includeFilter.end() )
					m_includeFilter.push_back( token );
			}
		}
		pos = endBracket + 1;
	}
}

}
