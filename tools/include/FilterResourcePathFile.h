// Copyright © 2025 CCP ehf.

#pragma once
#ifndef FilterResourcePathFile_H
#define FilterResourcePathFile_H

#include <string>

#include "FilterPrefixmap.h"
#include "FilterResourceFilter.h"

namespace ResourceTools
{

// -------------------------------------------------------------
// Description:
//   FilterResourcePathFile is a class that represents a resfile/respaths
//   attribute from a filter .ini file.
//   Each "respaths" attribute may contain multiple line entries,
//   stored in the FilterResourcePathFileEntry class.
// -------------------------------------------------------------
class FilterResourcePathFile
{
public:
	// Construct a FilterResourcePathFile object for a resfile/respaths attribute.
	explicit FilterResourcePathFile( const std::string& rawPathFileAttrib,
									 const FilterPrefixMap& parentPrefixMap,
									 const FilterResourceFilter& parentSectionFilter );

	// Gets a map of fully resolved relative paths and the associated FilterResourceFilter include/exclude filters.
	const std::map<std::string, FilterResourceFilter>& GetResolvedPathMap() const;

private:
	// Parse the rawPathFileAttrib and populate the m_resolvedPathMap.
	void ParseRawPathFileAttribute( const std::string& rawPathFileAttrib );

	// The "parent" prefix map from the [DEFAULT] section
	const FilterPrefixMap& m_parentPrefixMap;

	// The "parent" filter from the [namedSection] containing this resfile/respaths attribute
	const FilterResourceFilter& m_parentSectionFilter;

	// Map of fully resolved paths to their combined FilterResourceFilter objects.
	std::map<std::string, FilterResourceFilter> m_resolvedPathMap;
};

}

#endif // FilterResourcePathFile_H
