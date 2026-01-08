// Copyright © 2025 CCP ehf.

#include <stdexcept>
#include <FilterNamedSection.h>

namespace ResourceTools
{

FilterNamedSection::FilterNamedSection( const std::string& sectionName, const std::string& filter, const std::string& respaths, const std::string& resfile, const FilterPrefixMap& parentPrefixMap ) :
	m_sectionName( sectionName ),
	m_rawFilter( filter ),
	m_rawRespaths( respaths ),
	m_rawResfile( resfile ),
	m_parentPrefixMap( parentPrefixMap )
{
	m_filter = FilterResourceFilter( m_rawFilter );

	ParseNamedSection();
}

const std::map<std::string, FilterResourceFilter>& FilterNamedSection::GetCombinedResolvedPathMap() const
{
	return m_resolvedCombinedPathMap;
}

const std::map<std::string, FilterResourceFilter>& FilterNamedSection::GetResolvedRespathsMap() const
{
	return m_respaths.GetResolvedPathMap();
}

const std::map<std::string, FilterResourceFilter>& FilterNamedSection::GetResolvedResfileMap() const
{
	if( m_resfile )
	{
		return m_resfile->GetResolvedPathMap();
	}
	else
	{
		static const std::map<std::string, FilterResourceFilter> emptyResfileMap;
		return emptyResfileMap;
	}
}

void FilterNamedSection::ParseNamedSection()
{
	// Respath
	if( m_rawRespaths.empty() )
	{
		throw std::invalid_argument( "Respaths attribute is empty for section: " + m_sectionName );
	}
	m_respaths = FilterResourcePathFile( m_rawRespaths, m_parentPrefixMap, m_filter );

	// Resfile
	if( !m_rawResfile.empty() )
	{
		m_resfile = FilterResourcePathFile( m_rawResfile, m_parentPrefixMap, m_filter );
	}
	else
	{
		m_resfile.reset();
	}

	// Populate the combined map.
	for( const auto& kv : m_respaths.GetResolvedPathMap() )
	{
		m_resolvedCombinedPathMap[kv.first] = kv.second;
	}

	// Add resfile to the combined map
	if( m_resfile )
	{
		// Allow "resfile" to contain multiple entries (future proofing)
		for( const auto& kv : m_resfile->GetResolvedPathMap() )
		{
			// Combine filters of both if same key already exists
			auto it = m_resolvedCombinedPathMap.find( kv.first );
			if( it != m_resolvedCombinedPathMap.end() )
			{
				// Combine the filters (using raw filter strings)
				std::string combinedRawFilter = it->second.GetRawFilter() + " " + kv.second.GetRawFilter();
				FilterResourceFilter combinedFilter( combinedRawFilter );
				m_resolvedCombinedPathMap[kv.first] = combinedFilter;
			}
			else
			{
				m_resolvedCombinedPathMap[kv.first] = kv.second;
			}
		}
	}
}

}
