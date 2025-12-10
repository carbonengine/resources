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

	const FilterPrefixmap& GetPrefixmap() const;

private:
	FilterPrefixmap m_prefixmap;
};

}

#endif // FILTERDEFAULTSECTION_H
