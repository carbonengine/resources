// Copyright © 2025 CCP ehf.

#include "FilterPrefixMapEntry.h"

namespace ResourceTools
{

FilterPrefixMapEntry::FilterPrefixMapEntry( const std::string& prefix, const std::string& rawPaths ) :
	m_prefix( prefix )
{
	m_paths.clear();

	AppendPaths( prefix, rawPaths );
}

void FilterPrefixMapEntry::AppendPaths( const std::string& prefix, const std::string& rawPaths )
{
	if( prefix != m_prefix )
		throw std::invalid_argument( "Prefix mismatch while appending: " + prefix + "(incoming) != " + m_prefix + "(existing)" );

	std::size_t pos = 0;
	while( pos < rawPaths.size() )
	{
		// Split the string up by semicolons (in case of multiple paths)
		std::size_t semicolon = rawPaths.find( ';', pos );
		std::string path = ( semicolon == std::string::npos ) ? rawPaths.substr( pos ) : rawPaths.substr( pos, semicolon - pos );
		if( !path.empty() )
			m_paths.insert( path );

		if( semicolon == std::string::npos )
			break;
		pos = semicolon + 1;
	}
	if( m_paths.empty() )
		throw std::invalid_argument( "Invalid prefixmap format: No paths associated with prefix: " + m_prefix );
}

const std::string& FilterPrefixMapEntry::GetPrefix() const
{
	return m_prefix;
}

const std::set<std::string>& FilterPrefixMapEntry::GetPaths() const
{
	return m_paths;
}

}
