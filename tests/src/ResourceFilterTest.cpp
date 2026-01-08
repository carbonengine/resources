// Copyright © 2025 CCP ehf.

#include "ResourceFilterTest.h"

#include <INIReader.h>
#include <FilterResourceFilter.h>
#include <FilterPrefixMap.h>
#include <FilterPrefixMapEntry.h>
#include <FilterDefaultSection.h>
#include <FilterResourcePathFile.h>

TEST_F( ResourceFilterTest, Example1IniParsing )
{
	// Use the test fixture's helper to get the absolute path
	const std::filesystem::path iniPath = GetTestFileFileAbsolutePath( "ExampleIniFiles/example1.ini" );
	INIReader reader( iniPath.string() );
	ASSERT_EQ( reader.ParseError(), 0 ) << "Failed to parse example1.ini";

	// There should only be 2 sections
	EXPECT_EQ( reader.Sections().size(), 2 );

	// Check [default] section
	ASSERT_TRUE( reader.HasSection( "default" ) );
	EXPECT_EQ( reader.Get( "default", "prefixmap", "" ), "res:./Indicies;./resourcesOnBranch res2:./ResourceGroups" );
	EXPECT_EQ( reader.Get( "default", "version", "" ), "1.2" );
	EXPECT_EQ( reader.Keys( "default" ).size(), 2 );

	// Check [testyamlfilesovermultilinerespaths] section
	ASSERT_TRUE( reader.HasSection( "testYamlFilesOverMultiLineResPathsWithEmptyLines" ) );
	EXPECT_EQ( reader.Get( "testYamlFilesOverMultiLineResPathsWithEmptyLines", "filter", "" ), "[ .yaml ]" );
	EXPECT_EQ( reader.Get( "testYamlFilesOverMultiLineResPathsWithEmptyLines", "resfile", "" ), "res:/binaryFileIndex_v0_0_0.txt" );
	auto respathValueGet = reader.Get( "testYamlFilesOverMultiLineResPathsWithEmptyLines", "respaths", "" );
	std::string respathValueGetString = reader.Get( "testYamlFilesOverMultiLineResPathsWithEmptyLines", "respaths", "" );
	EXPECT_EQ( respathValueGet, respathValueGetString );
	EXPECT_EQ( respathValueGet, "res:/firstLine/...\nres:/secondLine/...\nres2:/thirdLine/..." ); // Note: Under the hood, INIReader converts multi-empty-lines to a single \n line breaks
	EXPECT_EQ( reader.Keys( "testYamlFilesOverMultiLineResPathsWithEmptyLines" ).size(), 3 );
}

// -----------------------------------------

TEST_F( ResourceFilterTest, OnlyIncludeFilter )
{
	ResourceTools::FilterResourceFilter filter( "[ .this .is .included ]" );
	const auto& includes = filter.GetIncludeFilter();
	const auto& excludes = filter.GetExcludeFilter();
	EXPECT_EQ( includes.size(), 3 );
	EXPECT_EQ( includes[0], ".this" );
	EXPECT_EQ( includes[1], ".is" );
	EXPECT_EQ( includes[2], ".included" );
	EXPECT_TRUE( excludes.empty() );
}

TEST_F( ResourceFilterTest, OnlyExcludeFilter )
{
	ResourceTools::FilterResourceFilter filter( "![ .excluded .extension ]" );
	const auto& includes = filter.GetIncludeFilter();
	const auto& excludes = filter.GetExcludeFilter();
	EXPECT_TRUE( includes.empty() );
	EXPECT_EQ( excludes.size(), 2 );
	EXPECT_EQ( excludes[0], ".excluded" );
	EXPECT_EQ( excludes[1], ".extension" );
}

TEST_F( ResourceFilterTest, ComplexIncludeExcludeFilter )
{
	ResourceTools::FilterResourceFilter filter( "[ .red .gr2 .dds .png .yaml ] [ .txt ] ![ .csv .xls ] [ .bat .sh ] ![ .blk .yel ]" );
	const auto& includes = filter.GetIncludeFilter();
	const auto& excludes = filter.GetExcludeFilter();
	std::vector<std::string> expectedIncludes = { ".red", ".gr2", ".dds", ".png", ".yaml", ".txt", ".bat", ".sh" };
	std::vector<std::string> expectedExcludes = { ".csv", ".xls", ".blk", ".yel" };
	EXPECT_EQ( includes, expectedIncludes );
	EXPECT_EQ( excludes, expectedExcludes );
}

TEST_F( ResourceFilterTest, SimpleIncludeFilter )
{
	ResourceTools::FilterResourceFilter filter( "[ .red ]" );
	const auto& includes = filter.GetIncludeFilter();
	EXPECT_EQ( includes.size(), 1 );
	EXPECT_EQ( includes[0], ".red" );
}

TEST_F( ResourceFilterTest, SimpleExcludeFilter )
{
	ResourceTools::FilterResourceFilter filter( "![ .blk ]" );
	const auto& excludes = filter.GetExcludeFilter();
	EXPECT_EQ( excludes.size(), 1 );
	EXPECT_EQ( excludes[0], ".blk" );
}

TEST_F( ResourceFilterTest, IncludeExcludeInclude )
{
	// Include .in1 and .in2
	// Exclude .in2, .ex1, and .ex2         (removes .in2 from include)
	// Include .ex1, .in3 and .in1	        (removes .ex1 from exclude, adds .in3, keeps .in1)
	// Resulting include: .in1, .ex1, .in3
	// Resulting exclude: .in2, .ex2
	ResourceTools::FilterResourceFilter filter( "[ .in1 .in2 ] ![ .in2 .ex1 .ex2 ] [ .ex1 .in3 .in1 ]" );
	const auto& includes = filter.GetIncludeFilter();
	const auto& excludes = filter.GetExcludeFilter();
	std::vector<std::string> expectedIncludes = { ".in1", ".ex1", ".in3" };
	std::vector<std::string> expectedExcludes = { ".in2", ".ex2" };
	EXPECT_EQ( includes, expectedIncludes );
	EXPECT_EQ( excludes, expectedExcludes );
}

TEST_F( ResourceFilterTest, MissingClosingIncludeBracketBeforeNextOpenExcludeBracket )
{
	try
	{
		// This test filter has a missing closing bracket for the first include filter, before the next exclude filter starts
		ResourceTools::FilterResourceFilter filter( "[ .in1   !  [ .ex1 ]" );
		FAIL() << "Expected std::invalid_argument (1)";
	}
	catch( const std::invalid_argument& e )
	{
		EXPECT_STREQ( e.what(), "Invalid filter format: matching end bracket ']' not present before the next start bracket '['" );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument (2)";
	}
}

TEST_F( ResourceFilterTest, ExcludeMarkerWithoutBracket_v1 )
{
	try
	{
		ResourceTools::FilterResourceFilter filter( "! .ex1 ]" );
		FAIL() << "Expected std::invalid_argument (1)";
	}
	catch( const std::invalid_argument& e )
	{
		EXPECT_STREQ( e.what(), "Invalid filter format: missing '['" );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument (2)";
	}
}

TEST_F( ResourceFilterTest, ExcludeMarkerWithoutBracket_v2 )
{
	try
	{
		ResourceTools::FilterResourceFilter filter( "  [ .in1 ] ! .ex1 ]" );
		FAIL() << "Expected std::invalid_argument (1)";
	}
	catch( const std::invalid_argument& e )
	{
		EXPECT_STREQ( e.what(), "Invalid filter format: missing '['" );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument (2)";
	}
}

TEST_F( ResourceFilterTest, ExcludeMarkerWithoutBracket_v3 )
{
	try
	{
		ResourceTools::FilterResourceFilter filter( "  [ .in1 ] ![ .ex1 ] ! " );
		FAIL() << "Expected std::invalid_argument (1)";
	}
	catch( const std::invalid_argument& e )
	{
		EXPECT_STREQ( e.what(), "Invalid filter format: exclude filter marker found without a [ token ] section" );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument (2)";
	}
}

TEST_F( ResourceFilterTest, MissingOpeningBracket_v1 )
{
	try
	{
		ResourceTools::FilterResourceFilter filter( ".in1 .in2 ]" );
		FAIL() << "Expected std::invalid_argument (1)";
	}
	catch( const std::invalid_argument& e )
	{
		EXPECT_STREQ( e.what(), "Invalid filter format: missing '['" );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument (2)";
	}
}

TEST_F( ResourceFilterTest, MissingOpeningBracket_v2 )
{
	try
	{
		ResourceTools::FilterResourceFilter filter( " [ .in1 .in2 ] .in3 ] " );
		FAIL() << "Expected std::invalid_argument (1)";
	}
	catch( const std::invalid_argument& e )
	{
		EXPECT_STREQ( e.what(), "Invalid filter format: missing '['" );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument (2)";
	}
}

TEST_F( ResourceFilterTest, MissingClosingBracket_v1 )
{
	try
	{
		ResourceTools::FilterResourceFilter filter( "[ .in1 .in2 " );
		FAIL() << "Expected std::invalid_argument (1)";
	}
	catch( const std::invalid_argument& e )
	{
		EXPECT_STREQ( e.what(), "Invalid filter format: missing ']'" );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument (2)";
	}
}

TEST_F( ResourceFilterTest, MissingClosingBracket_v2 )
{
	try
	{
		ResourceTools::FilterResourceFilter filter( "[ .in1 .in2 ] [ .in3 " );
		FAIL() << "Expected std::invalid_argument (1)";
	}
	catch( const std::invalid_argument& e )
	{
		EXPECT_STREQ( e.what(), "Invalid filter format: missing ']'" );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument (2)";
	}
}

TEST_F( ResourceFilterTest, MissingClosingBracket_v3 )
{
	try
	{
		ResourceTools::FilterResourceFilter filter( "[ .in1 .in2 ] [ .in3 [ .in4 ]" );
		FAIL() << "Expected std::invalid_argument (1)";
	}
	catch( const std::invalid_argument& e )
	{
		EXPECT_STREQ( e.what(), "Invalid filter format: matching end bracket ']' not present before the next start bracket '['" );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument (2)";
	}
}

TEST_F( ResourceFilterTest, CondensedValidFilterStringv1 )
{
	ResourceTools::FilterResourceFilter filter( "[inToken1 inToken2]![exToken1 exToken2][inToken3]" );
	const auto& includes = filter.GetIncludeFilter();
	const auto& excludes = filter.GetExcludeFilter();
	std::vector<std::string> expectedIncludes = { "inToken1", "inToken2", "inToken3" };
	std::vector<std::string> expectedExcludes = { "exToken1", "exToken2" };
	EXPECT_EQ( includes, expectedIncludes );
	EXPECT_EQ( excludes, expectedExcludes );
}

TEST_F( ResourceFilterTest, CondensedValidFilterStringv2 )
{
	ResourceTools::FilterResourceFilter filter( "![exToken1][inToken1 inToken2]![exToken2][inToken3]" );
	const auto& includes = filter.GetIncludeFilter();
	const auto& excludes = filter.GetExcludeFilter();
	std::vector<std::string> expectedIncludes = { "inToken1", "inToken2", "inToken3" };
	std::vector<std::string> expectedExcludes = { "exToken1", "exToken2" };
	EXPECT_EQ( includes, expectedIncludes );
	EXPECT_EQ( excludes, expectedExcludes );
}

// -----------------------------------------

TEST_F( ResourceFilterTest, SinglePrefixMultiplePaths )
{
	ResourceTools::FilterPrefixMap map( "prefix1:/somePath;../otherPath" );
	const auto& prefixMap = map.GetPrefixMap();
	ASSERT_EQ( prefixMap.size(), 1 ) << "There should only be 1 prefix in the map";

	// If iterator is at end, the prefix was not found
	auto it = prefixMap.find( "prefix1" );
	ASSERT_NE( it, prefixMap.end() ) << "Prefix 'prefix1' not found in the map";

	std::set<std::string> expected = { "/somePath", "../otherPath" };
	EXPECT_EQ( it->second.GetPrefix(), "prefix1" ) << "Prefix should be 'prefix1'";
	EXPECT_EQ( it->first, it->second.GetPrefix() ) << "Value of FilterPrefixMap.m_prefixMap key does not match associated FilterPrefixMapEntry.m_prefix";
	EXPECT_EQ( it->second.GetPaths(), expected ) << "Paths do not match expected values";
}

TEST_F( ResourceFilterTest, MultiplePrefixes )
{
	ResourceTools::FilterPrefixMap map( "prefix1:/path1;/path2 prefix2:/newPath1" );
	const auto& prefixMap = map.GetPrefixMap();
	ASSERT_EQ( prefixMap.size(), 2 ) << "There should be 2 prefixes in the map";

	// Make sure both prefixes exist
	auto it1 = prefixMap.find( "prefix1" );
	auto it2 = prefixMap.find( "prefix2" );
	ASSERT_NE( it1, prefixMap.end() ) << "Prefix 'prefix1' not found in the map";
	ASSERT_NE( it2, prefixMap.end() ) << "Prefix 'prefix2' not found in the map";

	std::set<std::string> expected1 = { "/path1", "/path2" };
	std::set<std::string> expected2 = { "/newPath1" };
	EXPECT_EQ( it1->second.GetPrefix(), "prefix1" ) << "Prefix should be 'prefix1'";
	EXPECT_EQ( it2->second.GetPrefix(), "prefix2" ) << "Prefix should be 'prefix2'";
	EXPECT_EQ( it1->first, it1->second.GetPrefix() ) << "Value of FilterPrefixMap.m_prefixMap key does not match associated FilterPrefixMapEntry.m_prefix";
	EXPECT_EQ( it2->first, it2->second.GetPrefix() ) << "Value of FilterPrefixMap.m_prefixMap key does not match associated FilterPrefixMapEntry.m_prefix";
	EXPECT_EQ( it1->second.GetPaths(), expected1 ) << "Paths do not match expected values";
	EXPECT_EQ( it2->second.GetPaths(), expected2 ) << "Paths do not match expected values";
}

TEST_F( ResourceFilterTest, DuplicateSamePrefixPathsInDifferentOrder )
{
	ResourceTools::FilterPrefixMap map( "prefix1:/path1;/path2 prefix1:/path2;/path1" );

	// There should only be one prefix (prefix1)
	const auto& prefixMap = map.GetPrefixMap();
	ASSERT_EQ( prefixMap.size(), 1 ) << "There should only be 1 prefix in the map";
	auto it = prefixMap.find( "prefix1" );
	ASSERT_NE( it, prefixMap.end() ) << "Prefix 'prefix1' not found in the map";

	// There should be only 2 paths, sorted in set
	std::set<std::string> expected_a = { "/path1", "/path2" };
	std::set<std::string> expected_b = { "/path2", "/path1" };
	EXPECT_EQ( it->second.GetPrefix(), "prefix1" ) << "Prefix should be 'prefix1'";
	EXPECT_EQ( it->second.GetPaths(), expected_a ) << "Paths do not match expected values - inserted in alphabetical order";
	EXPECT_EQ( it->second.GetPaths(), expected_b ) << "Paths do not match expected values - inserted in reverse alphabetical order";
	EXPECT_EQ( it->first, it->second.GetPrefix() ) << "Value of FilterPrefixMap.m_prefixMap key does not match associated FilterPrefixMapEntry.m_prefix";
}

TEST_F( ResourceFilterTest, MultiplePrefixesAppendToPaths )
{
	ResourceTools::FilterPrefixMap map( "prefix1:/path2;/path1 prefix2:/otherPath1;/otherPath2 prefix1:/path3;/path1" );
	const auto& prefixMap = map.GetPrefixMap();
	ASSERT_EQ( prefixMap.size(), 2 ) << "There should be 2 prefixes in the map";
	auto it1 = prefixMap.find( "prefix1" );
	auto it2 = prefixMap.find( "prefix2" );
	ASSERT_NE( it1, prefixMap.end() ) << "Prefix 'prefix1' not found in the map";
	ASSERT_NE( it2, prefixMap.end() ) << "Prefix 'prefix2' not found in the map";

	// Prefix1 should have 3 paths and prefix2 should have 2
	std::set<std::string> prefix1Paths = { "/path1", "/path2", "/path3" };
	std::set<std::string> prefix2Paths = { "/otherPath1", "/otherPath2" };
	EXPECT_EQ( it1->second.GetPrefix(), "prefix1" ) << "Prefix should be 'prefix1'";
	EXPECT_EQ( it2->second.GetPrefix(), "prefix2" ) << "Prefix should be 'prefix2'";
	EXPECT_EQ( it1->second.GetPaths(), prefix1Paths ) << "Paths do not match expected values";
	EXPECT_EQ( it2->second.GetPaths(), prefix2Paths ) << "Paths do not match expected values";
	EXPECT_EQ( it1->first, it1->second.GetPrefix() ) << "Value of FilterPrefixMap.m_prefixMap key does not match associated FilterPrefixMapEntry.m_prefix";
	EXPECT_EQ( it2->first, it2->second.GetPrefix() ) << "Value of FilterPrefixMap.m_prefixMap key does not match associated FilterPrefixMapEntry.m_prefix";
}

TEST_F( ResourceFilterTest, DifferentWhitespacesBetweenPrefixes )
{
	std::string input = "prefix1:/path1\tprefixTab:/path2\nprefixNewLine:/path3";
	ResourceTools::FilterPrefixMap map( input );
	const auto& prefixMap = map.GetPrefixMap();
	ASSERT_EQ( prefixMap.size(), 3 ) << "There should only be 3 prefix in the map";
	auto it1 = prefixMap.find( "prefix1" );
	auto it2 = prefixMap.find( "prefixTab" );
	auto it3 = prefixMap.find( "prefixNewLine" );
	EXPECT_NE( it1, prefixMap.end() ) << "Prefix 'prefix1' not found in the map";
	EXPECT_NE( it2, prefixMap.end() ) << "Prefix 'prefixTab' not found in the map";
	EXPECT_NE( it3, prefixMap.end() ) << "Prefix 'prefixNewLine' not found in the map";

	std::set<std::string> prefix1Paths = { "/path1" };
	std::set<std::string> prefixTabPaths = { "/path2" };
	std::set<std::string> prefixNewLinePaths = { "/path3" };
	EXPECT_EQ( it1->second.GetPrefix(), "prefix1" ) << "Prefix should be 'prefix1'";
	EXPECT_EQ( it2->second.GetPrefix(), "prefixTab" ) << "Prefix should be 'prefixTab'";
	EXPECT_EQ( it3->second.GetPrefix(), "prefixNewLine" ) << "Prefix should be 'prefixNewLine'";
	EXPECT_EQ( it1->second.GetPaths(), prefix1Paths ) << "Paths do not match expected values";
	EXPECT_EQ( it2->second.GetPaths(), prefixTabPaths ) << "Paths do not match expected values";
	EXPECT_EQ( it3->second.GetPaths(), prefixNewLinePaths ) << "Paths do not match expected values";
	EXPECT_EQ( it1->first, it1->second.GetPrefix() ) << "Value of FilterPrefixMap.m_prefixMap key does not match associated FilterPrefixMapEntry.m_prefix";
	EXPECT_EQ( it2->first, it2->second.GetPrefix() ) << "Value of FilterPrefixMap.m_prefixMap key does not match associated FilterPrefixMapEntry.m_prefix";
	EXPECT_EQ( it3->first, it3->second.GetPrefix() ) << "Value of FilterPrefixMap.m_prefixMap key does not match associated FilterPrefixMapEntry.m_prefix";
}

TEST_F( ResourceFilterTest, ParsePrefixMap_InvalidMissingColon )
{
	try
	{
		ResourceTools::FilterPrefixMap prefixmap( "prefix1/path1" );
		FAIL() << "Expected std::invalid_argument (1)";
	}
	catch( const std::invalid_argument& e )
	{
		EXPECT_STREQ( e.what(), "Invalid prefixmap format: missing ':'" );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument (2)";
	}
}

TEST_F( ResourceFilterTest, ParsePrefixMap_InvalidEmptyPrefix )
{
	try
	{
		ResourceTools::FilterPrefixMap prefixmap( ":/path1" );
		FAIL() << "Expected std::invalid_argument (1)";
	}
	catch( const std::invalid_argument& e )
	{
		EXPECT_STREQ( e.what(), "Invalid prefixmap format: empty prefix" );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument (2)";
	}
}

TEST_F( ResourceFilterTest, ParsePrefixMap_InvalidNoPaths )
{
	try
	{
		ResourceTools::FilterPrefixMap prefixmap( "prefix1:" );
		FAIL() << "Expected std::invalid_argument (1)";
	}
	catch( const std::invalid_argument& e )
	{
		EXPECT_STREQ( e.what(), "Invalid prefixmap format: No paths defined for prefix: prefix1" );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument (2)";
	}
}

TEST_F( ResourceFilterTest, FilterPrefixMapEntry_PrefixMismatchOnAppend )
{
	try
	{
		ResourceTools::FilterPrefixMapEntry entry( "prefix1", "/path1" );
		entry.AppendPaths( "prefix2", "/path2" );
		FAIL() << "Expected std::invalid_argument (1)";
	}
	catch( const std::invalid_argument& e )
	{
		EXPECT_STREQ( e.what(), "Prefix mismatch while appending path(s): prefix2 (incoming) != prefix1 (existing)" );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument (2)";
	}
}

TEST_F( ResourceFilterTest, FilterPrefixMapEntry_InvalidNoPathsOnAppend )
{
	try
	{
		ResourceTools::FilterPrefixMapEntry entry( "prefix1", "" ); // Empty string for paths
		FAIL() << "Expected std::invalid_argument (1)";
	}
	catch( const std::invalid_argument& e )
	{
		EXPECT_STREQ( e.what(), "Invalid prefixmap format: No paths appended for prefix: prefix1" );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument (2)";
	}
}

// -----------------------------------------

TEST_F( ResourceFilterTest, FilterDefaultSection_InitializeValid )
{
	std::string input = "prefix1:/path1;../path2 prefix2:/path3";
	ResourceTools::FilterDefaultSection defaultSection( input );
	const auto& prefixMap = defaultSection.GetPrefixMap();
	ASSERT_EQ( prefixMap.size(), 2 ) << "There should be 2 prefixes in the map";
	auto it1 = prefixMap.find( "prefix1" );
	auto it2 = prefixMap.find( "prefix2" );
	ASSERT_NE( it1, prefixMap.end() ) << "Prefix 'prefix1' not found in the map";
	ASSERT_NE( it2, prefixMap.end() ) << "Prefix 'prefix2' not found in the map";

	std::set<std::string> prefix1Paths = { "/path1", "../path2" };
	std::set<std::string> prefix2Paths = { "/path3" };
	EXPECT_EQ( it1->second.GetPrefix(), "prefix1" ) << "Prefix should be 'prefix1'";
	EXPECT_EQ( it2->second.GetPrefix(), "prefix2" ) << "Prefix should be 'prefix2'";
	EXPECT_EQ( it1->second.GetPaths(), prefix1Paths ) << "Paths do not match expected values";
	EXPECT_EQ( it2->second.GetPaths(), prefix2Paths ) << "Paths do not match expected values";
	EXPECT_EQ( it1->first, it1->second.GetPrefix() ) << "Value of FilterPrefixMap.m_prefixMap key does not match associated FilterPrefixMapEntry.m_prefix";
	EXPECT_EQ( it2->first, it2->second.GetPrefix() ) << "Value of FilterPrefixMap.m_prefixMap key does not match associated FilterPrefixMapEntry.m_prefix";
}

TEST_F( ResourceFilterTest, FilterDefaultSection_InitializeInvalidMissingColon )
{
	try
	{
		ResourceTools::FilterDefaultSection defaultSection( "prefix1/path1" );
		FAIL() << "Expected std::invalid_argument (1)";
	}
	catch( const std::invalid_argument& e )
	{
		EXPECT_STREQ( e.what(), "Invalid prefixmap format: missing ':'" );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument (2)";
	}
}

TEST_F( ResourceFilterTest, FilterDefaultSection_InitializeInvalidEmptyPrefix )
{
	try
	{
		ResourceTools::FilterDefaultSection defaultSection( ":/path1" );
		FAIL() << "Expected std::invalid_argument (1)";
	}
	catch( const std::invalid_argument& e )
	{
		EXPECT_STREQ( e.what(), "Invalid prefixmap format: empty prefix" );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument (2)";
	}
}

// -----------------------------------------

TEST_F( ResourceFilterTest, FilterResourcePathFile_SingleLine_NoFilter )
{
	std::string prefixMapStr = "prefix1:/path1;../path2";
	std::string parentFilterStr = "[ .in1 .in2 ] ![ .ex1 ]";
	ResourceTools::FilterPrefixMap prefixMap( prefixMapStr );
	ResourceTools::FilterResourceFilter parentFilter( parentFilterStr );

	std::string rawResPathAttrib = "prefix1:/foo/bar";
	ResourceTools::FilterResourcePathFile pathFile( rawResPathAttrib, prefixMap, parentFilter );
	const auto& resolvedPathMap = pathFile.GetResolvedPathMap();

	// Check the resolved path and filters against expected
	std::set<std::string> expectedPaths = { "/path1/foo/bar", "../path2/foo/bar" };
	std::vector<std::string> expectedIncludes = { ".in1", ".in2" };
	std::vector<std::string> expectedExcludes = { ".ex1" };

	for( const auto& p : expectedPaths )
	{
		EXPECT_TRUE( resolvedPathMap.count( p ) );
	}

	for( const auto& kv : resolvedPathMap )
	{
		EXPECT_EQ( kv.second.GetIncludeFilter(), expectedIncludes );
		EXPECT_EQ( kv.second.GetExcludeFilter(), expectedExcludes );
	}
}

TEST_F( ResourceFilterTest, FilterResourcePathFile_SingleLine_InlineIncludeExclude )
{
	std::string prefixMapStr = "prefix1:/path1";
	std::string parentFilterStr = "[ .in1 .in2 ] ![ .ex1 ]";
	ResourceTools::FilterPrefixMap prefixMap( prefixMapStr );
	ResourceTools::FilterResourceFilter parentFilter( parentFilterStr );

	std::string rawPathFileAttrib = "prefix1:/foo/bar [ .inLine1 ] ![ .exLine1 ]";
	ResourceTools::FilterResourcePathFile pathFile( rawPathFileAttrib, prefixMap, parentFilter );
	const auto& resolvedPathMap = pathFile.GetResolvedPathMap();

	// Check the resolved path and filters against expected
	std::set<std::string> expectedPaths = { "/path1/foo/bar" };
	std::vector<std::string> expectedIncludes = { ".in1", ".in2", ".inLine1" };
	std::vector<std::string> expectedExcludes = { ".ex1", ".exLine1" };

	for( const auto& p : expectedPaths )
	{
		EXPECT_TRUE( resolvedPathMap.count( p ) );
	}

	for( const auto& kv : resolvedPathMap )
	{
		EXPECT_EQ( kv.second.GetIncludeFilter(), expectedIncludes );
		EXPECT_EQ( kv.second.GetExcludeFilter(), expectedExcludes );
	}
}

TEST_F( ResourceFilterTest, FilterResourcePathFile_SingleLine_InlineOverridesParentFilter )
{
	std::string prefixMapStr = "prefix1:/path1;../subPath2;/path3";
	std::string parentFilterStr = "[ .parIn1 .parIn2 ] ![ .parEx1 ]";
	ResourceTools::FilterPrefixMap prefixMap( prefixMapStr );
	ResourceTools::FilterResourceFilter parentFilter( parentFilterStr );

	// Override the "location" of the parent include filter (.parIn2) and exclude filter (.parEx1), moving them to opposite filter side
	std::string rawPathFileAttrib = "prefix1:/foo [ .lineIn1 .parEx1 ] ![ .parIn2 .lineEx1 ]";
	ResourceTools::FilterResourcePathFile pathFile( rawPathFileAttrib, prefixMap, parentFilter );
	const auto& resolvedPathMap = pathFile.GetResolvedPathMap();

	// Check the resolved path and filters against expected (some filters switched around)
	std::set<std::string> expectedPaths = { "/path1/foo", "../subPath2/foo", "/path3/foo" };
	std::vector<std::string> expectedIncludes = { ".parIn1", ".lineIn1", ".parEx1" };
	std::vector<std::string> expectedExcludes = { ".parIn2", ".lineEx1" };

	for( const auto& p : expectedPaths )
	{
		EXPECT_TRUE( resolvedPathMap.count( p ) );
	}

	for( const auto& kv : resolvedPathMap )
	{
		EXPECT_EQ( kv.second.GetIncludeFilter(), expectedIncludes );
		EXPECT_EQ( kv.second.GetExcludeFilter(), expectedExcludes );
	}
}

TEST_F( ResourceFilterTest, FilterResourcePathFile_MultiLine_MixedFiltersWithOverrides )
{
	std::string prefixMapStr = "prefix1:/path1;../path2 prefix2:/path3";
	std::string parentFilterStr = "[ .parIn1 .parIn2 ] ![ .parEx1 ]";
	ResourceTools::FilterPrefixMap prefixMap( prefixMapStr );
	ResourceTools::FilterResourceFilter parentFilter( parentFilterStr );

	std::string rawPathFileAttrib =
		"prefix1:/firstLine [ .inLine1 ] ![ .parIn1 ]\n"  // Add .inLine1 to include and move .parIn1 from include to exclude filter
		"prefix2:/secondLine\n"                           // Keep parent filters unchanged
		"prefix1:/thirdLine ![ .exLine3 ] [ .parEx1 ]\n"  // Add .exLine3 to exclude filter and move .parEx1 from exclude to include
		"prefix2:/fourthLine [ .inLine4 ] ![ .exLine4 ]"; // Add .inLine4 to include and .exLine4 to exclude filter
	ResourceTools::FilterResourcePathFile pathFile( rawPathFileAttrib, prefixMap, parentFilter );
	const auto& resolved = pathFile.GetResolvedPathMap();

	// Check the resolved path and filters against expected (multiple switching of filters and overrides)
	std::set<std::string> expectedPaths = { "/path1/firstLine", "../path2/firstLine", "/path3/secondLine", "/path1/thirdLine", "../path2/thirdLine", "/path3/fourthLine" };
	for( const auto& p : expectedPaths )
	{
		EXPECT_TRUE( resolved.count( p ) );
	}

	for( const auto& kv : resolved )
	{
		if( kv.first == "/path1/firstLine" || kv.first == "../path2/firstLine" )
		{
			// Add .inLine1 to include and move .parIn1 from include to exclude filter
			EXPECT_EQ( kv.second.GetIncludeFilter(), std::vector<std::string>( { ".parIn2", ".inLine1" } ) );
			EXPECT_EQ( kv.second.GetExcludeFilter(), std::vector<std::string>( { ".parEx1", ".parIn1" } ) );
		}
		else if( kv.first == "/path3/secondLine" )
		{
			// Keep parent filters unchanged
			EXPECT_EQ( kv.second.GetIncludeFilter(), std::vector<std::string>( { ".parIn1", ".parIn2" } ) );
			EXPECT_EQ( kv.second.GetExcludeFilter(), std::vector<std::string>( { ".parEx1" } ) );
		}
		else if( kv.first == "/path1/thirdLine" || kv.first == "../path2/thirdLine" )
		{
			// Add .exLine3 to exclude filter and move .parEx1 from exclude to include
			EXPECT_EQ( kv.second.GetIncludeFilter(), std::vector<std::string>( { ".parIn1", ".parIn2", ".parEx1" } ) );
			EXPECT_EQ( kv.second.GetExcludeFilter(), std::vector<std::string>( { ".exLine3" } ) );
		}
		else if( kv.first == "/path3/fourthLine" )
		{
			// Add .inLine4 to include and .exLine4 to exclude filter
			EXPECT_EQ( kv.second.GetIncludeFilter(), std::vector<std::string>( { ".parIn1", ".parIn2", ".inLine4" } ) );
			EXPECT_EQ( kv.second.GetExcludeFilter(), std::vector<std::string>( { ".parEx1", ".exLine4" } ) );
		}
	}
}

TEST_F( ResourceFilterTest, FilterResourcePathFile_SingleLine_DuplicateOverrides )
{
	std::string prefixMapStr = "prefix1:/path1";
	std::string parentFilterStr = "[ .parIn1 .parIn2 ] ![ .parEx1 ]";
	ResourceTools::FilterPrefixMap prefixMap( prefixMapStr );
	ResourceTools::FilterResourceFilter parentFilter( parentFilterStr );

	// Make sure we DUPLICATE the inline with the same as the parents (should NOT result in combined duplicates)
	std::string rawPathFileAttrib = "prefix1:/foo/bar [ .parIn2 .parIn1 .lineIn1 .parIn1 .parIn2 ] ![ .lineEx1 .parEx1 ]";
	ResourceTools::FilterResourcePathFile pathFile( rawPathFileAttrib, prefixMap, parentFilter );
	const auto& resolvedPathMap = pathFile.GetResolvedPathMap();

	// Check the resolved path and filters against expected (NO duplicates in final filters list)
	std::set<std::string> expectedPaths = { "/path1/foo/bar" };
	std::vector<std::string> expectedIncludes = { ".parIn1", ".parIn2", ".lineIn1" };
	std::vector<std::string> expectedExcludes = { ".parEx1", ".lineEx1" };

	for( const auto& p : expectedPaths )
	{
		EXPECT_TRUE( resolvedPathMap.count( p ) );
	}

	for( const auto& kv : resolvedPathMap )
	{
		EXPECT_EQ( kv.second.GetIncludeFilter(), expectedIncludes );
		EXPECT_EQ( kv.second.GetExcludeFilter(), expectedExcludes );
	}
}

TEST_F( ResourceFilterTest, FilterResourcePathFile_Invalid_MissingPrefix )
{
	std::string prefixMapStr = "prefix1:/path1;../path2 prefix2:/path3";
	std::string parentFilterStr = "[ .in1 .in2 ] ![ .ex1 ]";
	ResourceTools::FilterPrefixMap prefixMap( prefixMapStr );
	ResourceTools::FilterResourceFilter parentFilter( parentFilterStr );

	std::string rawPathFileAttrib = "/foo/bar"; // respath is missing the prefix:
	EXPECT_THROW( ResourceTools::FilterResourcePathFile pathFile( rawPathFileAttrib, prefixMap, parentFilter ), std::invalid_argument );
}

TEST_F( ResourceFilterTest, FilterResourcePathFile_Invalid_UnknownPrefix )
{
	std::string prefixMapStr = "prefix1:/path1;../path2 prefix2:/path3";
	std::string parentFilterStr = "[ .in1 .in2 ] ![ .ex1 ]";
	ResourceTools::FilterPrefixMap prefixMap( prefixMapStr );
	ResourceTools::FilterResourceFilter parentFilter( parentFilterStr );

	std::string rawPathFileAttrib = "prefixNotInPrefixMap:/foo/bar"; // unknown prefix
	EXPECT_THROW( ResourceTools::FilterResourcePathFile pathFile( rawPathFileAttrib, prefixMap, parentFilter ), std::invalid_argument );
}

TEST_F( ResourceFilterTest, FilterResourcePathFile_Invalid_MalformedInlineFilter )
{
	std::string prefixMapStr = "prefix1:/path1;../path2 prefix2:/path3";
	std::string parentFilterStr = "[ .in1 .in2 ] ![ .ex1 ]";
	ResourceTools::FilterPrefixMap prefixMap( prefixMapStr );
	ResourceTools::FilterResourceFilter parentFilter( parentFilterStr );

	std::string rawPathFileAttrib = "prefix1:/foo/bar [ .yaml "; // missing closing bracket of inline include filter
	EXPECT_THROW( ResourceTools::FilterResourcePathFile pathFile( rawPathFileAttrib, prefixMap, parentFilter ), std::invalid_argument );
}

// -----------------------------------------
