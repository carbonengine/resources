// Copyright © 2025 CCP ehf.

#include <stdexcept>
#include <FilterDefaultSection.h>

namespace CarbonResources
{

FilterDefaultSection::FilterDefaultSection( const std::string& prefixmapStr ) :
	m_prefixmap( prefixmapStr )
{
	throw std::logic_error( "Not implemented yet exception" );
}

const FilterPrefixmap& FilterDefaultSection::GetPrefixmap() const
{
	throw std::logic_error( "Not implemented yet exception" );
}

}
