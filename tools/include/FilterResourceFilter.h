// Copyright © 2025 CCP ehf.

#ifndef FILTERRESOURCEFILTER_H
#define FILTERRESOURCEFILTER_H

#include <string>
#include <vector>

namespace ResourceTools
{

// Class representing a resource filter with include and exclude filters.
// - This is for filter attribute of a NamedSection AND the "combined resolved" filter for each respaths/resfile line
class FilterResourceFilter
{
public:
	FilterResourceFilter() = default;

	explicit FilterResourceFilter( const std::string& rawFilter );

	// Used as input when constructing a combined resolved filter for a respaths/resfile line.
	const std::string& GetRawFilter() const;

	const std::vector<std::string>& GetIncludeFilter() const;

	const std::vector<std::string>& GetExcludeFilter() const;

private:
	std::string m_rawFilter;

	std::vector<std::string> m_includeFilter;

	std::vector<std::string> m_excludeFilter;

	void ParseFilters();

	// Static helper placing tokens in the correct vector, moving it if need be.
	static void PlaceTokenInCorrectVector( const std::string& token, std::vector<std::string>& fromVector, std::vector<std::string>& toVector );
};

}

#endif // FILTERRESOURCEFILTER_H
