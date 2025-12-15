// Copyright © 2025 CCP ehf.

#ifndef FILTERDEFAULTSECTION_H
#define FILTERDEFAULTSECTION_H

#include <FilterPrefixmap.h>
#include <FilterPrefixMapEntry.h>

namespace ResourceTools
{

class FilterDefaultSection
{
public:
	explicit FilterDefaultSection( const std::string& prefixmapStr );

	const std::map<std::string, FilterPrefixMapEntry>& GetPrefixMap() const;

private:
	FilterPrefixMap m_prefixMap;
};

}

#endif // FILTERDEFAULTSECTION_H
