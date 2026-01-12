// Copyright © 2025 CCP ehf.

#ifndef FILTERNAMEDSECTION_H
#define FILTERNAMEDSECTION_H

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <FilterPrefixMap.h>
#include <FilterResourceFilter.h>
#include <FilterResourcePathFile.h>

namespace ResourceTools
{

class FilterNamedSection
{
public:
	explicit FilterNamedSection( std::string sectionName,
								 const std::string& filter,
								 const std::string& respaths,
								 const std::string& resfile,
								 const FilterPrefixMap& parentPrefixMap );

	const std::string& GetSectionName() const;

	// Return combined resolved path map from both respaths and optional resfile
	const std::map<std::string, FilterResourceFilter>& GetCombinedResolvedPathMap();

	const std::map<std::string, FilterResourceFilter>& GetResolvedRespathsMap() const;

	const std::map<std::string, FilterResourceFilter>& GetResolvedResfileMap() const;

private:
	std::string m_sectionName;

	const FilterPrefixMap& m_parentPrefixMap; // The "parent" prefix map from the [DEFAULT] section

	FilterResourceFilter m_filter;

	FilterResourcePathFile m_respaths;

	std::optional<FilterResourcePathFile> m_resfile;

	// Combined map of fully resolved respaths and resfile FilterResourceFilter objects
	std::map<std::string, FilterResourceFilter> m_resolvedCombinedPathMap;
};

}

#endif // FILTERNAMEDSECTION_H
