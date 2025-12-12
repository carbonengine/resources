// Copyright © 2025 CCP ehf.

#ifndef FILTERPREFIXMAP_H
#define FILTERPREFIXMAP_H

#include <string>
#include <map>
#include <vector>
#include "FilterPrefixMapEntry.h"

namespace ResourceTools
{

// Class representing the prefixmap attribute.
// - used to resolve actual resfile/respaths attributes of a NamedSection.
class FilterPrefixMap
{
public:
	// Constructor that takes a raw filter string and parses it into a map of prefixes to list of paths.
	explicit FilterPrefixMap( const std::string& rawPrefixMap );

	// Gets the prefix map.
	const std::map<std::string, FilterPrefixMapEntry> GetPrefixMap() const;

private:
	// Map of prefixes to FilterPrefixMapEntry objects.
	std::map<std::string, FilterPrefixMapEntry> m_prefixMap;

	void ParsePrefixMap( const std::string& rawPrefixMap );
};

}

#endif // FILTERPREFIXMAP_H
