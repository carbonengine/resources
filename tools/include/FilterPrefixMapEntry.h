// Copyright © 2025 CCP ehf.

#pragma once
#ifndef FilterPrefixMapEntry_H
#define FilterPrefixMapEntry_H

#include <set>
#include <string>

namespace ResourceTools
{

// -------------------------------------------------------------
// Description:
//   FilterPrefixMapEntry is a class that represents each
//   "prefix1:pathA;pathB" (or "prefix2:pathC") entry, separated by
//   a whitespace, from the "prefixmap" attribute.
//   Located as part of the [DEFAULT] section of a filter .ini file.
// -------------------------------------------------------------
class FilterPrefixMapEntry
{
public:
	// Constructs a FilterPrefixMapEntry object with the given prefix and raw paths (one or more ; separated).
	explicit FilterPrefixMapEntry( const std::string& prefix, const std::string& rawPaths );

	// Parses rawPaths and appends individual path entries to the m_paths set.
	void AppendPaths( const std::string& prefix, const std::string& rawPaths );

	// Gets the prefix identifier for this entry.
	const std::string& GetPrefix() const;

	// Gets the ordered set of parsed paths for this entry.
	const std::set<std::string>& GetPaths() const;

private:
	// The prefix identifier for this entry.
	std::string m_prefix;

	// Ordered set of parsed paths for this prefix.
	std::set<std::string> m_paths;
};

}

#endif // FilterPrefixMapEntry_H
