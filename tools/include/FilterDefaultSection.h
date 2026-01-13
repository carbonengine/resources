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
	FilterDefaultSection() = default;

	explicit FilterDefaultSection( const std::string& prefixmapStr );

	const FilterPrefixMap& GetPrefixMap() const;

private:
	FilterPrefixMap m_prefixMap;
};

}

#endif // FILTERDEFAULTSECTION_H
