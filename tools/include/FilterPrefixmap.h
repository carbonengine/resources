// Copyright © 2025 CCP ehf.

#ifndef FILTERPREFIXMAP_H
#define FILTERPREFIXMAP_H

#include <string>
#include <map>
#include <vector>
#include <FilterPrefixMapEntry.h>

namespace ResourceTools
{

// Class representing the prefixmap attribute.
class FilterPrefixMap
{
public:
	FilterPrefixMap() = default;

	explicit FilterPrefixMap( const std::string& rawPrefixMap );

	const std::map<std::string, FilterPrefixMapEntry>& GetMapEntries() const;

private:
	// Map of prefixes to FilterPrefixMapEntry objects.
	std::map<std::string, FilterPrefixMapEntry> m_prefixMapEntries;

	void ParsePrefixMap( const std::string& rawPrefixMap );
};

}

#endif // FILTERPREFIXMAP_H
