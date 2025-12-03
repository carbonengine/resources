// Copyright © 2025 CCP ehf.

#ifndef FILTERRESOURCELINE_H
#define FILTERRESOURCELINE_H

#include <string>
#include "FilterPrefixmap.h"
#include "FilterResourceFilter.h"

namespace CarbonResources
{

class FilterResourceLine
{
public:
	explicit FilterResourceLine( const std::string& line, const FilterPrefixmap& prefixMap, const FilterResourceFilter& sectionFilter );

	bool IsValid() const;

private:
	std::string m_line;
	const FilterPrefixmap& m_prefixMap;
	FilterResourceFilter m_resFilter;
};

}

#endif // FILTERRESOURCELINE_H
