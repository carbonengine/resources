// Copyright © 2025 CCP ehf.

#include <stdexcept>
#include <FilterDefaultSection.h>

namespace ResourceTools
{

FilterDefaultSection::FilterDefaultSection( const std::string& prefixmapStr ) :
	m_prefixMap( prefixmapStr )
{
}

const std::map<std::string, FilterPrefixMapEntry>& FilterDefaultSection::GetPrefixMap() const
{
	return m_prefixMap.GetPrefixMap();
}


}
