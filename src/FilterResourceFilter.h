// Copyright © 2025 CCP ehf.

#ifndef FILTERRESOURCEFILTER_H
#define FILTERRESOURCEFILTER_H

#include <string>
#include <vector>

namespace CarbonResources
{

class FilterResourceFilter
{
public:
	explicit FilterResourceFilter( const std::string& rawFilter );

	const std::string& GetRawFilter() const;

	const std::vector<std::string>& GetIncludeFilter() const;

	const std::vector<std::string>& GetExcludeFilter() const;

private:
	std::string m_rawFilter;
	std::vector<std::string> m_includeFilter;
	std::vector<std::string> m_excludeFilter;

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
