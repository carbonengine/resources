// Copyright © 2025 CCP ehf.

#ifndef FILTERRESOURCELINE_H
#define FILTERRESOURCELINE_H

#include <string>
#include "FilterPrefixmap.h"
#include "FilterResourceFilter.h"

namespace ResourceTools
{

/// @brief Class representing a single line of a resfile/respaths attribute.
class FilterResourceLine
{
public:
	/// @brief Constructor that takes a rawLine string, a reference to an already constructed prefixMap and sectionFilter.
	/// @param rawLine the raw value of a resfile/respaths line string to parse.
	/// @param prefixMap reference to an already constructed FilterPrefixmap object.
	/// @param sectionFilter reference to an already constructed FilterResourceFilter object representing the "parent" filter for this section.
	/// @note Calls ParseLine() which may throw std::invalid_argument if the rawLine string is malformed.
	/// @see CarbonResources::FilterResourceLine::ParseLine
	explicit FilterResourceLine( const std::string& rawLine, const FilterPrefixmap& prefixMap, const FilterResourceFilter& sectionFilter );

	// bool IsValid() const;  // TODO: Remove this, probably don't need it.

	// TODO: Add a getter function that combines the section filter and optional line filter along with the concatenated prefixMap + respath/resfile value.

private:
	/// @brief The raw string, representing the value of the resfile/respaths attribute.
	/// @note Set in the constructor and used in ParseLine().
	std::string m_rawLine;

	/// @brief Reference to an already constructed FilterPrefixmap object.
	const FilterPrefixmap& m_prefixMap;

	/// @brief Reference to an already constructed FilterResourceFilter object representing the "parent" filter for this section.
	const FilterResourceFilter& m_sectionFilter;

	// TODO: Decide how to handle optional FilterResourceFilter for the line-specific filter, if any
	//       Probably add: std::optional<FilterResourceFilter> m_lineFilter
	//       Then add a getter function (probably private) that combines the section filter and line filter as needed.

	/// @brief The resolved full path after applying the prefix map to the file/path portion of the rawLine.
	std::string m_linePath;

	/// @brief Parses the rawLine string into its components (m_linePath, optional m_lineFilter).
	/// @note Throws std::invalid_argument if parsing fails, bubbling the error up to the caller (class constructor).
	/// @return void
	void ParseLine();
};

}

#endif // FILTERRESOURCELINE_H
