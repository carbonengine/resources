// Copyright © 2025 CCP ehf.

#pragma once
#ifndef FilterNamedSection_H
#define FilterNamedSection_H

#include <map>
#include <optional>
#include <string>

#include "FilterPrefixmap.h"
#include "FilterResourceFilter.h"
#include "FilterResourcePathFile.h"

namespace ResourceTools
{

// -------------------------------------------------------------
// Description:
//   FilterNamedSection is a class that represents all the contents
//   of a named section (e.g: [SomeSectionName]) from a filter .ini file.
// -------------------------------------------------------------
class FilterNamedSection
{
public:
	// Constructs a FilterNamedSection object for the given section.
	// Using the raw string "filter", "respaths", optional "resfile", and
	// parent "prefixmap" (from [DEFAULT] section) attributes as inputs.
	explicit FilterNamedSection( const std::string& sectionName,
								 const std::string& rawFilter,
								 const std::string& rawRespaths,
								 const std::string& rawResfile,
								 const FilterPrefixMap& parentPrefixMap );

	// Return the name of this section (e.g: [SomeSectionName]) from the .ini file.
	const std::string& GetSectionName() const;

	// Return the combined resolved path map from both the "respaths" and optional "resfile" attributes.
	// This is the main function to use to get the final resolved paths and their associated filters for this section.
	const std::map<std::string, FilterResourceFilter>& GetCombinedResolvedPathMap();

	// Return the resolved path map from the "respaths" attribute. Only used in tests to verify correctness of data.
	const std::map<std::string, FilterResourceFilter>& GetResolvedRespathsMap() const;

	// Return the resolved path map from the optional "resfile" attribute. Only used in tests to verify correctness of data.
	const std::map<std::string, FilterResourceFilter>& GetResolvedResfileMap() const;

private:
	// The name of this section (e.g: [SomeSectionName]) from the .ini file.
	std::string m_sectionName;

	// The parsed "filter" attribute for this named section.
	FilterResourceFilter m_filter;

	// The parsed "respaths" attribute.
	FilterResourcePathFile m_respaths;

	// The optional parsed "resfile" attribute.
	std::optional<FilterResourcePathFile> m_resfile;

	// Combined map of fully resolved "respaths" and "resfile" FilterResourceFilter objects
	std::map<std::string, FilterResourceFilter> m_resolvedCombinedPathMap;
};

}

#endif // FilterNamedSection_H
