// Copyright © 2025 CCP ehf.

#pragma once
#ifndef FilterResourcePathFileEntry_H
#define FilterResourcePathFileEntry_H

#include <set>
#include <string>

#include "FilterPrefixmap.h"
#include "FilterResourceFilter.h"

namespace ResourceTools
{

// -------------------------------------------------------------
// Description:
//   FilterResourcePathFileEntry is a class that represents a single line entry
//   from a resfile/respaths attribute in a filter .ini file.
// Note:
//   The raw representation of it from a filter .ini file can be:
//    - "prefix:/pathPart/..."  (sub-folder wildcard, without an inline filter)
//    - "prefix:/pathPart/*  [ .txt ] ![ .yaml] " (current folder wildcard, with an optional inline include and exclude filter)
// -------------------------------------------------------------
class FilterResourcePathFileEntry
{
public:
	// Construct a FilterResourcePathFileEntry object for a single resfile/respaths attribute line entry.
	explicit FilterResourcePathFileEntry( const std::string& rawPathLine,
										  const FilterPrefixMap& parentPrefixMap,
										  const FilterResourceFilter& parentSectionFilter );

	// Gets the (possibly combined) filter for this resfile/respaths attribute line entry.
	const FilterResourceFilter& GetEntryFilter() const;

	// Gets the resolved path set for this resfile/respaths attribute line entry.
	const std::set<std::string>& GetResolvedPaths() const;

private:
	// The "parent" prefix map from the [DEFAULT] section
	const FilterPrefixMap& m_parentPrefixMap;

	// The "parent" filter from the [namedSection]
	const FilterResourceFilter& m_parentSectionFilter;

	// The combined filter for this resfile/respaths attribute line entry.
	FilterResourceFilter m_entryFilter;

	// The set of resolved paths (sorted).
	std::set<std::string> m_resolvedPaths;

	// Parse the rawPathLine and constructing the m_entryFilter and m_resolvedPaths
	void ParseRawPathLine( const std::string& rawPathLine );
};

}

#endif //FilterResourcePathFileEntry_H
