// Copyright © 2025 CCP ehf.

#include <stdexcept>
#include "FilterResourceLine.h"

namespace CarbonResources
{

FilterResourceLine::FilterResourceLine( const std::string& line, const FilterPrefixmap& prefixMap, const FilterResourceFilter& sectionFilter ) :
	m_line( line ),
	m_prefixMap( prefixMap ),
	m_resFilter( sectionFilter )
{
	throw std::logic_error( "Not implemented yet exception" );
}

bool FilterResourceLine::IsValid() const
{
	throw std::logic_error( "Not implemented yet exception" );
}

}
