// Copyright © 2025 CCP ehf.

#include "ResourceFilterTest.h"

#include <INIReader.h>
#include <FilterResourceFilter.h>
#include <FilterPrefixMap.h>
#include <FilterPrefixMapEntry.h>
#include <FilterDefaultSection.h>
#include <FilterResourcePathFile.h>
#include <FilterNamedSection.h>
#include <FilterResourceFile.h>

TEST_F( ResourceFilterTest, Example1IniParsing )
{
	// Use the test fixture's helper to get the absolute path
	const std::filesystem::path iniPath = GetTestFileFileAbsolutePath( "ExampleIniFiles/example1.ini" );
	INIReader reader( iniPath.string() );
	ASSERT_EQ( reader.ParseError(), 0 ) << "Failed to parse example1.ini";

	// There should only be 2 sections
	EXPECT_EQ( reader.Sections().size(), 2 );

	// Check [default] section
	ASSERT_TRUE( reader.HasSection( "DEFAULT" ) );
	EXPECT_EQ( reader.Get( "DEFAULT", "prefixmap", "" ), "res:./Indicies;./resourcesOnBranch res2:./ResourceGroups" );
	EXPECT_EQ( reader.Get( "DEFAULT", "version", "" ), "1.2" );
	EXPECT_EQ( reader.Keys( "DEFAULT" ).size(), 2 );

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

TEST_F( ResourceFilterTest, FilterResourceFilter_OnlyIncludeFilter )
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

TEST_F( ResourceFilterTest, FilterResourceFilter_OnlyExcludeFilter_Toplevel )
{
	ResourceTools::FilterResourceFilter filter( "![ .excluded .extension ]", true );
	const auto& includes = filter.GetIncludeFilter();
	const auto& excludes = filter.GetExcludeFilter();

	EXPECT_EQ( excludes.size(), 2 );
	EXPECT_EQ( excludes[0], ".excluded" );
	EXPECT_EQ( excludes[1], ".extension" );

	EXPECT_EQ( includes.size(), 1 );
	EXPECT_EQ( includes[0], "*" ); // Wild-card added when no include filter specified for TOP-LEVEL filter
}

TEST_F( ResourceFilterTest, FilterResourceFilter_OnlyExcludeFilter_Inline )
{
	ResourceTools::FilterResourceFilter filter( "![ .excluded .extension ]" );
	const auto& includes = filter.GetIncludeFilter();
	const auto& excludes = filter.GetExcludeFilter();

	EXPECT_EQ( excludes.size(), 2 );
	EXPECT_EQ( excludes[0], ".excluded" );
	EXPECT_EQ( excludes[1], ".extension" );

	EXPECT_EQ( includes.size(), 0 );
	EXPECT_TRUE( includes.empty() ); // No wild-card added when no include filter specified INLINE
}

TEST_F( ResourceFilterTest, FilterResourceFilter_ComplexIncludeExcludeFilter )
{
	ResourceTools::FilterResourceFilter filter( "[ .red .gr2 .dds .png .yaml ] [ .txt ] ![ .csv .xls ] [ .bat .sh ] ![ .blk .yel ]" );
	const auto& includes = filter.GetIncludeFilter();
	const auto& excludes = filter.GetExcludeFilter();
	std::vector<std::string> expectedIncludes = { ".red", ".gr2", ".dds", ".png", ".yaml", ".txt", ".bat", ".sh" };
	std::vector<std::string> expectedExcludes = { ".csv", ".xls", ".blk", ".yel" };
	EXPECT_EQ( includes, expectedIncludes );
	EXPECT_EQ( excludes, expectedExcludes );
}

TEST_F( ResourceFilterTest, FilterResourceFilter_SimpleIncludeFilter )
{
	ResourceTools::FilterResourceFilter filter( "[ .red ]" );
	const auto& includes = filter.GetIncludeFilter();
	EXPECT_EQ( includes.size(), 1 );
	EXPECT_EQ( includes[0], ".red" );
}

TEST_F( ResourceFilterTest, FilterResourceFilter_SimpleExcludeFilter )
{
	ResourceTools::FilterResourceFilter filter( "![ .blk ]" );
	const auto& excludes = filter.GetExcludeFilter();
	EXPECT_EQ( excludes.size(), 1 );
	EXPECT_EQ( excludes[0], ".blk" );
}

TEST_F( ResourceFilterTest, FilterResourceFilter_IncludeExcludeInclude )
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

TEST_F( ResourceFilterTest, FilterResourceFilter_MissingClosingIncludeBracketBeforeNextOpenExcludeBracket )
{
	try
	{
		// This test filter has a missing closing bracket for the first include filter, before the next exclude filter starts
		ResourceTools::FilterResourceFilter filter( "[ .in1   !  [ .ex1 ]" );
		FAIL() << "Expected std::invalid_argument to be thrown";
	}
	catch( const std::invalid_argument& e )
	{
		EXPECT_STREQ( e.what(), "Invalid filter format: matching end bracket ']' not present before the next start bracket '['" );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument when constructing FilterResourceFilter";
	}
}

TEST_F( ResourceFilterTest, FilterResourceFilter_ExcludeMarkerWithoutBracket_v1 )
{
	try
	{
		ResourceTools::FilterResourceFilter filter( "! .ex1 ]" );
		FAIL() << "Expected std::invalid_argument to be thrown";
	}
	catch( const std::invalid_argument& e )
	{
		EXPECT_STREQ( e.what(), "Invalid filter format: missing '['" );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument when constructing FilterResourceFilter";
	}
}

TEST_F( ResourceFilterTest, FilterResourceFilter_ExcludeMarkerWithoutBracket_v2 )
{
	try
	{
		ResourceTools::FilterResourceFilter filter( "  [ .in1 ] ! .ex1 ]" );
		FAIL() << "Expected std::invalid_argument to be thrown";
	}
	catch( const std::invalid_argument& e )
	{
		EXPECT_STREQ( e.what(), "Invalid filter format: missing '['" );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument when constructing FilterResourceFilter";
	}
}

TEST_F( ResourceFilterTest, FilterResourceFilter_ExcludeMarkerWithoutBracket_v3 )
{
	try
	{
		ResourceTools::FilterResourceFilter filter( "  [ .in1 ] ![ .ex1 ] ! " );
		FAIL() << "Expected std::invalid_argument to be thrown";
	}
	catch( const std::invalid_argument& e )
	{
		EXPECT_STREQ( e.what(), "Invalid filter format: exclude filter marker found without a [ token ] section" );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument when constructing FilterResourceFilter";
	}
}

TEST_F( ResourceFilterTest, FilterResourceFilter_MissingOpeningBracket_v1 )
{
	try
	{
		ResourceTools::FilterResourceFilter filter( ".in1 .in2 ]" );
		FAIL() << "Expected std::invalid_argument to be thrown";
	}
	catch( const std::invalid_argument& e )
	{
		EXPECT_STREQ( e.what(), "Invalid filter format: missing '['" );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument when constructing FilterResourceFilter";
	}
}

TEST_F( ResourceFilterTest, FilterResourceFilter_MissingOpeningBracket_v2 )
{
	try
	{
		ResourceTools::FilterResourceFilter filter( " [ .in1 .in2 ] .in3 ] " );
		FAIL() << "Expected std::invalid_argument to be thrown";
	}
	catch( const std::invalid_argument& e )
	{
		EXPECT_STREQ( e.what(), "Invalid filter format: missing '['" );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument when constructing FilterResourceFilter";
	}
}

TEST_F( ResourceFilterTest, FilterResourceFilter_MissingClosingBracket_v1 )
{
	try
	{
		ResourceTools::FilterResourceFilter filter( "[ .in1 .in2 " );
		FAIL() << "Expected std::invalid_argument to be thrown";
	}
	catch( const std::invalid_argument& e )
	{
		EXPECT_STREQ( e.what(), "Invalid filter format: missing ']'" );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument when constructing FilterResourceFilter";
	}
}

TEST_F( ResourceFilterTest, FilterResourceFilter_MissingClosingBracket_v2 )
{
	try
	{
		ResourceTools::FilterResourceFilter filter( "[ .in1 .in2 ] [ .in3 " );
		FAIL() << "Expected std::invalid_argument to be thrown";
	}
	catch( const std::invalid_argument& e )
	{
		EXPECT_STREQ( e.what(), "Invalid filter format: missing ']'" );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument when constructing FilterResourceFilter";
	}
}

TEST_F( ResourceFilterTest, FilterResourceFilter_MissingClosingBracket_v3 )
{
	try
	{
		ResourceTools::FilterResourceFilter filter( "[ .in1 .in2 ] [ .in3 [ .in4 ]" );
		FAIL() << "Expected std::invalid_argument to be thrown";
	}
	catch( const std::invalid_argument& e )
	{
		EXPECT_STREQ( e.what(), "Invalid filter format: matching end bracket ']' not present before the next start bracket '['" );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument when constructing FilterResourceFilter";
	}
}

TEST_F( ResourceFilterTest, FilterResourceFilter_CondensedValidFilterStringv1 )
{
	ResourceTools::FilterResourceFilter filter( "[inToken1 inToken2]![exToken1 exToken2][inToken3]" );
	const auto& includes = filter.GetIncludeFilter();
	const auto& excludes = filter.GetExcludeFilter();
	std::vector<std::string> expectedIncludes = { "inToken1", "inToken2", "inToken3" };
	std::vector<std::string> expectedExcludes = { "exToken1", "exToken2" };
	EXPECT_EQ( includes, expectedIncludes );
	EXPECT_EQ( excludes, expectedExcludes );
}

TEST_F( ResourceFilterTest, FilterResourceFilter_CondensedValidFilterStringv2 )
{
	ResourceTools::FilterResourceFilter filter( "![exToken1][inToken1 inToken2]![exToken2][inToken3]" );
	const auto& includes = filter.GetIncludeFilter();
	const auto& excludes = filter.GetExcludeFilter();
	std::vector<std::string> expectedIncludes = { "inToken1", "inToken2", "inToken3" };
	std::vector<std::string> expectedExcludes = { "exToken1", "exToken2" };
	EXPECT_EQ( includes, expectedIncludes );
	EXPECT_EQ( excludes, expectedExcludes );
}

TEST_F( ResourceFilterTest, FilterResourceFilter_EmptyFilterString_TopLevel )
{
	ResourceTools::FilterResourceFilter filter( "", true );
	const auto& includes = filter.GetIncludeFilter();
	const auto& excludes = filter.GetExcludeFilter();

	EXPECT_EQ( includes.size(), 1 );
	EXPECT_EQ( includes[0], "*" ); // Wild-card added when no include filter specified on TOP-LEVEL filter

	EXPECT_TRUE( excludes.empty() );
}

TEST_F( ResourceFilterTest, FilterResourceFilter_EmptyFilterString_Inline )
{
	ResourceTools::FilterResourceFilter filter( "" );
	const auto& includes = filter.GetIncludeFilter();
	const auto& excludes = filter.GetExcludeFilter();

	EXPECT_EQ( includes.size(), 0 );
	EXPECT_TRUE( includes.empty() ); // No wild-card added when no include filter specified INLINE

	EXPECT_TRUE( excludes.empty() );
}

// -----------------------------------------

TEST_F( ResourceFilterTest, FilterPrefixMap_SinglePrefixMultiplePaths )
{
	ResourceTools::FilterPrefixMap map( "prefix1:/somePath;../otherPath" );
	const auto& prefixMapEntries = map.GetMapEntries();
	ASSERT_EQ( prefixMapEntries.size(), 1 ) << "There should only be 1 prefix in the map";

	// If iterator is at end, the prefix was not found
	auto it = prefixMapEntries.find( "prefix1" );
	ASSERT_NE( it, prefixMapEntries.end() ) << "Prefix 'prefix1' not found in the map";

	std::set<std::string> expected = { "/somePath", "../otherPath" };
	EXPECT_EQ( it->second.GetPrefix(), "prefix1" ) << "Prefix should be 'prefix1'";
	EXPECT_EQ( it->first, it->second.GetPrefix() ) << "Value of FilterPrefixMap.m_prefixMap key does not match associated FilterPrefixMapEntry.m_prefix";
	EXPECT_EQ( it->second.GetPaths(), expected ) << "Paths do not match expected values";
}

TEST_F( ResourceFilterTest, FilterPrefixMap_MultiplePrefixes )
{
	ResourceTools::FilterPrefixMap map( "prefix1:/path1;/path2 prefix2:/newPath1" );
	const auto& prefixMapEntries = map.GetMapEntries();
	ASSERT_EQ( prefixMapEntries.size(), 2 ) << "There should be 2 prefixes in the map";

	// Make sure both prefixes exist
	auto it1 = prefixMapEntries.find( "prefix1" );
	auto it2 = prefixMapEntries.find( "prefix2" );
	ASSERT_NE( it1, prefixMapEntries.end() ) << "Prefix 'prefix1' not found in the map";
	ASSERT_NE( it2, prefixMapEntries.end() ) << "Prefix 'prefix2' not found in the map";

	std::set<std::string> expected1 = { "/path1", "/path2" };
	std::set<std::string> expected2 = { "/newPath1" };
	EXPECT_EQ( it1->second.GetPrefix(), "prefix1" ) << "Prefix should be 'prefix1'";
	EXPECT_EQ( it2->second.GetPrefix(), "prefix2" ) << "Prefix should be 'prefix2'";
	EXPECT_EQ( it1->first, it1->second.GetPrefix() ) << "Value of FilterPrefixMap.m_prefixMap key does not match associated FilterPrefixMapEntry.m_prefix";
	EXPECT_EQ( it2->first, it2->second.GetPrefix() ) << "Value of FilterPrefixMap.m_prefixMap key does not match associated FilterPrefixMapEntry.m_prefix";
	EXPECT_EQ( it1->second.GetPaths(), expected1 ) << "Paths do not match expected values";
	EXPECT_EQ( it2->second.GetPaths(), expected2 ) << "Paths do not match expected values";
}

TEST_F( ResourceFilterTest, FilterPrefixMap_DuplicateSamePrefixPathsInDifferentOrder )
{
	ResourceTools::FilterPrefixMap map( "prefix1:/path1;/path2 prefix1:/path2;/path1" );

	// There should only be one prefix (prefix1)
	const auto& prefixMapEntries = map.GetMapEntries();
	ASSERT_EQ( prefixMapEntries.size(), 1 ) << "There should only be 1 prefix in the map";
	auto it = prefixMapEntries.find( "prefix1" );
	ASSERT_NE( it, prefixMapEntries.end() ) << "Prefix 'prefix1' not found in the map";

	// There should be only 2 paths, sorted in set
	std::set<std::string> expected_a = { "/path1", "/path2" };
	std::set<std::string> expected_b = { "/path2", "/path1" };
	EXPECT_EQ( it->second.GetPrefix(), "prefix1" ) << "Prefix should be 'prefix1'";
	EXPECT_EQ( it->second.GetPaths(), expected_a ) << "Paths do not match expected values - inserted in alphabetical order";
	EXPECT_EQ( it->second.GetPaths(), expected_b ) << "Paths do not match expected values - inserted in reverse alphabetical order";
	EXPECT_EQ( it->first, it->second.GetPrefix() ) << "Value of FilterPrefixMap.m_prefixMap key does not match associated FilterPrefixMapEntry.m_prefix";
}

TEST_F( ResourceFilterTest, FilterPrefixMap_MultiplePrefixesAppendToPaths )
{
	ResourceTools::FilterPrefixMap map( "prefix1:/path2;/path1 prefix2:/otherPath1;/otherPath2 prefix1:/path3;/path1" );
	const auto& prefixMapEntries = map.GetMapEntries();
	ASSERT_EQ( prefixMapEntries.size(), 2 ) << "There should be 2 prefixes in the map";
	auto it1 = prefixMapEntries.find( "prefix1" );
	auto it2 = prefixMapEntries.find( "prefix2" );
	ASSERT_NE( it1, prefixMapEntries.end() ) << "Prefix 'prefix1' not found in the map";
	ASSERT_NE( it2, prefixMapEntries.end() ) << "Prefix 'prefix2' not found in the map";

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

TEST_F( ResourceFilterTest, FilterPrefixMap_DifferentWhitespacesBetweenPrefixes )
{
	std::string input = "prefix1:/path1\tprefixTab:/path2\nprefixNewLine:/path3";
	ResourceTools::FilterPrefixMap map( input );
	const auto& prefixMapEntries = map.GetMapEntries();
	ASSERT_EQ( prefixMapEntries.size(), 3 ) << "There should only be 3 prefix in the map";
	auto it1 = prefixMapEntries.find( "prefix1" );
	auto it2 = prefixMapEntries.find( "prefixTab" );
	auto it3 = prefixMapEntries.find( "prefixNewLine" );
	EXPECT_NE( it1, prefixMapEntries.end() ) << "Prefix 'prefix1' not found in the map";
	EXPECT_NE( it2, prefixMapEntries.end() ) << "Prefix 'prefixTab' not found in the map";
	EXPECT_NE( it3, prefixMapEntries.end() ) << "Prefix 'prefixNewLine' not found in the map";

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

TEST_F( ResourceFilterTest, FilterPrefixMap_Invalid_MissingColon )
{
	try
	{
		ResourceTools::FilterPrefixMap prefixmap( "prefix1/path1" );
		FAIL() << "Expected std::invalid_argument to be thrown";
	}
	catch( const std::invalid_argument& e )
	{
		EXPECT_STREQ( e.what(), "Invalid prefixmap format: missing ':'" );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument when constructing FilterPrefixMap";
	}
}

TEST_F( ResourceFilterTest, FilterPrefixMap_Invalid_EmptyPrefix )
{
	try
	{
		ResourceTools::FilterPrefixMap prefixmap( ":/path1" );
		FAIL() << "Expected std::invalid_argument to be thrown";
	}
	catch( const std::invalid_argument& e )
	{
		EXPECT_STREQ( e.what(), "Invalid prefixmap format: empty prefix" );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument when constructing FilterPrefixMap";
	}
}

TEST_F( ResourceFilterTest, FilterPrefixMap_Invalid_NoPaths )
{
	try
	{
		ResourceTools::FilterPrefixMap prefixmap( "prefix1:" );
		FAIL() << "Expected std::invalid_argument to be thrown";
	}
	catch( const std::invalid_argument& e )
	{
		EXPECT_STREQ( e.what(), "Invalid prefixmap format: No paths defined for prefix: prefix1" );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument when constructing FilterPrefixMap";
	}
}

TEST_F( ResourceFilterTest, FilterPrefixMapEntry_PrefixMismatchOnAppend )
{
	try
	{
		ResourceTools::FilterPrefixMapEntry entry( "prefix1", "/path1" );
		entry.AppendPaths( "prefix2", "/path2" );
		FAIL() << "Expected std::invalid_argument to be thrown";
	}
	catch( const std::invalid_argument& e )
	{
		EXPECT_STREQ( e.what(), "Prefix mismatch while appending path(s): prefix2 (incoming) != prefix1 (existing)" );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument when appending paths to FilterPrefixMapEntry";
	}
}

TEST_F( ResourceFilterTest, FilterPrefixMapEntry_InvalidNoPathsOnAppend )
{
	try
	{
		ResourceTools::FilterPrefixMapEntry entry( "prefix1", "" ); // Empty string for paths
		FAIL() << "Expected std::invalid_argument to be thrown";
	}
	catch( const std::invalid_argument& e )
	{
		EXPECT_STREQ( e.what(), "Invalid prefixmap format: No paths appended for prefix: prefix1" );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument when constructing FilterPrefixMapEntry with no paths";
	}
}

// -----------------------------------------

TEST_F( ResourceFilterTest, FilterDefaultSection_InitializeValid )
{
	std::string input = "prefix1:/path1;../path2 prefix2:/path3";
	ResourceTools::FilterDefaultSection defaultSection( input );
	const auto& prefixMapEntries = defaultSection.GetPrefixMap().GetMapEntries();
	ASSERT_EQ( prefixMapEntries.size(), 2 ) << "There should be 2 prefixes in the map";
	auto it1 = prefixMapEntries.find( "prefix1" );
	auto it2 = prefixMapEntries.find( "prefix2" );
	ASSERT_NE( it1, prefixMapEntries.end() ) << "Prefix 'prefix1' not found in the map";
	ASSERT_NE( it2, prefixMapEntries.end() ) << "Prefix 'prefix2' not found in the map";

	std::set<std::string> prefix1Paths = { "/path1", "../path2" };
	std::set<std::string> prefix2Paths = { "/path3" };
	EXPECT_EQ( it1->second.GetPrefix(), "prefix1" ) << "Prefix should be 'prefix1'";
	EXPECT_EQ( it2->second.GetPrefix(), "prefix2" ) << "Prefix should be 'prefix2'";
	EXPECT_EQ( it1->second.GetPaths(), prefix1Paths ) << "Paths do not match expected values";
	EXPECT_EQ( it2->second.GetPaths(), prefix2Paths ) << "Paths do not match expected values";
	EXPECT_EQ( it1->first, it1->second.GetPrefix() ) << "Value of FilterPrefixMap.m_prefixMap key does not match associated FilterPrefixMapEntry.m_prefix";
	EXPECT_EQ( it2->first, it2->second.GetPrefix() ) << "Value of FilterPrefixMap.m_prefixMap key does not match associated FilterPrefixMapEntry.m_prefix";
}

TEST_F( ResourceFilterTest, FilterDefaultSection_Initialize_InvalidMissingColon )
{
	try
	{
		ResourceTools::FilterDefaultSection defaultSection( "prefix1/path1" );
		FAIL() << "Expected std::invalid_argument to be thrown";
	}
	catch( const std::invalid_argument& e )
	{
		EXPECT_STREQ( e.what(), "Invalid prefixmap format: missing ':'" );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument when constructing FilterDefaultSection";
	}
}

TEST_F( ResourceFilterTest, FilterDefaultSection_Initialize_InvalidEmptyPrefix )
{
	try
	{
		ResourceTools::FilterDefaultSection defaultSection( ":/path1" );
		FAIL() << "Expected std::invalid_argument to be thrown";
	}
	catch( const std::invalid_argument& e )
	{
		EXPECT_STREQ( e.what(), "Invalid prefixmap format: empty prefix" );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument when constructing FilterDefaultSection";
	}
}

// -----------------------------------------

void MapContainsPaths( const std::set<std::string>& allExpectedPaths,
					   const std::map<std::string, ResourceTools::FilterResourceFilter>& resolvedMap,
					   const std::string messagePrefix = "" )
{
	for( const auto& path : allExpectedPaths )
	{
		EXPECT_TRUE( resolvedMap.find( path ) != resolvedMap.end() ) << messagePrefix << " - Expected path not found in resolved map: " << path;
	}
	if( resolvedMap.size() != allExpectedPaths.size() )
	{
		FAIL() << messagePrefix << " - Resolved map size (" << resolvedMap.size() << ") does not match expected paths size (" << allExpectedPaths.size() << ")";
	}
}

void ValidatePathMap( const std::set<std::string>& expectedPaths,
					  const std::map<std::string, ResourceTools::FilterResourceFilter>& resolvedPathMap,
					  const std::vector<std::string>& expectedIncludes,
					  const std::vector<std::string>& expectedExcludes,
					  const std::string messagePrefix = "" )
{
	for( const auto& p : expectedPaths )
	{
		EXPECT_TRUE( resolvedPathMap.count( p ) ) << messagePrefix << " - Expected path not found in resolved path map: " << p;
	}

	for( const auto& kv : resolvedPathMap )
	{
		// Ignore resolved paths that are not in the expectedPaths set (useful when checking partial expectedPaths, because of include/exclude overrides from default)
		if( expectedPaths.find( kv.first ) == expectedPaths.end() )
		{
			continue;
		}
		EXPECT_EQ( kv.second.GetIncludeFilter(), expectedIncludes ) << messagePrefix << " - Include filter does not match expected for path: " << kv.first;
		EXPECT_EQ( kv.second.GetExcludeFilter(), expectedExcludes ) << messagePrefix << " - Exclude filter does not match expected for path: " << kv.first;
	}
}


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

	ValidatePathMap( expectedPaths, resolvedPathMap, expectedIncludes, expectedExcludes );
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

	ValidatePathMap( expectedPaths, resolvedPathMap, expectedIncludes, expectedExcludes );
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

	ValidatePathMap( expectedPaths, resolvedPathMap, expectedIncludes, expectedExcludes );
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

	ValidatePathMap( expectedPaths, resolvedPathMap, expectedIncludes, expectedExcludes );
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

TEST_F( ResourceFilterTest, FilterNamedSection_Valid_SingleLineRespath )
{
	std::string sectionName = "FilterNamedSection_Valid_SingleLineRespath";
	std::string filter = "[ .in1 .in2 ] ![ .ex1 ]";
	std::string respaths = "testPrefix:/foo/bar";
	std::string defaultParentPrefixMapStr = "testPrefix:/myPath";

	ResourceTools::FilterPrefixMap defaultPrefixMap( defaultParentPrefixMapStr );
	ResourceTools::FilterNamedSection namedSection( sectionName, filter, respaths, "", defaultPrefixMap );

	// Expected values:
	std::set<std::string> expectedPaths = { "/myPath/foo/bar" };
	std::vector<std::string> expectedIncludes = { ".in1", ".in2" };
	std::vector<std::string> expectedExcludes = { ".ex1" };

	const auto& resolvedRespathMap = namedSection.GetResolvedRespathsMap();
	const auto& resolvedResfileMap = namedSection.GetResolvedResfileMap();
	const auto& combinedMap = namedSection.GetCombinedResolvedPathMap();

	ValidatePathMap( expectedPaths, resolvedRespathMap, expectedIncludes, expectedExcludes, "ResolvedRespathsMap" );
	EXPECT_TRUE( resolvedResfileMap.empty() );
	ValidatePathMap( expectedPaths, combinedMap, expectedIncludes, expectedExcludes, "CombinedResolvedPathMap" );
}

TEST_F( ResourceFilterTest, FilterNamedSection_Valid_EmptyFilter_TopLevel )
{
	std::string sectionName = "FilterNamedSection_Valid_EmptyFilter_TopLevel";
	std::string defaultParentPrefixMapStr = "testPrefix:/myPath";
	std::string filter = ""; // Empty filter string at top-level should add wildcard include
	std::string respaths = "testPrefix:/foo/bar";

	ResourceTools::FilterPrefixMap defaultPrefixMap( defaultParentPrefixMapStr );
	ResourceTools::FilterNamedSection namedSection( sectionName, filter, respaths, "", defaultPrefixMap );

	// Expected values:
	std::set<std::string> expectedPaths = { "/myPath/foo/bar" };
	std::vector<std::string> expectedIncludes = { "*" };
	std::vector<std::string> expectedExcludes = { };

	const auto& resolvedRespathMap = namedSection.GetResolvedRespathsMap();
	const auto& resolvedResfileMap = namedSection.GetResolvedResfileMap();
	const auto& combinedMap = namedSection.GetCombinedResolvedPathMap();

	ValidatePathMap( expectedPaths, resolvedRespathMap, expectedIncludes, expectedExcludes, "ResolvedRespathsMap" );
	EXPECT_TRUE( resolvedResfileMap.empty() );
	ValidatePathMap( expectedPaths, combinedMap, expectedIncludes, expectedExcludes, "CombinedResolvedPathMap" );
}

TEST_F( ResourceFilterTest, FilterNamedSection_Valid_OnlyExcludeFilter_TopLevel )
{
	std::string sectionName = "FilterNamedSection_Valid_OnlyExcludeFilter_TopLevel";
	std::string defaultParentPrefixMapStr = "testPrefix:/myPath";
	std::string filter = "![ .ex1 ]"; // When there is only an exclude filter at top-level, it should add wildcard include
	std::string respaths = "testPrefix:/foo/bar";

	ResourceTools::FilterPrefixMap defaultPrefixMap( defaultParentPrefixMapStr );
	ResourceTools::FilterNamedSection namedSection( sectionName, filter, respaths, "", defaultPrefixMap );

	// Expected values:
	std::set<std::string> expectedPaths = { "/myPath/foo/bar" };
	std::vector<std::string> expectedIncludes = { "*" };
	std::vector<std::string> expectedExcludes = { ".ex1" };

	const auto& resolvedRespathMap = namedSection.GetResolvedRespathsMap();
	const auto& resolvedResfileMap = namedSection.GetResolvedResfileMap();
	const auto& combinedMap = namedSection.GetCombinedResolvedPathMap();

	ValidatePathMap( expectedPaths, resolvedRespathMap, expectedIncludes, expectedExcludes, "ResolvedRespathsMap" );
	EXPECT_TRUE( resolvedResfileMap.empty() );
	ValidatePathMap( expectedPaths, combinedMap, expectedIncludes, expectedExcludes, "CombinedResolvedPathMap" );
}

TEST_F( ResourceFilterTest, FilterNamedSection_Valid_MultiLineRespath )
{
	std::string sectionName = "FilterNamedSection_Valid_MultiLineRespath";
	std::string filter = "[ .in1 .in2 ] ![ .ex1 ]";
	std::string respaths =
		"prefix1:/firstLine [ .inLine1 ] ![ .exLine1 ]\n" // Add entries to both include and exclude filters
		"prefix2:/secondLine";                            // Just using vanilla parent filter
	std::string defaultParentPrefixMapStr = "prefix1:/pathA;/pathB prefix2:/path2";

	ResourceTools::FilterPrefixMap defaultPrefixMap( defaultParentPrefixMapStr );
	ResourceTools::FilterNamedSection namedSection( sectionName, filter, respaths, "", defaultPrefixMap );

	// Expected values:
	std::set<std::string> allExpectedPaths = { "/pathA/firstLine", "/pathB/firstLine", "/path2/secondLine" };
	std::set<std::string> firstLinePaths = { "/pathA/firstLine", "/pathB/firstLine" };
	std::set<std::string> secondLinePaths = { "/path2/secondLine" };
	std::vector<std::string> defaultIncludes = { ".in1", ".in2" };
	std::vector<std::string> defaultExcludes = { ".ex1" };
	std::vector<std::string> firstLineIncludes = { ".in1", ".in2", ".inLine1" };
	std::vector<std::string> firstLineExcludes = { ".ex1", ".exLine1" };

	const auto& resolvedRespathsMap = namedSection.GetResolvedRespathsMap();
	const auto& resolvedResfileMap = namedSection.GetResolvedResfileMap();
	const auto& combinedMap = namedSection.GetCombinedResolvedPathMap();

	MapContainsPaths( allExpectedPaths, resolvedRespathsMap, "ResolvedRespathsMap" );
	ValidatePathMap( firstLinePaths, resolvedRespathsMap, firstLineIncludes, firstLineExcludes, "FirstLine ResolvedRespathsMap" );
	ValidatePathMap( secondLinePaths, resolvedRespathsMap, defaultIncludes, defaultExcludes, "SecondLine ResolvedRespathsMap" );
	EXPECT_TRUE( resolvedResfileMap.empty() );
	MapContainsPaths( allExpectedPaths, combinedMap, "CombinedResolvedMap" );
	ValidatePathMap( firstLinePaths, combinedMap, firstLineIncludes, firstLineExcludes, "FirstLine ResolvedRespathsMap" );
	ValidatePathMap( secondLinePaths, combinedMap, defaultIncludes, defaultExcludes, "SecondLine ResolvedRespathsMap" );
}

TEST_F( ResourceFilterTest, FilterNamedSection_Valid_RespathAndResfile )
{
	std::string sectionName = "FilterNamedSection_Valid_RespathAndResfile";
	std::string defaultParentPrefixMapStr = "prefix1:/pathA;/pathB prefix2:/pathC";
	std::string filter = "[ .in1 .in2 ] ![ .ex1 ]";
	std::string respaths = "prefix1:/respaths";
	std::string resfile = "prefix2:/resfile";

	ResourceTools::FilterPrefixMap defaultPrefixMap( defaultParentPrefixMapStr );
	ResourceTools::FilterNamedSection namedSection( sectionName, filter, respaths, resfile, defaultPrefixMap );

	// Expected values:
	std::set<std::string> allExpectedPaths = { "/pathA/respaths", "/pathB/respaths", "/pathC/resfile" };
	std::set<std::string> respathsPaths = { "/pathA/respaths", "/pathB/respaths" };
	std::set<std::string> resfilesPaths = { "/pathC/resfile" };
	std::vector<std::string> defaultIncludes = { ".in1", ".in2" };
	std::vector<std::string> defaultExcludes = { ".ex1" };

	const auto& respathsMap = namedSection.GetResolvedRespathsMap();
	const auto& resfileMap = namedSection.GetResolvedResfileMap();
	const auto& combinedMap = namedSection.GetCombinedResolvedPathMap();

	ASSERT_EQ( respathsMap.size(), 2 ); // prefix1 has two paths
	MapContainsPaths( respathsPaths, respathsMap, "ResolvedRespathsMap" );
	ValidatePathMap( respathsPaths, respathsMap, defaultIncludes, defaultExcludes, "ResolvedRespathsMap" );

	ASSERT_EQ( resfileMap.size(), 1 ); // prefix2 has one path
	MapContainsPaths( resfilesPaths, resfileMap, "ResolvedResfileMap" );
	ValidatePathMap( resfilesPaths, resfileMap, defaultIncludes, defaultExcludes, "ResolvedResfileMap" );

	ASSERT_EQ( combinedMap.size(), 3 ); // 2 from respaths, 1 from resfile
	MapContainsPaths( allExpectedPaths, combinedMap, "ResolvedCombinedMap" );
	ValidatePathMap( allExpectedPaths, combinedMap, defaultIncludes, defaultExcludes, "ResolvedCombinedMap" );
}


TEST_F( ResourceFilterTest, FilterNamedSection_Valid_RespathSet_ResfileEmpty )
{
	std::string sectionName = "FilterNamedSection_Valid_RespathSet_ResfileEmpty";
	std::string parentPrefixMapStr = "prefix1:/pathA";
	std::string filter = "[ .in1 .in2 ] ![ .ex1 ]";
	std::string respaths = "prefix1:/foo/bar";

	ResourceTools::FilterPrefixMap defaultPrefixMap( parentPrefixMapStr );
	ResourceTools::FilterNamedSection namedSection( sectionName, filter, respaths, "", defaultPrefixMap );

	// Expected values:
	std::set<std::string> onlyValidPaths = { "/pathA/foo/bar" };
	std::vector<std::string> defaultIncludes = { ".in1", ".in2" };
	std::vector<std::string> defaultExcludes = { ".ex1" };

	const auto& respathsMap = namedSection.GetResolvedRespathsMap();
	const auto& resfileMap = namedSection.GetResolvedResfileMap();
	const auto& combinedMap = namedSection.GetCombinedResolvedPathMap();

	ASSERT_EQ( respathsMap.size(), 1 );
	MapContainsPaths( onlyValidPaths, respathsMap, "ResolvedRespathsMap" );
	ValidatePathMap( onlyValidPaths, respathsMap, defaultIncludes, defaultExcludes, "ResolvedRespathsMap" );

	ASSERT_EQ( resfileMap.size(), 0 ); // Nothing in resfile
	EXPECT_TRUE( resfileMap.empty() );

	ASSERT_EQ( combinedMap.size(), 1 ); // 1 from respaths, 0 from resfile
	MapContainsPaths( onlyValidPaths, combinedMap, "ResolvedCombinedMap" );
	ValidatePathMap( onlyValidPaths, combinedMap, defaultIncludes, defaultExcludes, "ResolvedCombinedMap" );
}

TEST_F( ResourceFilterTest, FilterNamedSection_Invalid_RespathMissing )
{
	std::string sectionName = "FilterNamedSection_Invalid_RespathMissing";
	std::string defaultParentPrefixMapStr = "prefix1:/path1";
	std::string filter = "[ .in1 ]";
	std::string resfile = "prefix1:/foo/bar";

	ResourceTools::FilterPrefixMap defaultPrefixMap( defaultParentPrefixMapStr );

	// TODO: Should change code to throw defined error code/type
	try
	{
		ResourceTools::FilterNamedSection namedSection( sectionName, filter, "", resfile, defaultPrefixMap );
		FAIL() << "Expected std::invalid_argument when constructing FilterNamedSection with missing respaths";
	}
	catch( const std::invalid_argument& e )
	{
		std::string errorString = "Respaths attribute is empty for section: " + sectionName;
		EXPECT_STREQ( e.what(), errorString.c_str() );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument to be thrown";
	}
}

TEST_F( ResourceFilterTest, FilterNamedSection_Valid_CombinedResolvedMap )
{
	std::string sectionName = "FilterNamedSection_Valid_CombinedResolvedMap";
	std::string defaultParentPrefixMapStr = "prefixA:/path1";
	std::string filter = "[ .in1 ]";
	std::string respaths = "prefixA:/foo/bar";
	std::string resfile = "prefixA:/loo/car";

	ResourceTools::FilterPrefixMap defaultPrefixMap( defaultParentPrefixMapStr );
	ResourceTools::FilterNamedSection namedSection( sectionName, filter, respaths, resfile, defaultPrefixMap );

	// Expected values:
	std::set<std::string> combinedPaths = { "/path1/foo/bar", "/path1/loo/car" };
	std::set<std::string> respathsPaths = { "/path1/foo/bar" };
	std::set<std::string> resfilesPaths = { "/path1/loo/car" };
	std::vector<std::string> defaultIncludes = { ".in1" };
	std::vector<std::string> defaultExcludes = {};

	const auto& respathsMap = namedSection.GetResolvedRespathsMap();
	const auto& resfileMap = namedSection.GetResolvedResfileMap();
	const auto& combinedMap = namedSection.GetCombinedResolvedPathMap();

	ASSERT_EQ( respathsMap.size(), 1 ); // "/path1/foo/bar"
	MapContainsPaths( respathsPaths, respathsMap, "ResolvedRespathsMap" );
	ValidatePathMap( respathsPaths, respathsMap, defaultIncludes, defaultExcludes, "ResolvedRespathsMap" );

	ASSERT_EQ( resfileMap.size(), 1 ); // "/path1/loo/bar"
	MapContainsPaths( resfilesPaths, resfileMap, "ResolvedResfileMap" );
	ValidatePathMap( resfilesPaths, resfileMap, defaultIncludes, defaultExcludes, "ResolvedResfileMap" );

	ASSERT_EQ( combinedMap.size(), 2 ); // both
	MapContainsPaths( combinedPaths, combinedMap, "ResolvedCombinedMap" );
	ValidatePathMap( combinedPaths, combinedMap, defaultIncludes, defaultExcludes, "ResolvedCombinedMap" );
}

TEST_F( ResourceFilterTest, FilterNamedSection_Valid_CombinedResolvedMap_EmptyTopLevelFilter )
{
	std::string sectionName = "FilterNamedSection_Valid_CombinedResolvedMap_EmptyTopLevelFilter";
	std::string defaultParentPrefixMapStr = "prefixA:/path1";
	std::string filter = ""; // Empty top-level filter should add wildcard ("*") include
	std::string respaths = "prefixA:/foo/bar";
	std::string resfile = "prefixA:/loo/car";

	ResourceTools::FilterPrefixMap defaultPrefixMap( defaultParentPrefixMapStr );
	ResourceTools::FilterNamedSection namedSection( sectionName, filter, respaths, resfile, defaultPrefixMap );

	// Expected values:
	std::set<std::string> combinedPaths = { "/path1/foo/bar", "/path1/loo/car" };
	std::set<std::string> respathsPaths = { "/path1/foo/bar" };
	std::set<std::string> resfilesPaths = { "/path1/loo/car" };
	std::vector<std::string> defaultIncludes = { "*" };
	std::vector<std::string> defaultExcludes = {};

	const auto& respathsMap = namedSection.GetResolvedRespathsMap();
	const auto& resfileMap = namedSection.GetResolvedResfileMap();
	const auto& combinedMap = namedSection.GetCombinedResolvedPathMap();

	ASSERT_EQ( respathsMap.size(), 1 ); // "/path1/foo/bar"
	MapContainsPaths( respathsPaths, respathsMap, "ResolvedRespathsMap" );
	ValidatePathMap( respathsPaths, respathsMap, defaultIncludes, defaultExcludes, "ResolvedRespathsMap" );

	ASSERT_EQ( resfileMap.size(), 1 ); // "/path1/loo/bar"
	MapContainsPaths( resfilesPaths, resfileMap, "ResolvedResfileMap" );
	ValidatePathMap( resfilesPaths, resfileMap, defaultIncludes, defaultExcludes, "ResolvedResfileMap" );

	ASSERT_EQ( combinedMap.size(), 2 ); // both
	MapContainsPaths( combinedPaths, combinedMap, "ResolvedCombinedMap" );
	ValidatePathMap( combinedPaths, combinedMap, defaultIncludes, defaultExcludes, "ResolvedCombinedMap" );
	for( const auto& kv : combinedMap )
	{
		EXPECT_EQ( kv.second.GetIncludeFilter().size(), 1 );
		EXPECT_EQ( kv.second.GetIncludeFilter()[0], "*" ); // Wild-card added when no include filter specified for TOP-LEVEL filter

		EXPECT_EQ( kv.second.GetExcludeFilter().size(), 0 );
		EXPECT_TRUE( kv.second.GetExcludeFilter().empty() );
	}
}

TEST_F( ResourceFilterTest, FilterNamedSection_Valid_CombinedResolvedMap_OnlyExcludeTopLevelFilter )
{
	std::string sectionName = "FilterNamedSection_Valid_CombinedResolvedMap_OnlyExcludeTopLevelFilter";
	std::string defaultParentPrefixMapStr = "prefixA:/path1";
	std::string filter = " ![ .ex1 ] "; // Only exclude filter at top-level should add wildcard ("*") include as well
	std::string respaths = "prefixA:/foo/bar";
	std::string resfile = "prefixA:/loo/car";

	ResourceTools::FilterPrefixMap defaultPrefixMap( defaultParentPrefixMapStr );
	ResourceTools::FilterNamedSection namedSection( sectionName, filter, respaths, resfile, defaultPrefixMap );

	// Expected values:
	std::set<std::string> combinedPaths = { "/path1/foo/bar", "/path1/loo/car" };
	std::set<std::string> respathsPaths = { "/path1/foo/bar" };
	std::set<std::string> resfilesPaths = { "/path1/loo/car" };
	std::vector<std::string> defaultIncludes = { "*" }; // Default added
	std::vector<std::string> defaultExcludes = { ".ex1" };

	const auto& respathsMap = namedSection.GetResolvedRespathsMap();
	const auto& resfileMap = namedSection.GetResolvedResfileMap();
	const auto& combinedMap = namedSection.GetCombinedResolvedPathMap();

	ASSERT_EQ( respathsMap.size(), 1 ); // "/path1/foo/bar"
	MapContainsPaths( respathsPaths, respathsMap, "ResolvedRespathsMap" );
	ValidatePathMap( respathsPaths, respathsMap, defaultIncludes, defaultExcludes, "ResolvedRespathsMap" );

	ASSERT_EQ( resfileMap.size(), 1 ); // "/path1/loo/bar"
	MapContainsPaths( resfilesPaths, resfileMap, "ResolvedResfileMap" );
	ValidatePathMap( resfilesPaths, resfileMap, defaultIncludes, defaultExcludes, "ResolvedResfileMap" );

	ASSERT_EQ( combinedMap.size(), 2 ); // both
	MapContainsPaths( combinedPaths, combinedMap, "ResolvedCombinedMap" );
	ValidatePathMap( combinedPaths, combinedMap, defaultIncludes, defaultExcludes, "ResolvedCombinedMap" );
	for( const auto& kv : combinedMap )
	{
		EXPECT_EQ( kv.second.GetIncludeFilter().size(), 1 );
		EXPECT_EQ( kv.second.GetIncludeFilter()[0], "*" ); // Wild-card added when no include filter specified for TOP-LEVEL filter

		EXPECT_EQ( kv.second.GetExcludeFilter().size(), 1 );
		EXPECT_EQ( kv.second.GetExcludeFilter()[0], ".ex1" );
	}
}

TEST_F( ResourceFilterTest, FilterNamedSection_Valid_DiffCombinedResolvedMap_EmptyTopLevelFilter_OverrideRespath )
{
	std::string sectionName = "FilterNamedSection_Valid_DiffCombinedResolvedMap_EmptyTopLevelFilter_OverrideRespath";
	std::string defaultParentPrefixMapStr = "prefixA:/path1";
	std::string filter = ""; // Empty top-level filter should add wildcard ("*") include filter
	std::string respaths = "prefixA:/foo/bar [ .inlineInclude ] ![ .inlineExclude ]"; // Inline filters to override
	std::string resfile = "prefixA:/loo/car";

	ResourceTools::FilterPrefixMap defaultPrefixMap( defaultParentPrefixMapStr );
	ResourceTools::FilterNamedSection namedSection( sectionName, filter, respaths, resfile, defaultPrefixMap );

	// Expected values:
	std::set<std::string> combinedPaths = { "/path1/foo/bar", "/path1/loo/car" };
	std::set<std::string> respathsPaths = { "/path1/foo/bar" };
	std::set<std::string> resfilesPaths = { "/path1/loo/car" };
	std::vector<std::string> defaultIncludes = { "*" }; // Default "*" added to top-level include
	std::vector<std::string> defaultExcludes = {};
	std::vector<std::string> overrideIncludes = { "*", ".inlineInclude" };
	std::vector<std::string> overrideExcludes = { ".inlineExclude" };

	const auto& respathsMap = namedSection.GetResolvedRespathsMap();
	const auto& resfileMap = namedSection.GetResolvedResfileMap();
	const auto& combinedMap = namedSection.GetCombinedResolvedPathMap();

	ASSERT_EQ( respathsMap.size(), 1 ); // "/path1/foo/bar" + "[ *, .inlineInclude ] ![ .inlineExclude ]"
	MapContainsPaths( respathsPaths, respathsMap, "ResolvedRespathsMap" );
	ValidatePathMap( respathsPaths, respathsMap, overrideIncludes, overrideExcludes, "ResolvedRespathsMap" );

	ASSERT_EQ( resfileMap.size(), 1 ); // "/path1/loo/bar" + "[ * ]"
	MapContainsPaths( resfilesPaths, resfileMap, "ResolvedResfileMap" );
	ValidatePathMap( resfilesPaths, resfileMap, defaultIncludes, defaultExcludes, "ResolvedResfileMap" );

	ASSERT_EQ( combinedMap.size(), 2 ); // both
	MapContainsPaths( combinedPaths, combinedMap, "ResolvedCombinedMap" );
	// Manually validate combined map since it combines both overrides and defaults
	for( const auto& kv : combinedMap )
	{
		// The "respaths" part:
		if( kv.first == "/path1/foo/bar" )
		{
			EXPECT_EQ( kv.second.GetIncludeFilter().size(), 2 );
			EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), "*" ) != kv.second.GetIncludeFilter().end() );
			EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".inlineInclude" ) != kv.second.GetIncludeFilter().end() );

			EXPECT_EQ( kv.second.GetExcludeFilter().size(), 1 );
			EXPECT_EQ( kv.second.GetExcludeFilter()[0], ".inlineExclude" );
		}
		// The "resfile" part:
		else if( kv.first == "/path1/loo/car" )
		{
			EXPECT_EQ( kv.second.GetIncludeFilter().size(), 1 );
			EXPECT_EQ( kv.second.GetIncludeFilter()[0], "*" ); // Wild-card added when no include filter specified for TOP-LEVEL filter

			EXPECT_EQ( kv.second.GetExcludeFilter().size(), 0 );
		}
	}
}

TEST_F( ResourceFilterTest, FilterNamedSection_Valid_DiffCombinedResolvedMap_EmptyTopLevelFilter_OverrideResfile )
{
	std::string sectionName = "FilterNamedSection_Valid_DiffCombinedResolvedMap_EmptyTopLevelFilter_OverrideResfile";
	std::string defaultParentPrefixMapStr = "prefixA:/path1";
	std::string filter = ""; // Empty top-level filter should add wildcard ("*") include filter
	std::string respaths = "prefixA:/foo/bar";
	std::string resfile = "prefixA:/loo/car [ .inlineInclude ] ![ .inlineExclude ]"; // Inline filters to override

	ResourceTools::FilterPrefixMap defaultPrefixMap( defaultParentPrefixMapStr );
	ResourceTools::FilterNamedSection namedSection( sectionName, filter, respaths, resfile, defaultPrefixMap );

	// Expected values:
	std::set<std::string> combinedPaths = { "/path1/foo/bar", "/path1/loo/car" };
	std::set<std::string> respathsPaths = { "/path1/foo/bar" };
	std::set<std::string> resfilesPaths = { "/path1/loo/car" };
	std::vector<std::string> defaultIncludes = { "*" }; // Default "*" added to top-level include
	std::vector<std::string> defaultExcludes = {};
	std::vector<std::string> overrideIncludes = { "*", ".inlineInclude" };
	std::vector<std::string> overrideExcludes = { ".inlineExclude" };

	const auto& respathsMap = namedSection.GetResolvedRespathsMap();
	const auto& resfileMap = namedSection.GetResolvedResfileMap();
	const auto& combinedMap = namedSection.GetCombinedResolvedPathMap();

	ASSERT_EQ( respathsMap.size(), 1 ); // "/path1/foo/bar" + "[ * ]"
	MapContainsPaths( respathsPaths, respathsMap, "ResolvedRespathsMap" );
	ValidatePathMap( respathsPaths, respathsMap, defaultIncludes, defaultExcludes, "ResolvedRespathsMap" );

	ASSERT_EQ( resfileMap.size(), 1 ); // "/path1/loo/bar" + "[ *, .inlineInclude ] ![ .inlineExclude ]"
	MapContainsPaths( resfilesPaths, resfileMap, "ResolvedResfileMap" );
	ValidatePathMap( resfilesPaths, resfileMap, overrideIncludes, overrideExcludes, "ResolvedResfileMap" );

	ASSERT_EQ( combinedMap.size(), 2 ); // both
	MapContainsPaths( combinedPaths, combinedMap, "ResolvedCombinedMap" );
	// Manually validate combined map since it combines both overrides and defaults
	for( const auto& kv : combinedMap )
	{
		// The "resfile" part:
		if( kv.first == "/path1/loo/car" )
		{
			EXPECT_EQ( kv.second.GetIncludeFilter().size(), 2 );
			EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), "*" ) != kv.second.GetIncludeFilter().end() );
			EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".inlineInclude" ) != kv.second.GetIncludeFilter().end() );

			EXPECT_EQ( kv.second.GetExcludeFilter().size(), 1 );
			EXPECT_EQ( kv.second.GetExcludeFilter()[0], ".inlineExclude" );
		}
		// The "respaths" part:
		else if( kv.first == "/path1/foo/bar" )
		{
			EXPECT_EQ( kv.second.GetIncludeFilter().size(), 1 );
			EXPECT_EQ( kv.second.GetIncludeFilter()[0], "*" ); // Wild-card added when no include filter specified for TOP-LEVEL filter

			EXPECT_EQ( kv.second.GetExcludeFilter().size(), 0 );
		}
	}
}

TEST_F( ResourceFilterTest, FilterNamedSection_Valid_DiffCombinedResolvedMap_OnlyExcludeTopLevelFilter_OverrideRespath )
{
	std::string sectionName = "FilterNamedSection_Valid_DiffCombinedResolvedMap_OnlyExcludeTopLevelFilter_OverrideRespath";
	std::string defaultParentPrefixMapStr = "prefixA:/path1";
	std::string filter = "![ .toplevelExclude ]"; // Only exclude filter at top-level should add wildcard ("*") include as well
	std::string respaths = "prefixA:/foo/bar [ .inlineInclude ] ![ .inlineExclude ]"; // Inline filters to override
	std::string resfile = "prefixA:/loo/car";

	ResourceTools::FilterPrefixMap defaultPrefixMap( defaultParentPrefixMapStr );
	ResourceTools::FilterNamedSection namedSection( sectionName, filter, respaths, resfile, defaultPrefixMap );

	// Expected values:
	std::set<std::string> combinedPaths = { "/path1/foo/bar", "/path1/loo/car" };
	std::set<std::string> respathsPaths = { "/path1/foo/bar" };
	std::set<std::string> resfilesPaths = { "/path1/loo/car" };
	std::vector<std::string> defaultIncludes = { "*" }; // Default "*" added to top-level include
	std::vector<std::string> defaultExcludes = { ".toplevelExclude" };
	std::vector<std::string> overrideIncludes = { "*", ".inlineInclude" };
	std::vector<std::string> overrideExcludes = { ".toplevelExclude", ".inlineExclude" };

	const auto& respathsMap = namedSection.GetResolvedRespathsMap();
	const auto& resfileMap = namedSection.GetResolvedResfileMap();
	const auto& combinedMap = namedSection.GetCombinedResolvedPathMap();

	ASSERT_EQ( respathsMap.size(), 1 ); // "/path1/foo/bar" + "[ *, .inlineInclude ] ![ .toplevelExclude .inlineExclude ]"
	MapContainsPaths( respathsPaths, respathsMap, "ResolvedRespathsMap" );
	ValidatePathMap( respathsPaths, respathsMap, overrideIncludes, overrideExcludes, "ResolvedRespathsMap" );

	ASSERT_EQ( resfileMap.size(), 1 ); // "/path1/loo/bar" + "[ * ]"
	MapContainsPaths( resfilesPaths, resfileMap, "ResolvedResfileMap" );
	ValidatePathMap( resfilesPaths, resfileMap, defaultIncludes, defaultExcludes, "ResolvedResfileMap" );

	ASSERT_EQ( combinedMap.size(), 2 ); // both
	MapContainsPaths( combinedPaths, combinedMap, "ResolvedCombinedMap" );
	// Manually validate combined map since it combines both overrides and defaults
	for( const auto& kv : combinedMap )
	{
		// The "respaths" part:
		if( kv.first == "/path1/foo/bar" )
		{
			EXPECT_EQ( kv.second.GetIncludeFilter().size(), 2 );
			EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), "*" ) != kv.second.GetIncludeFilter().end() );
			EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".inlineInclude" ) != kv.second.GetIncludeFilter().end() );

			EXPECT_EQ( kv.second.GetExcludeFilter().size(), 2 );
			EXPECT_TRUE( std::find( kv.second.GetExcludeFilter().begin(), kv.second.GetExcludeFilter().end(), ".toplevelExclude" ) != kv.second.GetExcludeFilter().end() );
			EXPECT_TRUE( std::find( kv.second.GetExcludeFilter().begin(), kv.second.GetExcludeFilter().end(), ".inlineExclude" ) != kv.second.GetExcludeFilter().end() );
		}
		// The "resfile" part:
		else if( kv.first == "/path1/loo/car" )
		{
			EXPECT_EQ( kv.second.GetIncludeFilter().size(), 1 );
			EXPECT_EQ( kv.second.GetIncludeFilter()[0], "*" ); // Wild-card added when no include filter specified for TOP-LEVEL filter

			EXPECT_EQ( kv.second.GetExcludeFilter().size(), 1 );
			EXPECT_EQ( kv.second.GetExcludeFilter()[0], ".toplevelExclude" ); // Top-level exclude filter
		}
	}
}

TEST_F( ResourceFilterTest, FilterNamedSection_Valid_DiffCombinedResolvedMap_OnlyExcludeTopLevelFilter_OverrideResfile )
{
	std::string sectionName = "FilterNamedSection_Valid_DiffCombinedResolvedMap_OnlyExcludeTopLevelFilter_OverrideRespath";
	std::string defaultParentPrefixMapStr = "prefixA:/path1";
	std::string filter = "![ .toplevelExclude ]"; // Only exclude filter at top-level should add wildcard ("*") include as well
	std::string respaths = "prefixA:/foo/bar";
	std::string resfile = "prefixA:/loo/car [ .inlineInclude ] ![ .inlineExclude ]"; // Inline filters to override";

	ResourceTools::FilterPrefixMap defaultPrefixMap( defaultParentPrefixMapStr );
	ResourceTools::FilterNamedSection namedSection( sectionName, filter, respaths, resfile, defaultPrefixMap );

	// Expected values:
	std::set<std::string> combinedPaths = { "/path1/foo/bar", "/path1/loo/car" };
	std::set<std::string> respathsPaths = { "/path1/foo/bar" };
	std::set<std::string> resfilesPaths = { "/path1/loo/car" };
	std::vector<std::string> defaultIncludes = { "*" }; // Default "*" added to top-level include
	std::vector<std::string> defaultExcludes = { ".toplevelExclude" };
	std::vector<std::string> overrideIncludes = { "*", ".inlineInclude" };
	std::vector<std::string> overrideExcludes = { ".toplevelExclude", ".inlineExclude" };

	const auto& respathsMap = namedSection.GetResolvedRespathsMap();
	const auto& resfileMap = namedSection.GetResolvedResfileMap();
	const auto& combinedMap = namedSection.GetCombinedResolvedPathMap();

	ASSERT_EQ( respathsMap.size(), 1 ); // "/path1/foo/bar" + "[ * ]"
	MapContainsPaths( respathsPaths, respathsMap, "ResolvedRespathsMap" );
	ValidatePathMap( respathsPaths, respathsMap, defaultIncludes, defaultExcludes, "ResolvedRespathsMap" );

	ASSERT_EQ( resfileMap.size(), 1 ); // "/path1/loo/bar" + "[ *, .inlineInclude ] ![ .toplevelExclude .inlineExclude ]"
	MapContainsPaths( resfilesPaths, resfileMap, "ResolvedResfileMap" );
	ValidatePathMap( resfilesPaths, resfileMap, overrideIncludes, overrideExcludes, "ResolvedResfileMap" );

	ASSERT_EQ( combinedMap.size(), 2 ); // both
	MapContainsPaths( combinedPaths, combinedMap, "ResolvedCombinedMap" );
	// Manually validate combined map since it combines both overrides and defaults
	for( const auto& kv : combinedMap )
	{
		// The "resfile" part:
		if( kv.first == "/path1/loo/car" )
		{
			EXPECT_EQ( kv.second.GetIncludeFilter().size(), 2 );
			EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), "*" ) != kv.second.GetIncludeFilter().end() );
			EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".inlineInclude" ) != kv.second.GetIncludeFilter().end() );

			EXPECT_EQ( kv.second.GetExcludeFilter().size(), 2 );
			EXPECT_TRUE( std::find( kv.second.GetExcludeFilter().begin(), kv.second.GetExcludeFilter().end(), ".toplevelExclude" ) != kv.second.GetExcludeFilter().end() );
			EXPECT_TRUE( std::find( kv.second.GetExcludeFilter().begin(), kv.second.GetExcludeFilter().end(), ".inlineExclude" ) != kv.second.GetExcludeFilter().end() );
		}
		// The "respaths" part:
		else if( kv.first == "/path1/foo/bar" )
		{
			EXPECT_EQ( kv.second.GetIncludeFilter().size(), 1 );
			EXPECT_EQ( kv.second.GetIncludeFilter()[0], "*" ); // Wild-card added when no include filter specified for TOP-LEVEL filter

			EXPECT_EQ( kv.second.GetExcludeFilter().size(), 1 );
			EXPECT_EQ( kv.second.GetExcludeFilter()[0], ".toplevelExclude" ); // Top-level exclude filter
		}
	}
}

TEST_F( ResourceFilterTest, FilterNamedSection_Valid_CombinedResolvedMap_OverwrittenByResfileMap )
{
	std::string sectionName = "FilterNamedSection_Valid_CombinedResolvedMap_OverwrittenByResfileMap";
	std::string defaultParentPrefixMapStr = "prefix1:/pathA;/pathB";
	std::string filter = "[ .in1 .in2 ] ![ .ex1 ]";
	std::string respaths = "prefix1:/foo/bar";
	std::string resfile = "prefix1:/foo/bar [ .extra ]"; // Same path, extra include filter

	ResourceTools::FilterPrefixMap defaultPrefixMap( defaultParentPrefixMapStr );
	ResourceTools::FilterNamedSection namedSection( sectionName, filter, respaths, resfile, defaultPrefixMap );

	// Expected values:
	std::set<std::string> allPaths = { "/pathA/foo/bar", "/pathB/foo/bar" };
	std::vector<std::string> defaultIncludes = { ".in1", ".in2" };
	std::vector<std::string> allExcludes = { ".ex1" };
	std::vector<std::string> overrideIncludes = { ".in1", ".in2", ".extra" };

	const auto& combinedMap = namedSection.GetCombinedResolvedPathMap();
	const auto& respathsMap = namedSection.GetResolvedRespathsMap();
	const auto& resfileMap = namedSection.GetResolvedResfileMap();

	ASSERT_EQ( respathsMap.size(), 2 );
	MapContainsPaths( allPaths, respathsMap, "ResolvedRespathsMap" );
	ValidatePathMap( allPaths, respathsMap, defaultIncludes, allExcludes, "ResolvedRespathsMap" );

	ASSERT_EQ( resfileMap.size(), 2 );
	MapContainsPaths( allPaths, resfileMap, "ResolvedResfileMap" );
	ValidatePathMap( allPaths, resfileMap, overrideIncludes, allExcludes, "ResolvedResfileMap" );

	ASSERT_EQ( combinedMap.size(), 2 ); // Both, same count but now with overrides
	MapContainsPaths( allPaths, combinedMap, "ResolvedCombinedMap" );
	ValidatePathMap( allPaths, combinedMap, overrideIncludes, allExcludes, "ResolvedCombinedMap" );

	// Re-validate that RespathsMap is unchanged
	const auto& respathsAgainMap = namedSection.GetResolvedRespathsMap();
	MapContainsPaths( allPaths, respathsAgainMap, "ResolvedRespathsMap-Again" );
	ValidatePathMap( allPaths, respathsAgainMap, defaultIncludes, allExcludes, "ResolvedRespathsMap-Again" );
}

TEST_F( ResourceFilterTest, FilterNamedSection_Valid_SameCombinedResolvedMap_EmptyTopLevelFilter_OverwrittenByResfileMap )
{
	std::string sectionName = "FilterNamedSection_Valid_SameCombinedResolvedMap_EmptyTopLevelFilter_OverwrittenByResfileMap";
	std::string defaultParentPrefixMapStr = "prefix1:/pathA;/pathB";
	std::string filter = ""; // Empty top-level filter should add wildcard ("*") include
	std::string respaths = "prefix1:/foo/bar";
	std::string resfile = "prefix1:/foo/bar [ .extra ]"; // Same path, extra include filter

	ResourceTools::FilterPrefixMap defaultPrefixMap( defaultParentPrefixMapStr );
	ResourceTools::FilterNamedSection namedSection( sectionName, filter, respaths, resfile, defaultPrefixMap );

	// Expected values:
	std::set<std::string> allPaths = { "/pathA/foo/bar", "/pathB/foo/bar" };
	std::vector<std::string> defaultIncludes = { "*" };
	std::vector<std::string> allExcludes = {};
	std::vector<std::string> overrideIncludes = { "*", ".extra" };

	const auto& combinedMap = namedSection.GetCombinedResolvedPathMap();
	const auto& respathsMap = namedSection.GetResolvedRespathsMap();
	const auto& resfileMap = namedSection.GetResolvedResfileMap();

	ASSERT_EQ( respathsMap.size(), 2 );
	MapContainsPaths( allPaths, respathsMap, "ResolvedRespathsMap" );
	ValidatePathMap( allPaths, respathsMap, defaultIncludes, allExcludes, "ResolvedRespathsMap" );

	ASSERT_EQ( resfileMap.size(), 2 );
	MapContainsPaths( allPaths, resfileMap, "ResolvedResfileMap" );
	ValidatePathMap( allPaths, resfileMap, overrideIncludes, allExcludes, "ResolvedResfileMap" );

	ASSERT_EQ( combinedMap.size(), 2 ); // Both, same count but now with overrides
	MapContainsPaths( allPaths, combinedMap, "ResolvedCombinedMap" );
	ValidatePathMap( allPaths, combinedMap, overrideIncludes, allExcludes, "ResolvedCombinedMap" );
	for( const auto& kv : combinedMap )
	{
		EXPECT_EQ( kv.second.GetIncludeFilter().size(), 2 );
		EXPECT_EQ( kv.second.GetIncludeFilter()[0], "*" ); // Wild-card added when no include filter specified for TOP-LEVEL filter
		EXPECT_EQ( kv.second.GetIncludeFilter()[1], ".extra" );

		EXPECT_EQ( kv.second.GetExcludeFilter().size(), 0 );
		EXPECT_TRUE( kv.second.GetExcludeFilter().empty() );
	}

	// Re-validate that RespathsMap is unchanged
	const auto& respathsAgainMap = namedSection.GetResolvedRespathsMap();
	MapContainsPaths( allPaths, respathsAgainMap, "ResolvedRespathsMap-Again" );
	ValidatePathMap( allPaths, respathsAgainMap, defaultIncludes, allExcludes, "ResolvedRespathsMap-Again" );
}

TEST_F( ResourceFilterTest, FilterNamedSection_Valid_SameCombinedResolvedMap_EmptyTopLevelFilter_OverwrittenByRespathMap )
{
	std::string sectionName = "FilterNamedSection_Valid_SameCombinedResolvedMap_EmptyTopLevelFilter_OverwrittenByRespathMap";
	std::string defaultParentPrefixMapStr = "prefix1:/pathA;/pathB";
	std::string filter = ""; // Empty top-level filter should add wildcard ("*") include
	std::string respaths = "prefix1:/foo/bar [ .extra ]"; // Same path, extra include filter
	std::string resfile = "prefix1:/foo/bar";

	ResourceTools::FilterPrefixMap defaultPrefixMap( defaultParentPrefixMapStr );
	ResourceTools::FilterNamedSection namedSection( sectionName, filter, respaths, resfile, defaultPrefixMap );

	// Expected values:
	std::set<std::string> allPaths = { "/pathA/foo/bar", "/pathB/foo/bar" };
	std::vector<std::string> defaultIncludes = { "*" };
	std::vector<std::string> allExcludes = {};
	std::vector<std::string> overrideIncludes = { "*", ".extra" };

	const auto& combinedMap = namedSection.GetCombinedResolvedPathMap();
	const auto& respathsMap = namedSection.GetResolvedRespathsMap();
	const auto& resfileMap = namedSection.GetResolvedResfileMap();

	ASSERT_EQ( respathsMap.size(), 2 );
	MapContainsPaths( allPaths, respathsMap, "ResolvedRespathsMap" );
	ValidatePathMap( allPaths, respathsMap, overrideIncludes, allExcludes, "ResolvedRespathsMap" );

	ASSERT_EQ( resfileMap.size(), 2 );
	MapContainsPaths( allPaths, resfileMap, "ResolvedResfileMap" );
	ValidatePathMap( allPaths, resfileMap, defaultIncludes, allExcludes, "ResolvedResfileMap" );

	ASSERT_EQ( combinedMap.size(), 2 ); // Both, same count but now with overrides
	MapContainsPaths( allPaths, combinedMap, "ResolvedCombinedMap" );
	ValidatePathMap( allPaths, combinedMap, overrideIncludes, allExcludes, "ResolvedCombinedMap" );
	for( const auto& kv : combinedMap )
	{
		EXPECT_EQ( kv.second.GetIncludeFilter().size(), 2 );
		EXPECT_EQ( kv.second.GetIncludeFilter()[0], "*" ); // Wild-card added when no include filter specified for TOP-LEVEL filter
		EXPECT_EQ( kv.second.GetIncludeFilter()[1], ".extra" );

		EXPECT_EQ( kv.second.GetExcludeFilter().size(), 0 );
		EXPECT_TRUE( kv.second.GetExcludeFilter().empty() );
	}

	// Re-validate original  Respaths/files Maps
	const auto& respathsAgainMap = namedSection.GetResolvedRespathsMap();
	MapContainsPaths( allPaths, respathsAgainMap, "ResolvedRespathsMap-Again" );
	ValidatePathMap( allPaths, respathsAgainMap, overrideIncludes, allExcludes, "ResolvedRespathsMap-Again" );

	const auto& resfileAgainMap = namedSection.GetResolvedResfileMap();
	MapContainsPaths( allPaths, resfileAgainMap, "ResolvedResfileMap-Again" );
	ValidatePathMap( allPaths, resfileAgainMap, defaultIncludes, allExcludes, "ResolvedResfileMap-Again" );
}

TEST_F( ResourceFilterTest, FilterNamedSection_Valid_SameCombinedResolvedMap_ExcludeOnlyTopLevelFilter_OverwrittenByResfileMap )
{
	std::string sectionName = "FilterNamedSection_Valid_SameCombinedResolvedMap_ExcludeOnlyTopLevelFilter_OverwrittenByResfileMap";
	std::string defaultParentPrefixMapStr = "prefix1:/pathA;/pathB";
	std::string filter = " ![ .topLevelExclude ]"; // Only exclude filter at top-level should add wildcard ("*") include
	std::string respaths = "prefix1:/foo/bar";
	std::string resfile = "prefix1:/foo/bar [ .extra ]"; // Same path, extra include filter

	ResourceTools::FilterPrefixMap defaultPrefixMap( defaultParentPrefixMapStr );
	ResourceTools::FilterNamedSection namedSection( sectionName, filter, respaths, resfile, defaultPrefixMap );

	// Expected values:
	std::set<std::string> allPaths = { "/pathA/foo/bar", "/pathB/foo/bar" };
	std::vector<std::string> defaultIncludes = { "*" };
	std::vector<std::string> allExcludes = { ".topLevelExclude" };
	std::vector<std::string> overrideIncludes = { "*", ".extra" };

	const auto& combinedMap = namedSection.GetCombinedResolvedPathMap();
	const auto& respathsMap = namedSection.GetResolvedRespathsMap();
	const auto& resfileMap = namedSection.GetResolvedResfileMap();

	ASSERT_EQ( respathsMap.size(), 2 );
	MapContainsPaths( allPaths, respathsMap, "ResolvedRespathsMap" );
	ValidatePathMap( allPaths, respathsMap, defaultIncludes, allExcludes, "ResolvedRespathsMap" );

	ASSERT_EQ( resfileMap.size(), 2 );
	MapContainsPaths( allPaths, resfileMap, "ResolvedResfileMap" );
	ValidatePathMap( allPaths, resfileMap, overrideIncludes, allExcludes, "ResolvedResfileMap" );

	ASSERT_EQ( combinedMap.size(), 2 ); // Both, same count but now with overrides
	MapContainsPaths( allPaths, combinedMap, "ResolvedCombinedMap" );
	ValidatePathMap( allPaths, combinedMap, overrideIncludes, allExcludes, "ResolvedCombinedMap" );
	for( const auto& kv : combinedMap )
	{
		EXPECT_EQ( kv.second.GetIncludeFilter().size(), 2 );
		EXPECT_EQ( kv.second.GetIncludeFilter()[0], "*" ); // Wild-card added when no include filter specified for TOP-LEVEL filter
		EXPECT_EQ( kv.second.GetIncludeFilter()[1], ".extra" );

		EXPECT_EQ( kv.second.GetExcludeFilter().size(), 1 );
		EXPECT_EQ( kv.second.GetExcludeFilter()[0], ".topLevelExclude" );
	}

	// Re-validate that RespathsMap is unchanged
	const auto& respathsAgainMap = namedSection.GetResolvedRespathsMap();
	MapContainsPaths( allPaths, respathsAgainMap, "ResolvedRespathsMap-Again" );
	ValidatePathMap( allPaths, respathsAgainMap, defaultIncludes, allExcludes, "ResolvedRespathsMap-Again" );
}

TEST_F( ResourceFilterTest, FilterNamedSection_Valid_SameCombinedResolvedMap_ExcludeOnlyTopLevelFilter_OverwrittenByRespathMap )
{
	std::string sectionName = "FilterNamedSection_Valid_SameCombinedResolvedMap_ExcludeOnlyTopLevelFilter_OverwrittenByRespathMap";
	std::string defaultParentPrefixMapStr = "prefix1:/pathA;/pathB";
	std::string filter = " ![ .topLevelExclude ]"; // Only exclude filter at top-level should add wildcard ("*") include
	std::string respaths = "prefix1:/foo/bar [ .extra ]"; // Same path, extra include filter
	std::string resfile = "prefix1:/foo/bar";

	ResourceTools::FilterPrefixMap defaultPrefixMap( defaultParentPrefixMapStr );
	ResourceTools::FilterNamedSection namedSection( sectionName, filter, respaths, resfile, defaultPrefixMap );

	// Expected values:
	std::set<std::string> allPaths = { "/pathA/foo/bar", "/pathB/foo/bar" };
	std::vector<std::string> defaultIncludes = { "*" };
	std::vector<std::string> allExcludes = { ".topLevelExclude" };
	std::vector<std::string> overrideIncludes = { "*", ".extra" };

	const auto& combinedMap = namedSection.GetCombinedResolvedPathMap();
	const auto& respathsMap = namedSection.GetResolvedRespathsMap();
	const auto& resfileMap = namedSection.GetResolvedResfileMap();

	ASSERT_EQ( respathsMap.size(), 2 );
	MapContainsPaths( allPaths, respathsMap, "ResolvedRespathsMap" );
	ValidatePathMap( allPaths, respathsMap, overrideIncludes, allExcludes, "ResolvedRespathsMap" );

	ASSERT_EQ( resfileMap.size(), 2 );
	MapContainsPaths( allPaths, resfileMap, "ResolvedResfileMap" );
	ValidatePathMap( allPaths, resfileMap, defaultIncludes, allExcludes, "ResolvedResfileMap" );

	ASSERT_EQ( combinedMap.size(), 2 ); // Both, same count but now with overrides
	MapContainsPaths( allPaths, combinedMap, "ResolvedCombinedMap" );
	ValidatePathMap( allPaths, combinedMap, overrideIncludes, allExcludes, "ResolvedCombinedMap" );
	for( const auto& kv : combinedMap )
	{
		EXPECT_EQ( kv.second.GetIncludeFilter().size(), 2 );
		EXPECT_EQ( kv.second.GetIncludeFilter()[0], "*" ); // Wild-card added when no include filter specified for TOP-LEVEL filter
		EXPECT_EQ( kv.second.GetIncludeFilter()[1], ".extra" );

		EXPECT_EQ( kv.second.GetExcludeFilter().size(), 1 );
		EXPECT_EQ( kv.second.GetExcludeFilter()[0], ".topLevelExclude" );
	}

	// Re-validate original  Respaths/files Maps
	const auto& respathsAgainMap = namedSection.GetResolvedRespathsMap();
	MapContainsPaths( allPaths, respathsAgainMap, "ResolvedRespathsMap-Again" );
	ValidatePathMap( allPaths, respathsAgainMap, overrideIncludes, allExcludes, "ResolvedRespathsMap-Again" );

	const auto& resfileAgainMap = namedSection.GetResolvedResfileMap();
	MapContainsPaths( allPaths, resfileAgainMap, "ResolvedResfileMap-Again" );
	ValidatePathMap( allPaths, resfileAgainMap, defaultIncludes, allExcludes, "ResolvedResfileMap-Again" );
}

// ------------------------------------------

TEST_F( ResourceFilterTest, FilterResourceFile_Load_example1_ini )
{
	// Use the test fixture's helper to get the absolute path
	const std::filesystem::path iniPath = GetTestFileFileAbsolutePath( "ExampleIniFiles/example1.ini" );

	try
	{
		ResourceTools::FilterResourceFile resourceFile( iniPath.string() );
		const auto& fullPathMap = resourceFile.GetFullResolvedPathMap();

		// Validate the paths:
		std::set<std::string> expectedPaths = {
			// From the respaths attribute:
			"./Indicies/firstLine/...",           // "res:/firstLine/..."
			"./resourcesOnBranch/firstLine/...",  // "res:/firstLine/..."
			"./Indicies/secondLine/...",          // "res:/secondLine/..."
			"./resourcesOnBranch/secondLine/...", // "res:/secondLine/..."
			"./ResourceGroups/thirdLine/...",     // "res2:/thirdLine/..."
			// From the resfile attribute:
			"./Indicies/binaryFileIndex_v0_0_0.txt",         // "res:/binaryFileIndex_v0_0_0.txt"
			"./resourcesOnBranch/binaryFileIndex_v0_0_0.txt" // "res:/binaryFileIndex_v0_0_0.txt"
		};
		std::vector<std::string> expectedIncludes = { ".yaml" };
		std::vector<std::string> expectedExcludes = {};

		MapContainsPaths( expectedPaths, fullPathMap, "FullResolvedPathMap from example1.ini" );
		ValidatePathMap( expectedPaths, fullPathMap, expectedIncludes, expectedExcludes, "FullResolvedPathMap from example1.ini" );
	}
	catch( const std::exception& e )
	{
		FAIL() << "Exception thrown while loading example1.ini: " << e.what();
	}
	catch( ... )
	{
		FAIL() << "Unknown exception thrown while loading example1.ini";
	}
}

TEST_F( ResourceFilterTest, FilterResourceFile_Load_invalidMissingDefaultSection_ini )
{
	const std::filesystem::path iniPath = GetTestFileFileAbsolutePath( "ExampleIniFiles/invalidMissingDefaultSection.ini" );

	try
	{
		ResourceTools::FilterResourceFile resourceFile( iniPath.string() );
		FAIL() << "Expected std::invalid_argument when loading ini file missing [DEFAULT] section";
	}
	catch( const std::invalid_argument& e )
	{
		std::string expectedError = "Missing [DEFAULT] section in INI file: " + iniPath.string();
		EXPECT_STREQ( e.what(), expectedError.c_str() );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument when loading ini file missing [DEFAULT] section";
	}
}

TEST_F( ResourceFilterTest, FilterResourceFile_Load_invalidMissingNamedSection_ini )
{
	const std::filesystem::path iniPath = GetTestFileFileAbsolutePath( "ExampleIniFiles/invalidMissingNamedSection.ini" );

	try
	{
		ResourceTools::FilterResourceFile resourceFile( iniPath.string() );
		FAIL() << "Expected std::invalid_argument when loading ini file missing [NamedSection] section";
	}
	catch( const std::invalid_argument& e )
	{
		std::string expectedError = "No namedSections defined in INI file: " + iniPath.string();
		EXPECT_STREQ( e.what(), expectedError.c_str() );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument when loading ini file missing [NamedSection] section";
	}
}

TEST_F( ResourceFilterTest, FilterResourceFile_Load_iniFileNotFound )
{
	const std::filesystem::path iniPath = GetTestFileFileAbsolutePath( "ExampleIniFiles/iniFileNotFound.ini" );

	try
	{
		ResourceTools::FilterResourceFile resourceFile( iniPath.string() );
		FAIL() << "Expected std::runtime_error when loading non-existent ini file";
	}
	catch( const std::runtime_error& e )
	{
		std::string expectedError = "Failed to parse INI file: " + iniPath.string() + " - unable to open file";
		EXPECT_STREQ( e.what(), expectedError.c_str() );
	}
	catch( ... )
	{
		FAIL() << "Expected std::runtime_error when loading non-existent ini file";
	}
}

TEST_F( ResourceFilterTest, FilterResourceFile_Load_invalidPrefixmap_ini )
{
    const std::filesystem::path iniPath = GetTestFileFileAbsolutePath( "ExampleIniFiles/invalidPrefixmap.ini" );
    try
    {
        ResourceTools::FilterResourceFile resourceFile( iniPath.string() );
        FAIL() << "Expected std::invalid_argument when loading ini file with invalid prefixmap";
    }
    catch( const std::invalid_argument& e )
    {
        std::string expectedError = "Invalid prefixmap format: No paths defined for prefix: prefix1";
    	EXPECT_STREQ( e.what(), expectedError.c_str() );
    }
    catch( ... )
    {
        FAIL() << "Expected std::invalid_argument when loading invalidPrefixmap.ini file";
    }
}

TEST_F( ResourceFilterTest, FilterResourceFile_Load_invalidSectionFilter_ini )
{
    const std::filesystem::path iniPath = GetTestFileFileAbsolutePath( "ExampleIniFiles/invalidSectionFilter.ini" );
    try
    {
        ResourceTools::FilterResourceFile resourceFile( iniPath.string() );
        FAIL() << "Expected std::invalid_argument when loading ini file with invalid section filter";
    }
    catch( const std::invalid_argument& e )
    {
    	std::string expectedError = "Invalid filter format: missing ']'";
    	EXPECT_STREQ( e.what(), expectedError.c_str() );
    }
    catch( ... )
    {
        FAIL() << "Expected std::invalid_argument when loading invalidSectionFilter.ini file";
    }
}

TEST_F( ResourceFilterTest, FilterResourceFile_Load_invalidInlineFilter_ini )
{
    const std::filesystem::path iniPath = GetTestFileFileAbsolutePath( "ExampleIniFiles/invalidInlineFilter.ini" );
    try
    {
        ResourceTools::FilterResourceFile resourceFile( iniPath.string() );
        FAIL() << "Expected std::invalid_argument when loading ini file with invalid inline filter";
    }
    catch( const std::invalid_argument& e )
    {
    	std::string expectedError = "Invalid filter format: missing '['";
    	EXPECT_STREQ( e.what(), expectedError.c_str() );
    }
    catch( ... )
    {
        FAIL() << "Expected std::invalid_argument when loading invalidInlineFilter.ini file";
    }
}

TEST_F( ResourceFilterTest, FilterResourceFile_Load_invalidPrefixMismatch_ini )
{
    const std::filesystem::path iniPath = GetTestFileFileAbsolutePath( "ExampleIniFiles/invalidPrefixMismatch.ini" );
    try
    {
        ResourceTools::FilterResourceFile resourceFile( iniPath.string() );
        FAIL() << "Expected std::invalid_argument when loading ini file with prefix mismatch";
    }
    catch( const std::invalid_argument& e )
    {
    	std::string expectedError = "Prefix 'prefixDoesNotExist' not present in prefixMap for line: prefixDoesNotExist:/firstLine/*";
    	EXPECT_STREQ( e.what(), expectedError.c_str() );
    }
    catch( ... )
    {
        FAIL() << "Expected std::invalid_argument when loading invalidPrefixMismatch.ini file";
    }
}