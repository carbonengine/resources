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
};

}

#endif // FILTERRESOURCEFILTER_H
