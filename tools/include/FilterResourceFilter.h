// Copyright © 2025 CCP ehf.

#ifndef FILTERRESOURCEFILTER_H
#define FILTERRESOURCEFILTER_H

#include <string>
#include <vector>

namespace ResourceTools
{

/// @brief Class representing a resource filter with include and exclude filters.
/// @note This can be for the filter attribute of a NamedSection OR the optional filter for a respaths/resfile line (defined in FilterResourceLine).
/// see CarbonResources::FilterNamedSection and CarbonResources::FilterResourceLine for usage.
class FilterResourceFilter
{
public:
	/// @brief Constructor that takes a raw filter string and parses it into include and exclude filters.
	/// @param rawFilter the raw filter string to parse.
	/// @note Calls ParseFilters() which may throw std::invalid_argument if the filter string is malformed.
	/// @see CarbonResources::FilterResourceFilter::ParseFilters
	explicit FilterResourceFilter( const std::string& rawFilter );

	/// @brief Gets the raw filter string.
	/// @return the raw filter string.
	const std::string& GetRawFilter() const;

	/// @brief Gets the include filter vector.
	/// @return the valid include filter as a vector.
	const std::vector<std::string>& GetIncludeFilter() const;

	/// @brief Gets the exclude filter vector.
	/// @return the valid exclude filter as a vector.
	const std::vector<std::string>& GetExcludeFilter() const;

private:
	/// @brief The raw filter string.
	/// @note Set in the constructor and used in ParseFilters().
	std::string m_rawFilter;

	/// @brief The include filter vector.
	/// @note Populated in ParseFilters().
	/// @see CarbonResources::FilterResourceFilter::ParseFilters
	std::vector<std::string> m_includeFilter;

	/// @brief The exclude filter vector.
	/// @note Populated in ParseFilters().
	/// @see CarbonResources::FilterResourceFilter::ParseFilters
	std::vector<std::string> m_excludeFilter;

	/// @brief Parses the raw filter string into include and exclude filters.
	/// @return void
	/// @note Throws std::invalid_argument if the filter string is malformed, bubbling the error up to the caller (class constructor).
	void ParseFilters();

	/// @brief Static helper function that places a token in the correct vector, moving it from one vector to another if need be.
	/// @param token the token to place in the correct vector.
	/// @param fromVector the vector to remove the token from if it exists there.
	/// @param toVector the vector to add the token to if it does not already exist there.
	/// @see CarbonResources::FilterResourceFilter::ParseFilters for usage.
	/// @return void
	static void PlaceTokenInCorrectVector( const std::string& token, std::vector<std::string>& fromVector, std::vector<std::string>& toVector );
};

}

#endif // FILTERRESOURCEFILTER_H
