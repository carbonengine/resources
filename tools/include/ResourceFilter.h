// Copyright © 2026 CCP ehf.

#ifndef RESOURCEFILTER_H
#define RESOURCEFILTER_H

#include <filesystem>
#include <vector>
#include <memory>
#include <FilterResourceFile.h>

namespace ResourceTools
{

class ResourceFilter
{
public:
	ResourceFilter() = default;

	explicit ResourceFilter( const std::vector<std::filesystem::path>& iniFilePaths );

	void Initialize( const std::vector<std::filesystem::path>& iniFilePaths );

	bool HasFilters() const
	{
		return !m_filterFiles.empty();
	}

	// Returns the full relative resolved PathMaps from all resource .ini file
	// Key is the "relative resolved path", Value is the associated FilterResourceFilter (include and exclude filters)
	const std::map<std::string, FilterResourceFilter>& GetFullResolvedPathMap();

	// Check if the inFilePath should be included or excluded based on filtering rules
	bool ShouldInclude( const std::filesystem::path& inFilePath );

private:
	bool m_initialized{ false };

	std::vector<std::unique_ptr<FilterResourceFile>> m_filterFiles;

	// Resolved PathMap for all .ini files
	std::map<std::string, FilterResourceFilter> m_fullResolvedPathMap;

	// Helper function for wildcard matching paths (supports "*" and "...")
	static bool WildcardMatch( const std::string& pattern, const std::string& checkStr );

	// Helper function to normalize paths (i.e. deal with \ / .. . etc)
	static std::string NormalizePath( const std::string& path );
};

}

#endif // RESOURCEFILTER_H
