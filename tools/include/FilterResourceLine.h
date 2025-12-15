// Copyright © 2025 CCP ehf.

#ifndef FILTERRESOURCELINE_H
#define FILTERRESOURCELINE_H

#include <string>
#include <optional>
#include <FilterPrefixmap.h>
#include <FilterResourceFilter.h>

namespace ResourceTools
{

// Class representing a single raw line of a resfile/respaths attribute.
class FilterResourceLine
{
public:
	// Constructor that takes a rawLine string to parse into a linePath vector, based on already constructed prefixMap and an optional sectionFilter.
	// - sectionFilter is only optional, in case "resfile" attribute requires it
	explicit FilterResourceLine( const std::string& rawLine, const FilterPrefixMap& prefixMap, std::optional<FilterResourceFilter> sectionFilter );

	// TODO: Add a getter function that combines the section filter and optional line filter along with the concatenated prefixMap + respath/resfile value.

private:
	// Raw string, representing the value of the resfile/respaths line attribute.
	std::string m_rawLine;

	// Reference to an already constructed FilterPrefixMap object.
	const FilterPrefixMap& m_prefixMap;

	// Optional FilterResourceFilter object representing the "parent" filter for this section.
	// - optional, in case this is for a "resfile" attribute that MAY NOT have a section filter.
	std::optional<FilterResourceFilter> m_sectionFilter;

	// The optional FilterResourceFilter object representing the line-specific filter, if any.
	const std::optional<FilterResourceFilter> m_lineFilter;

	// A vector of FilterResourceLineEntries, with two elemants: resolved full paths after applying all relevant prefix maps to the path portion, along with the combined in-order m_sectionFilter + optional m_lineFilter.
	// NOTE:  FilterResourceLineEntry is a class/struct with a string path and combined FilterResourceFilter)
	// Replace this std::vector<std::string> m_linePath;
	// with std::vector<FilterResourceLineEntry> m_lineEntry;

	// Parses the rawLine string into its components (m_linePath, optional m_lineFilter).
	void ParseLine();
};

}

#endif // FILTERRESOURCELINE_H
