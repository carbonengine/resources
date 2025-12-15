// Copyright © 2025 CCP ehf.

#include "ResourceFilterTest.h"

#include <INIReader.h>
#include <FilterResourceFilter.h>
#include <FilterPrefixMap.h>
#include <FilterPrefixMapEntry.h>
#include <FilterDefaultSection.h>

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

TEST_F ( ResourceFilterTest, FilterPrefixMapEntry_PrefixMismatchOnAppend )
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

TEST_F ( ResourceFilterTest, FilterPrefixMapEntry_InvalidNoPathsOnAppend )
{
	try
	{
		ResourceTools::FilterPrefixMapEntry entry( "prefix1", "" );  // Empty string for paths
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

TEST_F ( ResourceFilterTest, FilterDefaultSection_InitializeValid )
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