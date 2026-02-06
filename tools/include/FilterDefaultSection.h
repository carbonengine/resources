// Copyright © 2025 CCP ehf.

#pragma once
#ifndef FilterDefaultSection_H
#define FilterDefaultSection_H

#include "FilterPrefixmap.h"

namespace ResourceTools
{

// -------------------------------------------------------------
// Description:
//   FilterDefaultSection is a class that represents the [DEFAULT]
//   section attribute of a .ini filter file.
// -------------------------------------------------------------
class FilterDefaultSection
{
public:
	FilterDefaultSection() = default;

	// Constructs a FilterDefaultSection object
	explicit FilterDefaultSection(const std::string& rawPrefixMap)
		: m_prefixMap(rawPrefixMap)
	{
	}

	// Gets the prefix map of the [DEFAULT] section
	const FilterPrefixMap& GetPrefixMap() const
	{
		return m_prefixMap;
	}

private:
	// Map of prefixes to FilterPrefixMapEntry objects, from the parsed prefixmap attribute.
	FilterPrefixMap m_prefixMap;
};

}

#endif // FilterDefaultSection_H
