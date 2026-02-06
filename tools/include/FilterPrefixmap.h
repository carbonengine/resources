// Copyright © 2025 CCP ehf.

#pragma once
#ifndef FilterPrefixmap_H
#define FilterPrefixmap_H

#include <map>
#include <string>

#include "FilterPrefixMapEntry.h"

namespace ResourceTools
{

// -------------------------------------------------------------
// Description:
//   FilterPrefixMap is a class that represents the "prefixmap"
//   attribute of a [DEFAULT] section inside a filter .ini file.
// -------------------------------------------------------------
class FilterPrefixMap
{
public:
	FilterPrefixMap() = default;

	// Constructs a FilterPrefixMap object from a raw prefixmap string
	explicit FilterPrefixMap( const std::string& rawPrefixMap );

	// Gets the map of keyed prefixes to FilterPrefixMapEntry objects
	const std::map<std::string, FilterPrefixMapEntry>& GetMapEntries() const;

private:
	// Called from constructor to parse the raw prefixmap string and populate m_prefixMapEntries.
	void ParsePrefixMap( const std::string& rawPrefixMap );

	// Map of prefixes to FilterPrefixMapEntry objects.
	std::map<std::string, FilterPrefixMapEntry> m_prefixMapEntries;
};

}

#endif // FilterPrefixmap_H
