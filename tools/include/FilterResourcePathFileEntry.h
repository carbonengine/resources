// Copyright © 2025 CCP ehf.

#ifndef FILTERRESOURCEPATHFILEENTRY_H
#define FILTERRESOURCEPATHFILEENTRY_H

#include <set>
#include <FilterPrefixmap.h>
#include <FilterResourceFilter.h>

namespace ResourceTools
{

class FilterResourcePathFileEntry
{
public:
	explicit FilterResourcePathFileEntry( std::string rawPathLine,
										  const FilterPrefixMap& parentPrefixMap,
										  const FilterResourceFilter& parentSectionFilter );

	const FilterResourceFilter& GetEntryFilter() const;

	const std::set<std::string>& GetResolvedPaths() const;

private:
	std::string m_rawPathLine;

	const FilterPrefixMap& m_parentPrefixMap; // The "parent" prefix map from the [DEFAULT] section

	FilterResourceFilter m_parentSectionFilter;

	// The combined filter for this resfile/respaths attribute line, built from the parentSectionFilter and any inline filter.
	FilterResourceFilter m_entryFilter;

	// The set of resolved paths (sorted).
	std::set<std::string> m_resolvedPaths;

	// Parse the m_rawPathLine by constructing the combined m_entryFilter and append paths to m_resolvedPaths based on the m_parentPrefixMap
	void ParseRawPathLine();
};

}

#endif //FILTERRESOURCEPATHFILEENTRY_H
