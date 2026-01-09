// Copyright © 2025 CCP ehf.

#ifndef FILTERRESOURCEPATHFILE_H
#define FILTERRESOURCEPATHFILE_H

#include <string>
#include <FilterPrefixmap.h>
#include <FilterResourceFilter.h>
#include <FilterResourcePathFileEntry.h>

namespace ResourceTools
{

// Class representing a resfile/respaths attribute.
class FilterResourcePathFile
{
public:
	FilterResourcePathFile() = default;

	explicit FilterResourcePathFile( std::string rawPathFileAttrib,
									 const FilterPrefixMap& parentPrefixMap,
									 const FilterResourceFilter& parentSectionFilter );

	// Get the map of fully resolved paths to their combined FilterResourceFilter objects.
	const std::map<std::string, FilterResourceFilter>& GetResolvedPathMap() const;

private:
	// The raw (multiline) respath attribute (same for resfile).
	std::string m_rawPathFileAttrib;

	const FilterPrefixMap& m_parentPrefixMap; // The "parent" prefix map from the [DEFAULT] section

	FilterResourceFilter m_parentSectionFilter;

	// Map of fully resolved paths to their combined FilterResourceFilter objects.
	std::map<std::string, FilterResourceFilter> m_resolvedPathMap;

	void ParseRawPathFileAttribute();
};

}

#endif // FILTERRESOURCEPATHFILE_H
