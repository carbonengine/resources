// Copyright © 2025 CCP ehf.

#ifndef FILTERRESOURCEFILTER_H
#define FILTERRESOURCEFILTER_H

#include <string>
#include <vector>

namespace ResourceTools
{

// Class representing a resource filter with include and exclude filters.
// - This can be for the filter attribute of a NamedSection OR the optional filter for a respaths/resfile line (defined in FilterResourceLine).
// - see ResourceTools::FilterNamedSection and ResourceTools::FilterResourceLine
class FilterResourceFilter
{
public:
	// Constructor that takes a raw filter string and parses it into include and exclude filters.
	explicit FilterResourceFilter( const std::string& rawFilter );

	// Gets the raw filter string.
	const std::string& GetRawFilter() const;

	// Gets the include filter vector.
	const std::vector<std::string>& GetIncludeFilter() const;

	// Gets the exclude filter vector.
	const std::vector<std::string>& GetExcludeFilter() const;

private:
	// Raw filter string.
	std::string m_rawFilter;

	// Include filter vector.
	std::vector<std::string> m_includeFilter;

	// Exclude filter vector.
	std::vector<std::string> m_excludeFilter;

	// Parse raw filter string into vectors of include and exclude filters.
	void ParseFilters();

	// Static helper to place a token in the correct vector, moving it from one vector to another, if need be.
	static void PlaceTokenInCorrectVector( const std::string& token, std::vector<std::string>& fromVector, std::vector<std::string>& toVector );
};

}

#endif // FILTERRESOURCEFILTER_H
