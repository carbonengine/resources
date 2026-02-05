// Copyright © 2025 CCP ehf.

#include "FilterDefaultSection.h"

namespace ResourceTools
{

FilterDefaultSection::FilterDefaultSection( const std::string& prefixmapStr ) :
	m_prefixMap( prefixmapStr )
{
}

const FilterPrefixMap& FilterDefaultSection::GetPrefixMap() const
{
	return m_prefixMap;
}


}
