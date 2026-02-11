// Copyright © 2025 CCP ehf.

#pragma once
#ifndef FilterResourceFile_H
#define FilterResourceFile_H

#include <filesystem>
#include <vector>

#include "FilterDefaultSection.h"
#include "FilterNamedSection.h"

namespace ResourceTools
{

// -------------------------------------------------------------
// Description:
//   FilterResourceFile is a class that represents the fully
//   parsed resource .ini file.
// -------------------------------------------------------------
class FilterResourceFile
{
public:
	// Construct a FilterResourceFile object by parsing the supplied resource .ini file.
	explicit FilterResourceFile( const std::filesystem::path& iniFilePath );

	// Returns the fully resolved PathMaps for all named sections
	// within this resource .ini file.
	// Key = "resolved path", Value = associated include/exclude filters
	const std::map<std::string, FilterResourceFilter>& GetIniFileResolvedPathMap() const;

private:
	// Parses the resource .ini file and populates the m_defaultSection and m_namedSections members.
	void ParseIniFile( const std::filesystem::path& iniFilePath );

	// Populates the m_iniFileResolvedPathMap member by combining the resolved path maps from all named sections in this INI file.
	void PopulateIniFileResolvedPathMap();

	// The parsed [DEFAULT] section of the resource .ini file
	FilterDefaultSection m_defaultSection;

	// Vector of all th parsed [NamedSection(s)] defined in the .ini file
	std::vector<FilterNamedSection> m_namedSections;

	// Resolved PathMap for all named sections defined in a resource .ini file
	std::map<std::string, FilterResourceFilter> m_iniFileResolvedPathMap;
};

}

#endif // FilterResourceFile_H
