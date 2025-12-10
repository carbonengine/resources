// Copyright © 2025 CCP ehf.

#include <stdexcept>

#include <FilterResourceFile.h>

namespace ResourceTools
{

FilterResourceFile::FilterResourceFile( const FilterDefaultSection& defaultSection, const std::vector<FilterNamedSection>& namedSections ) :
	m_defaultSection( defaultSection ),
	m_namedSections( namedSections )
{
	throw std::logic_error( "Not implemented yet exception" );
}

}
