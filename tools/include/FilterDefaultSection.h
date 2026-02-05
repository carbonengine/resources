// Copyright © 2025 CCP ehf.

#pragma once
#ifndef FilterDefaultSection_H
#define FilterDefaultSection_H

#include "FilterPrefixmap.h"

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

#endif // FilterDefaultSection_H
