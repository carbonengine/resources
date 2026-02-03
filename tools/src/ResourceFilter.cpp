// Copyright © 2026 CCP ehf.

#include <ResourceFilter.h>
#include <FilterResourceFile.h>
#include <regex>
#include <iostream>  // TODO: Added for debugging purposes, remove this

namespace ResourceTools
{

ResourceFilter::ResourceFilter( const std::vector<std::filesystem::path>& iniFilePaths )
{
	Initialize( iniFilePaths );
}

void ResourceFilter::Initialize( const std::vector<std::filesystem::path>& iniFilePaths )
{
	m_fullResolvedPathMap.clear();

	if( m_initialized )
	{
		throw std::runtime_error( "ResourceFilter is already initialized." );
	}

	std::set<std::filesystem::path> uniquePaths( iniFilePaths.begin(), iniFilePaths.end() );
	for( const auto& path : uniquePaths )
	{
		try
		{
			m_filterFiles.emplace_back( std::make_unique<FilterResourceFile>( path ) );
		}
		catch( const std::exception& e )
		{
			// Optionally log or handle error
			std::string errorMsg = "Unable to create ResourceFilter for: " + path.generic_string() + " - because of: " + e.what();
			throw std::runtime_error( errorMsg );
		}
	}

	m_initialized = true;
}

const std::map<std::string, FilterResourceFilter>& ResourceFilter::GetFullResolvedPathMap()
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
					m_fullResolvedPathMap.insert_or_assign( kv.first, kv.second );
				}
			}
		}
	}

	return m_fullResolvedPathMap;
}

bool ResourceFilter::ShouldInclude( const std::filesystem::path& inFilePath )
{
	// Make sure we work with the absolute path representation of the input file
	std::filesystem::path inFilePathAbs = std::filesystem::absolute( inFilePath );
	std::string inFileNormalAbsPathStr = NormalizePath( inFilePathAbs.generic_string() );

	// Priority: lower is higher priority:
	// -1 = exact match on filename (or folder)
	//  0 = wildcard match on same folder level
	//  1 = wildcard match, 1 folder up, etc...
	int bestIncludePriority = std::numeric_limits<int>::max();
	int bestExcludePriority = std::numeric_limits<int>::max();

	// TODO: Add debugging info
	std::cout << " - inFilePath:             " << inFilePath << std::endl;
	std::cout << " - inFilePathAbs:          " << inFilePathAbs.generic_string() << std::endl;
	std::cout << " - inFileNormalAbsPathStr: " << inFileNormalAbsPathStr << std::endl;

	// Get the full resolved path map and iterate through it (contains relative paths)
	const auto& resolvedPathMap = GetFullResolvedPathMap();

	for( const auto& [resolvedRelativePathStr, filter] : resolvedPathMap )
	{
		// Make sure to work with absolute paths for comparison
		std::filesystem::path resolvedRelativePath( resolvedRelativePathStr );
		std::filesystem::path resolvedPathAbs = std::filesystem::absolute( resolvedRelativePath );
		std::string resolvedNormalAbsPathStr = NormalizePath( resolvedPathAbs.generic_string() );

		// TODO: Add debugging info
		std::cout << "   Checking resolvedRelativePathStr:" << std::endl;
		std::cout << "   - resolvedRelativePathStr:  " << resolvedRelativePathStr << std::endl;
		std::cout << "   - resolvedRelativePath:     " << resolvedRelativePath.generic_string() << std::endl;
		std::cout << "   - resolvedPathAbs:          " << resolvedPathAbs.generic_string() << std::endl;
		std::cout << "   - resolvedNormalAbsPathStr: " << resolvedNormalAbsPathStr << std::endl;

		if( resolvedNormalAbsPathStr == inFileNormalAbsPathStr )
		{
			// If there is an exact match on the full filename path, this means highest priority and
			// SHOULD BE considered an "INCLUDE" even though resolvedPath has filters that might say otherwise.
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

			// TODO: Add debugging info
			std::cout << "   - Adjusted resolvedNormalAbsPathStr for ... : " << resolvedNormalAbsPathStr << std::endl;
		}

		// Perform Wildcard matching on the normalized absolute paths
		if( !WildcardMatch( resolvedNormalAbsPathStr, inFileNormalAbsPathStr ) )
		{
			// TODO: Add debugging info
			std::cout << "   No WildcardMatch for pattern: " << resolvedNormalAbsPathStr << " checkStr: " << inFileNormalAbsPathStr << std::endl;

			// There was NO wildcard match on paths, ignore this resolvedRelativePath entry
			continue;
		}

		// There is a Wildcard match - determine the folder depth difference
		// Reset the path variables to their Normalized absolute path components (in case they differ from non-Normalized version)
		inFilePathAbs = std::filesystem::absolute( inFileNormalAbsPathStr );
		resolvedPathAbs = std::filesystem::absolute( resolvedNormalAbsPathStr );

		// TODO: Add debugging info
		std::cout << "   WildcardMatch succeeded!" << std::endl;
		std::cout << "   - inFilePathAbs (normalized):      " << inFilePathAbs.generic_string() << std::endl;
		std::cout << "   - resolvedPathAbs (normalized):    " << resolvedPathAbs.generic_string() << std::endl;

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

	// TODO: Add debugging info
	std::cout << " - bestIncludePriority: " << bestIncludePriority << std::endl;
	std::cout << " - bestExcludePriority: " << bestExcludePriority << std::endl;

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


// pattern = The resolved path from the .ini file (can contain wildcards)
// checkStr = The input file path to check against the pattern
bool ResourceFilter::WildcardMatch( const std::string& pattern, const std::string& checkStr )
{
	// Replace ... with a unique token
	std::string pat = pattern;
	std::string token = "\x01";
	size_t pos;
	while( ( pos = pat.find( "..." ) ) != std::string::npos )
	{
		pat.replace( pos, 3, token );
	}

	// Escape special characters and deal with wildcards
	std::string regexPat;
	for( size_t i = 0; i < pat.size(); ++i )
	{
		if( pat[i] == '*' )
		{
			regexPat += "[^/]*";
		}
		else if( pat[i] == '\x01' )
		{
			regexPat += ".*";
		}
		else if( std::string( ".^$|()[]{}+?\\" ).find( pat[i] ) != std::string::npos )
		{
			// Regex special characters that need escaping
			regexPat += '\\';
			regexPat += pat[i];
		}
		else
		{
			regexPat += pat[i];
		}
	}

	try
	{
		std::regex re( regexPat, std::regex::ECMAScript | std::regex::icase );
		bool regexResult = std::regex_match( checkStr, re );
		return regexResult;
	}
	catch( const std::regex_error& e )
	{
		std::string errorMsg = "Regex Exception during WildcardMatching - regexPattern: " + regexPat + " checkString: " + checkStr + " - error details: " + e.what();
		throw std::runtime_error( errorMsg );
	}
	catch( const std::exception& e )
	{
		std::string errorMsg = "Standard Exception during WildcardMatching - regexPattern: " + regexPat + " checkString: " + checkStr + " - error details: " + e.what();
		throw std::runtime_error( errorMsg );
	}
}

std::string ResourceFilter::NormalizePath( const std::string& path )
{
	std::filesystem::path p( path );
	return p.lexically_normal().generic_string();
}

} // namespace ResourceTools
