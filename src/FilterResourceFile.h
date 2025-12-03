// Copyright © 2025 CCP ehf.

#ifndef FILTERRESOURCEFILE_H
#define FILTERRESOURCEFILE_H

#include <vector>
#include "FilterDefaultSection.h"
#include "FilterNamedSection.h"

namespace CarbonResources
{

class FilterResourceFile
{
public:
	explicit FilterResourceFile( const FilterDefaultSection& defaultSection, const std::vector<FilterNamedSection>& namedSections );

private:
	FilterDefaultSection m_defaultSection;
	std::vector<FilterNamedSection> m_namedSections;
};

}

#endif // FILTERRESOURCEFILE_H
