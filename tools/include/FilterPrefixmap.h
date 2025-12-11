// Copyright © 2025 CCP ehf.

#ifndef FILTERPREFIXMAP_H
#define FILTERPREFIXMAP_H

#include <string>
#include <map>
#include <vector>

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
	const std::map<std::string, std::vector<std::string>>& GetPrefixMap() const;

private:
	// Raw filter string.
	std::string m_rawPrefixMap;

	// Map of prefixes to list of paths.
	// e.g. "res:" -> { "/dir1", "/dir2" } and "res2:" -> { "/otherDir1" }
	std::map<std::string, std::vector<std::string>> m_prefixMap;

	void ParsePrefixMap();
};

}

#endif // FILTERPREFIXMAP_H
