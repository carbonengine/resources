// Copyright © 2025 CCP ehf.

#include <stdexcept>
#include <FilterDefaultSection.h>

namespace ResourceTools
{

FilterDefaultSection::FilterDefaultSection( const std::string& prefixmapStr ) :
	m_prefixMap( prefixmapStr )
{
	throw std::logic_error( "Not implemented yet exception" );
}

const FilterPrefixMap& FilterDefaultSection::GetPrefixmap() const
{
	throw std::logic_error( "Not implemented yet exception" );
}

}
