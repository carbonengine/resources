// Copyright © 2025 CCP ehf.

#include <stdexcept>
#include <FilterNamedSection.h>

namespace CarbonResources
{

FilterNamedSection::FilterNamedSection( const std::string& filter, const std::string& resfile, const std::vector<std::string>& respaths ) :
	m_filter( filter ),
	m_resfile( resfile ),
	m_respaths( respaths )
{
	throw std::logic_error( "Not implemented yet exception" );
}

}
