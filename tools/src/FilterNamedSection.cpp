// Copyright © 2025 CCP ehf.

#include "FilterNamedSection.h"

#include <stdexcept>

namespace ResourceTools
{

// -------------------------------------------------------------
// Description:
//   Constructs a FilterNamedSection object for a named section
//   within a filter .ini file.
// Arguments:
//   sectionName - name of the section (e.g: [SomeSectionName])
//   rawFilter - the "filter" attribute for this section (e.g: "[ .yaml .txt ]")
//   respaths - the "respaths" attribute of this section (e.g: "prefix:/someSubPath/*")
//   resfile - the optional "resfile" attribute of this section (e.g: "prefix:/pathToSomeFile.txt")
//   parentPrefixMap - the FilterPrefixMap from the [DEFAULT] section of the .ini file,
//     used to resolve prefixes in "respaths" and "resfile" attributes to actual paths.
// -------------------------------------------------------------
FilterNamedSection::FilterNamedSection( const std::string& sectionName,
										const std::string& rawFilter,
										const std::string& rawRespaths,
										const std::string& rawResfile,
										const FilterPrefixMap& parentPrefixMap ) :
	m_sectionName( sectionName ),
	m_filter( rawFilter, true ), // When constructing a [NamedSection], isToplevelFilter should always be true (as it's not an inline filter)
	m_respaths( rawRespaths, parentPrefixMap, m_filter ),
	m_resfile( rawResfile.empty() ? std::nullopt : std::make_optional<FilterResourcePathFile>( rawResfile, parentPrefixMap, m_filter ) )
{
	if( rawRespaths.empty() )
	{
		throw std::invalid_argument( "Respaths attribute is empty for section: " + m_sectionName );
	}
}

// -------------------------------------------------------------
// Description:
//   Gets the name of this section (e.g: [SomeSectionName]) from the .ini file.
// Return Value:
//   String representing the name of the section
// -------------------------------------------------------------
const std::string& FilterNamedSection::GetSectionName() const
{
	return m_sectionName;
}

// -------------------------------------------------------------
// Description:
//   Gets the combined resolved path map from both the "respaths" and optional "resfile" attributes.
//   This is the main getter function to use for the full resolved paths
//   and their associated filters from within this section.
// Return Value:
//   Map of resolved paths to their associated FilterResourceFilter objects.
// Note:
//   The combined map from both the "respaths" and "resfile" attributes
//   is populated during the first call to this function (and then cached).
// -------------------------------------------------------------
const std::map<std::string, FilterResourceFilter>& FilterNamedSection::GetCombinedResolvedPathMap()
{
	// Only populate the Combined map if not already done so.
	if( m_resolvedCombinedPathMap.empty() )
	{
		// Populate the combined map with "respaths" attribute entries first (as they are non-optional)
		for( const auto& kv : m_respaths.GetResolvedPathMap() )
		{
			m_resolvedCombinedPathMap.insert_or_assign( kv.first, kv.second );
		}

		// Add the "resfile" attribute entries to the combined map (in case there are any).
		// Make sure to combine any filters if the same key already exists from the "respaths" attribute.
		if( m_resfile )
		{
			// Allow "resfile" to contain multiple entries (future proofing it, not currently utilized as such in existing .ini filter files)
			for( const auto& kv : m_resfile->GetResolvedPathMap() )
			{
				// Combine filters of both if same key already exists (using the raw filter strings).
				// Else just add the "resfile" attribue entry to the combined map as is.
				auto it = m_resolvedCombinedPathMap.find( kv.first );
				if( it != m_resolvedCombinedPathMap.end() )
				{
					std::string combinedRawFilter = it->second.GetRawFilter() + " " + kv.second.GetRawFilter();
					FilterResourceFilter combinedFilter( combinedRawFilter );
					m_resolvedCombinedPathMap.insert_or_assign( kv.first, combinedFilter );
				}
				else
				{
					m_resolvedCombinedPathMap.insert_or_assign( kv.first, kv.second );
				}
			}
		}
	}

	return m_resolvedCombinedPathMap;
}

// -------------------------------------------------------------
// Description:
//   Gets the resolved path map from the "respaths" attribute only.
// Return Value:
//   Map of resolved paths to their associated FilterResourceFilter objects.
// Note:
//   Users of this class should use the GetCombinedResolvedPathMap()
//   function instead of this one to get the "full combined" result.
//   This function is exposed only to enable tests to verify correctness of data.
// -------------------------------------------------------------
const std::map<std::string, FilterResourceFilter>& FilterNamedSection::GetResolvedRespathsMap() const
{
	return m_respaths.GetResolvedPathMap();
}

// -------------------------------------------------------------
// Description:
//   Gets the resolved path map from the optional "resfile" attribute only.
// Return Value:
//   Pointer to a map of resolved paths to their associated FilterResourceFilter objects,
//   or nullptr if no resfile is present. This is a pointer (and not a reference) because of the optional m_resfile member.
// Note:
//   Users of this class should use the GetCombinedResolvedPathMap()
//   function instead of this one to get the "full combined" result.
//   This function is exposed only to enable tests to verify correctness of data.
// -------------------------------------------------------------
const std::map<std::string, FilterResourceFilter>* FilterNamedSection::GetResolvedResfileMap() const
{
	if( m_resfile )
	{
		return &m_resfile->GetResolvedPathMap();
	}

	return nullptr;
}

}
