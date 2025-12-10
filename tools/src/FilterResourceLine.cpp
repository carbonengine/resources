// Copyright © 2025 CCP ehf.

#include <stdexcept>
#include <FilterResourceLine.h>

namespace ResourceTools
{

FilterResourceLine::FilterResourceLine( const std::string& rawLine, const FilterPrefixmap& prefixMap, const FilterResourceFilter& sectionFilter ) :
	m_rawLine( rawLine ),
	m_prefixMap( prefixMap ),
	m_sectionFilter( sectionFilter )
{
	throw std::logic_error( "Not implemented yet exception" );
}

// TODO: Remove this, probably don't need it.
//bool FilterResourceLine::IsValid() const
//{
//	throw std::logic_error( "Not implemented yet exception" );
//}

void FilterResourceLine::ParseLine()
{
	throw std::logic_error( "Not implemented yet exception" );
}

}
