// Copyright © 2025 CCP ehf.

#ifndef FILTERDEFAULTSECTION_H
#define FILTERDEFAULTSECTION_H

#include "FilterPrefixmap.h"

namespace ResourceTools
{

class FilterDefaultSection
{
public:
	explicit FilterDefaultSection( const std::string& prefixmapStr );

	const FilterPrefixMap& GetPrefixmap() const;

private:
	FilterPrefixMap m_prefixMap;
};

}

#endif // FILTERDEFAULTSECTION_H
