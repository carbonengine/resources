// Copyright © 2026 CCP ehf.

#include "ResourceFilter.h"

#include <regex>

namespace ResourceTools
{

// -------------------------------------------------------------
// Description:
//   Construct a ResourceFilter object by passing in filter .ini file(s).
// Arguments:
//   iniFilePaths - vector of file paths to the filter .ini files
//   prefixmapAbsoluteBasePath - absolute base path for resolving prefixmap attribute within filter .ini file(s)
// -------------------------------------------------------------
ResourceFilter::ResourceFilter(
	const std::vector<std::filesystem::path>& iniFilePaths,
	const std::filesystem::path& prefixmapAbsoluteBasePath )
{
	Initialize( iniFilePaths, prefixmapAbsoluteBasePath );
}

// -------------------------------------------------------------
// Description:
//   Initializes the ResourceFilter by passing in and creating
//   FilterResourceFile objects for each of the supplied filter .ini file(s).
// Arguments:
//   iniFilePaths - vector of file paths to the filter .ini files
//   prefixmapAbsoluteBasePath - absolute base path for resolving prefixmap attribute within filter .ini file(s)
// Return Value:
//   None (void).
// -------------------------------------------------------------
void ResourceFilter::Initialize(
	const std::vector<std::filesystem::path>& iniFilePaths,
	const std::filesystem::path& prefixmapAbsoluteBasePath )
{
	if( m_initialized )
	{
		throw std::runtime_error( "ResourceFilter is already initialized." );
	}

	m_prefixmapAbsoluteBasePath = prefixmapAbsoluteBasePath;

	std::set<std::filesystem::path> uniquePaths( iniFilePaths.begin(), iniFilePaths.end() );
	for( const auto& path : uniquePaths )
	{
		try
		{
			// Supplied filter .ini files should already be passed in with absolute path.
			// In case they are not, fallback to resolving absolute path based on the
			// m_prefixmapAbsoluteBasePath property insted of the current working directory.
			// That property is otherwise intended for resolving relative paths within those filter .ini files.
			m_filterFiles.emplace_back( std::make_unique<FilterResourceFile>( GetAbsolutePathFromBase( path, m_prefixmapAbsoluteBasePath ) ) );
		}
		catch( const std::exception& e )
		{
			// Optionally log or handle error
			std::string errorMsg = "Unable to create ResourceFilter for: " + path.generic_string() + " - because of: " + e.what();
			throw std::runtime_error( errorMsg );
		}
	}

	// Now we can populate the full resolved path map from all .ini file(s) in
	// this ResourceFilter (combining filters for any overlapping paths).
	PopulateFullResolvedPathMap();

	m_initialized = true;
}

// -------------------------------------------------------------
// Description:
//   Determine if this ResourceFilter has any filters
// Return Value:
//   True = There are filter .ini files present
//   False = There are no filter .ini files
// -------------------------------------------------------------
bool ResourceFilter::HasFilters() const
{
	return !m_filterFiles.empty();
}

// -------------------------------------------------------------
// Description:
//   Returns the full resolved relative PathMaps from all FilterResourceFile(s)
//   in this ResourceFilter.
// Return Value:
//   Map of resolved paths to their associated filters from all
//   FilterResourceFile(s)
//   Key = "resolved relative path"
//   Value = FilterResourceFilter (with include/exclude filters)
// -------------------------------------------------------------
const std::map<std::string, FilterResourceFilter>& ResourceFilter::GetFullResolvedPathMap() const
{
	return m_fullResolvedPathMap;
}

// -------------------------------------------------------------
// Description:
//   Check if the inFilePath should be included or excluded based on
//   the filtering rules from all .ini file(s) in this ResourceFilter.
// Arguments:
//   inFilePath - the absolute file path to check against the filter rules
// Return Value:
//   True = the file path matches the include filter rules and should be included
//   False = the file path does not match the include filter rules and should be excluded
// -------------------------------------------------------------
bool ResourceFilter::FilePathMatchesIncludeFilterRules( const std::filesystem::path& inFilePath )
{
	// Make sure we work with the lexically normalized (already absolute) path representation of the input file.
	std::string inFileNormalAbsPathStr = NormalizePath( inFilePath.generic_string() );
	std::filesystem::path inFilePathAbs = std::filesystem::absolute( inFileNormalAbsPathStr );

	// Priority: lower is higher priority:
	// -1 = exact match on filename (or folder)
	//  0 = wildcard match on same folder level
	//  1 = wildcard match, 1 folder up, etc...
	int bestIncludePriority = std::numeric_limits<int>::max();
	int bestExcludePriority = std::numeric_limits<int>::max();

	// Get the full resolved path map and iterate through it (contains relative paths)
	const auto& resolvedPathMap = GetFullResolvedPathMap();
	for( const auto& [resolvedRelativePathStr, filter] : resolvedPathMap )
	{
		// Convert the relative path from the filter rules .ini file(s) to a lexically normalized absolute path
		// for comparison with the similarly lexically normalized absolute path of the input file (inFilePath).
		std::string resolvedNormalAbsPathStr = NormalizePath( GetAbsolutePathFromBase( resolvedRelativePathStr, m_prefixmapAbsoluteBasePath ).generic_string() );
		std::filesystem::path resolvedPathAbs = std::filesystem::absolute( resolvedNormalAbsPathStr );

		if( resolvedNormalAbsPathStr == inFileNormalAbsPathStr )
		{
			// If there is an exact match on the full filename path, this means highest priority
			// and the file path should be considered an "include". This is even though resolvedPath
			// may have filters that indicate exclude (explicitly specifying a full filename path
			// should override any wildcard exclusion filters on the same file path).
			bestIncludePriority = -1;
			continue;
		}

		// Make sure the resolvedNormalAbsPathStr contains the "..." (recursive folder wildcard) if the
		// original resolvedRelativePathStr had it.
		// Only check the end of the string for any combination of ["/...", "...", ".../"].
		if( resolvedRelativePathStr.find( "...", resolvedRelativePathStr.size() - 4 ) != std::string::npos )
		{
			if( resolvedNormalAbsPathStr.find( "...", resolvedNormalAbsPathStr.size() - 4 ) == std::string::npos )
			{
				if( resolvedNormalAbsPathStr.back() != '/' )
				{
					resolvedNormalAbsPathStr += '/';
				}
				resolvedNormalAbsPathStr += "...";
			}
		}

		// Perform Wildcard matching on the normalized absolute paths
		if( !WildcardMatch( resolvedNormalAbsPathStr, inFileNormalAbsPathStr ) )
		{
			// There was NO wildcard match on paths, ignore it
			continue;
		}

		// There is a Wildcard match - determine the folder depth difference
		auto inFileIt = inFilePathAbs.begin();
		auto resolvedIt = resolvedPathAbs.begin();
		while( inFileIt != inFilePathAbs.end() && resolvedIt != resolvedPathAbs.end() && *inFileIt == *resolvedIt )
		{
			++inFileIt;
			++resolvedIt;
		}

		std::filesystem::path remainingPath;
		int folderDiffDepthPriority = -1; // Start at -1 (making first iteration priority 0 = same folder level)
		while( inFileIt != inFilePathAbs.end() )
		{
			++folderDiffDepthPriority;
			remainingPath /= *inFileIt;
			++inFileIt;
		}
		std::string remainingPathStr = remainingPath.generic_string();

		// Next step is to check the include/exclude filters:
		for( const auto& includeToken : filter.GetIncludeFilter() )
		{
			if( includeToken == "*" || remainingPathStr.find( includeToken ) != std::string::npos )
			{
				if( folderDiffDepthPriority < bestIncludePriority )
					bestIncludePriority = folderDiffDepthPriority;
			}
		}

		for( const auto& excludeToken : filter.GetExcludeFilter() )
		{
			if( excludeToken == "*" || remainingPathStr.find( excludeToken ) != std::string::npos )
			{
				if( folderDiffDepthPriority < bestExcludePriority )
					bestExcludePriority = folderDiffDepthPriority;
			}
		}
	}

	// Apply priority rules:
	if( bestIncludePriority == std::numeric_limits<int>::max() )
	{
		return false; // No include match found => Exclude the file
	}
	if( bestExcludePriority == std::numeric_limits<int>::max() )
	{
		return true; // No exclude match found (but includePriority less than max => Include the file
	}
	if( bestIncludePriority < bestExcludePriority )
	{
		return true; // Include priority is lower => Include the file
	}
	if( bestExcludePriority < bestIncludePriority )
	{
		return false; // Exclude priority is lower => Exclude the file
	}
	// Both include and exclude have same priority => Exclude the file
	return false;
}

// -------------------------------------------------------------
// Description:
//   Populates the full resolved path map from all .ini file(s) in this ResourceFilter.
// Return Value:
//   None (void).
// -------------------------------------------------------------
void ResourceFilter::PopulateFullResolvedPathMap()
{
	if( m_fullResolvedPathMap.empty() )
	{
		// Populate the full resolved path map from all Filter INI files
		for( auto& iniFile : m_filterFiles )
		{
			auto& iniFilePathMap = iniFile->GetIniFileResolvedPathMap();
			for( const auto& kv : iniFilePathMap )
			{
				// Combine filters if the same path already exists
				auto it = m_fullResolvedPathMap.find( kv.first );
				if( it != m_fullResolvedPathMap.end() )
				{
					// Combine the filters (using raw filter strings)
					std::string combinedRawFilter = it->second.GetRawFilter() + " " + kv.second.GetRawFilter();
					FilterResourceFilter combinedFilter( combinedRawFilter );
					m_fullResolvedPathMap.insert_or_assign( kv.first, combinedFilter );
				}
				else
				{
					m_fullResolvedPathMap.insert( { kv.first, kv.second } );
				}
			}
		}
	}
}

// -------------------------------------------------------------
// Description:
//   Static helper function for wildcard matching path strings.
//   Supports the following wildcards:
//   - "*"   = matches any sequence of characters (at the same folder level)
//   - "..." = matches any sequence of characters (as any recursive folder level)
// Arguments:
//   pattern - The resolved path from the .ini file (can contain wildcards)
//   checkStr - The input file path to check against the pattern
// Return Value:
//   True = the checkStr matches the pattern (exact or with wildcards)
//   False = the checkStr does not match the pattern (neither exact nor with wildcards)
// -------------------------------------------------------------
bool ResourceFilter::WildcardMatch( std::string pattern, const std::string& checkStr )
{
	// Replace any "..." with a unique token (RECURSIVE_FOLDER_ELLIPSES_WILDCARD)
	constexpr char RECURSIVE_FOLDER_ELLIPSES_WILDCARD = '\x01';
	size_t pos;
	while( ( pos = pattern.find( "..." ) ) != std::string::npos )
	{
		pattern.replace( pos, 3, std::string( 1, RECURSIVE_FOLDER_ELLIPSES_WILDCARD ) );
	}

	// Escape special characters and deal with wildcards ("*" and "..." i.e. RECURSIVE_FOLDER_ELLIPSES_WILDCARD)
	std::string regexPattern;
	for( size_t i = 0; i < pattern.size(); ++i )
	{
		if( pattern[i] == '*' )
		{
			regexPattern += "[^/]*";
		}
		else if( pattern[i] == RECURSIVE_FOLDER_ELLIPSES_WILDCARD )
		{
			regexPattern += ".*";
		}
		else if( std::string( ".^$|()[]{}+?\\" ).find( pattern[i] ) != std::string::npos )
		{
			// Regex special characters that need escaping
			regexPattern += '\\';
			regexPattern += pattern[i];
		}
		else
		{
			regexPattern += pattern[i];
		}
	}

	try
	{
		std::regex re( regexPattern, std::regex::ECMAScript | std::regex::icase );
		bool regexResult = std::regex_match( checkStr, re );
		return regexResult;
	}
	catch( const std::regex_error& e )
	{
		std::string errorMsg = "Regex Exception during WildcardMatching - regexPattern: " + regexPattern + " checkString: " + checkStr + " - error details: " + e.what();
		throw std::runtime_error( errorMsg );
	}
	catch( const std::exception& e )
	{
		std::string errorMsg = "Standard Exception during WildcardMatching - regexPattern: " + regexPattern + " checkString: " + checkStr + " - error details: " + e.what();
		throw std::runtime_error( errorMsg );
	}
}

// -------------------------------------------------------------
// Description:
//   Static helper function to normalize paths by removing redundant
//   path components such as "." and ".." and converting to a generic format.
// Arguments:
//   path - the file path to normalize
// Return Value:
//   Normalized path string in generic format (using '/' as separator)
// -------------------------------------------------------------
std::string ResourceFilter::NormalizePath( const std::string& path )
{
	std::filesystem::path p( path );
	return p.lexically_normal().generic_string();
}

// -------------------------------------------------------------
// Description:
//   Static helper function to get an absolute path by combining a
//   relative path with an absolute base path (if not already absolute).
// Arguments:
//   relativeOrAbsolutePath - the input file path that is either relative or absolute
//   absoluteBasePath - the absolute base path to use (if needed)
// Return Value:
//   Guaranteed absolute file path
// -------------------------------------------------------------
std::filesystem::path ResourceFilter::GetAbsolutePathFromBase( const std::filesystem::path& relativeOrAbsolutePath,
															   const std::filesystem::path& absoluteBasePath )
{
	if( relativeOrAbsolutePath.is_absolute() )
	{
		// Path is already absolute, just return it
		return relativeOrAbsolutePath;
	}

	// Path is relative, combine it with the absolute base path
	return absoluteBasePath / relativeOrAbsolutePath;
}

} // namespace ResourceTools
