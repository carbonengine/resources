// Copyright © 2026 CCP ehf.

#pragma once
#ifndef ResourceFilter_H
#define ResourceFilter_H

#include <filesystem>
#include <vector>

#include "FilterResourceFile.h"
#include "FilterResourceFilter.h"

namespace ResourceTools
{

// -------------------------------------------------------------
// Description:
//   ResourceFilter is a class that wraps one or more filter .ini
//   files and exposes a way (function FilePathMatchesIncludeFilterRules)
//   to check if a given file path should be included or excluded based
//   on all the filtering rules defined in those .ini file(s).
// -------------------------------------------------------------
class ResourceFilter
{
public:
	ResourceFilter() = default;

	// Construct a ResourceFilter object by passing filter .ini file(s).
	explicit ResourceFilter( const std::vector<std::filesystem::path>& iniFilePaths );

	// Initializes the ResourceFilter by passing in and parsing the supplied filter .ini file(s).
	void Initialize( const std::vector<std::filesystem::path>& iniFilePaths );

	// Returns true if this ResourceFilter has any filter .ini files, false otherwise.
	bool HasFilters() const;

	// Returns the full resolved relative PathMaps from all resource .ini file(s).
	// Key = "resolved relative path", Value = FilterResourceFilter (include/exclude filters)
	const std::map<std::string, FilterResourceFilter>& GetFullResolvedPathMap();

	// Check if the inFilePath should be included or excluded based on
	// filtering rules from all the filter .ini file(s)
	bool FilePathMatchesIncludeFilterRules( const std::filesystem::path& inFilePath );

private:
	// Static helper function for wildcard matching path strings (supports "*" and "...")
	static bool WildcardMatch( std::string pattern, const std::string& checkStr );

	// Static helper function to normalize paths (i.e. deal with \ / .. . etc)
	static std::string NormalizePath( const std::string& path );

	// A flag used to prevent multiple initializations of the ResourceFilter
	bool m_initialized{ false };

	// Vector of all the filter .ini files (wrapped in FilterResourceFile objects)
	std::vector<std::unique_ptr<FilterResourceFile>> m_filterFiles;

	// Resolved PathMap from all the filter .ini files
	std::map<std::string, FilterResourceFilter> m_fullResolvedPathMap;
};

}

#endif // ResourceFilter_H
