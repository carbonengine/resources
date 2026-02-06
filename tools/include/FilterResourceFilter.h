// Copyright © 2025 CCP ehf.

#pragma once
#ifndef FilterResourceFilter_H
#define FilterResourceFilter_H

#include <string>
#include <vector>

namespace ResourceTools
{

// -------------------------------------------------------------
// Description:
//   FilterResourceFilter is a class that represents the parsed
//   include and exclude filters for either a topLevel or inLine filter.
// -------------------------------------------------------------
class FilterResourceFilter
{
public:
	FilterResourceFilter() = default;

	// Construct a FilterResourceFilter object by parsing the given raw filter string.
	// isTopLevelFilter:
	//   - true  = filter is from the "filter" attribute of a [NamedSection]
	//   - false = filter is an inline filter of a respaths/resfile line entry.
	explicit FilterResourceFilter( const std::string& rawFilter, bool isToplevelFilter = false );

	// Getters for the raw filter string.
	// Needed to construct combined filters for respaths/resfile attribute line entries.
	const std::string& GetRawFilter() const;

	// Getters for the parsed include filter vector.
	const std::vector<std::string>& GetIncludeFilter() const;

	// Getters for the parsed exclude filter vector.
	const std::vector<std::string>& GetExcludeFilter() const;

private:
	void ParseFilters();

	// Static helper function placing filter tokens in the correct include/exclude vector.
	static void PlaceTokenInCorrectVector( const std::string& token, std::vector<std::string>& fromVector, std::vector<std::string>& toVector );

	// Indicates whether this FilterResourceFilter is a top-level filter,
	// i.e. from the "filter" attribute (true) of a [NamedSection]
	// or an inline filter (false) i.e. from a respaths/resfile line "prefix1:pathA [ newInclude ]".
	bool m_isToplevelFilter = false;

	// The raw filter string as read from the .ini file (e.g: "[ .yaml .txt ] ![ .exe ]").
	// Stored to enable easy concatenation of filters when constructing combined resolved filters.
	std::string m_rawFilter;

	// Vector of the parsed include filter tokens, e.g: { ".yaml", ".txt" }.
	std::vector<std::string> m_includeFilter;

	// Vector of the parsed exclude filter tokens, e.g: { ".exe" }.
	std::vector<std::string> m_excludeFilter;
};

}

#endif // FilterResourceFilter_H
