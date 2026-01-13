// Copyright © 2025 CCP ehf.

#ifndef FILTERRESOURCEFILE_H
#define FILTERRESOURCEFILE_H

#include <filesystem>
#include <vector>
#include <FilterDefaultSection.h>
#include <FilterNamedSection.h>

namespace ResourceTools
{

class FilterResourceFile
{
public:
	explicit FilterResourceFile( const std::filesystem::path& iniFilePath );

	const std::map<std::string, FilterResourceFilter>& GetFullResolvedPathMap();

private:
	std::filesystem::path m_iniFilePath;

	FilterDefaultSection m_defaultSection;

	std::vector<FilterNamedSection> m_namedSections;

	// Resolved PathMap for all named sections defined in a resource .ini file
	std::map<std::string, FilterResourceFilter> m_fullResolvedPathMap;

	void ParseIniFile();
};

}

#endif // FILTERRESOURCEFILE_H
