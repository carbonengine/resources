// Copyright © 2025 CCP ehf.

#include "ResourceFilterTest.h"

#include <filesystem>
#include <map>
#include <set>
#include <string>

#include "INIReader.h"

#include "FilterDefaultSection.h"
#include "FilterNamedSection.h"
#include "FilterResourceFilter.h"
#include "FilterPrefixmap.h"
#include "FilterPrefixMapEntry.h"
#include "FilterResourceFile.h"
#include "FilterResourcePathFile.h"
#include "ResourceFilter.h"

TEST_F( ResourceFilterTest, LoadAndParseIniFile_example1Ini_UsingIniReader )
{
	// Use the test fixture's helper to get the absolute path
	const std::filesystem::path iniPath = GetTestFileFileAbsolutePath( "ExampleIniFiles/example1.ini" );
	INIReader reader( iniPath.generic_string() );
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
	EXPECT_EQ( respathValueGet, "res:/firstLine/...\nres:/secondLine/*\nres2:/thirdLine/..." ); // Note: Under the hood, INIReader converts multi-empty-lines to a single \n line breaks
	EXPECT_EQ( reader.Keys( "testYamlFilesOverMultiLineResPathsWithEmptyLines" ).size(), 3 );
}

// -----------------------------------------

TEST_F( ResourceFilterTest, FilterResourceFilter_ApplyRule_IncludeFilterOnly )
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

TEST_F( ResourceFilterTest, FilterResourceFilter_ApplyRule_TopLevelExcludeFilterOnly )
{
	// Top-level filter refers to the "filter" attribute of a NamedSection.
	// When there is no include filter specified at the top-level "filter" attribute,
	// a wild-card "*" should be added as default include.
	ResourceTools::FilterResourceFilter filter( "![ .excluded .extension ]", true );
	const auto& includes = filter.GetIncludeFilter();
	const auto& excludes = filter.GetExcludeFilter();

	EXPECT_EQ( excludes.size(), 2 );
	EXPECT_EQ( excludes[0], ".excluded" );
	EXPECT_EQ( excludes[1], ".extension" );

	// Include filter should contain wildcard when no explicit include filter specified at top-level ("filter" attribute)
	EXPECT_EQ( includes.size(), 1 );
	EXPECT_EQ( includes[0], "*" );
}

TEST_F( ResourceFilterTest, FilterResourceFilter_ApplyRule_InLineExcludeFilterOnly )
{
	// "InLine" filter refers to an optional filter element at the end of a
	// respaths/resfile attribute line (class FilterResourcePathFileEntry)
	// The InLine filter applies only to that line (in addition to any parent filter present, if applicable).
	ResourceTools::FilterResourceFilter filter( "![ .excluded .extension ]" );
	const auto& includes = filter.GetIncludeFilter();
	const auto& excludes = filter.GetExcludeFilter();

	EXPECT_EQ( excludes.size(), 2 );
	EXPECT_EQ( excludes[0], ".excluded" );
	EXPECT_EQ( excludes[1], ".extension" );

	// No wildcard should be added to include filter at in-line level
	EXPECT_EQ( includes.size(), 0 );
	EXPECT_TRUE( includes.empty() );
}

TEST_F( ResourceFilterTest, FilterResourceFilter_ApplyRules_UseComplexIncludeExcludeFilters )
{
	ResourceTools::FilterResourceFilter filter( "[ .red .gr2 .dds .png .yaml ] [ .txt ] ![ .csv .xls ] [ .bat .sh ] ![ .blk .yel ]" );
	const auto& includes = filter.GetIncludeFilter();
	const auto& excludes = filter.GetExcludeFilter();

	std::vector<std::string> expectedIncludes = { ".red", ".gr2", ".dds", ".png", ".yaml", ".txt", ".bat", ".sh" };
	std::vector<std::string> expectedExcludes = { ".csv", ".xls", ".blk", ".yel" };
	EXPECT_EQ( includes, expectedIncludes ) << "Include filters do not match expected values";
	EXPECT_EQ( excludes, expectedExcludes ) << "Exclude filters do not match expected values";
}

TEST_F( ResourceFilterTest, FilterResourceFilter_ApplyRule_SimpleIncludeFilterOnly )
{
	ResourceTools::FilterResourceFilter filter( "[ .red ]" );
	const auto& includes = filter.GetIncludeFilter();

	EXPECT_EQ( includes.size(), 1 );
	EXPECT_EQ( includes[0], ".red" );
}

TEST_F( ResourceFilterTest, FilterResourceFilter_ApplyRule_SimpleExcludeFilterOnly )
{
	ResourceTools::FilterResourceFilter filter( "![ .blk ]" );
	const auto& excludes = filter.GetExcludeFilter();

	EXPECT_EQ( excludes.size(), 1 );
	EXPECT_EQ( excludes[0], ".blk" );
}

TEST_F( ResourceFilterTest, FilterResourceFilter_ApplyRule_CombineIncludeExcludeIncludeFilters )
{
	// This test, combines multiple include and exclude filters, to test the logic of adding/removing filter elements:
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

TEST_F( ResourceFilterTest, FilterResourceFilter_CheckFailure_MissingClosingIncludeBracketBeforeNextOpenExcludeBracket )
{
	try
	{
		// This test filter has a missing closing bracket for the first include filter
		// before the next exclude filter starts.
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

TEST_F( ResourceFilterTest, FilterResourceFilter_CheckFailure_ExcludeMarkerWithoutOpenBracket )
{
	try
	{
		// This test filter has an exclude marker "!" but is missing the open bracket
		// for the exclude filter.
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

TEST_F( ResourceFilterTest, FilterResourceFilter_CheckFailure_ExcludeMarkerWithoutOpenBracketAfterValidIncludeFilter )
{
	try
	{
		// This test filter has a valid include filter, followed by an exclude marker "!"
		// but is missing the open bracket for the exclude filter.
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

TEST_F( ResourceFilterTest, FilterResourceFilter_CheckFailure_ExcludeMarkerWithoutBracketAfterValidIncludeAndExcludeFilters )
{
	try
	{
		// This test filter has a valid include and exclude filters,
		// followed by an exclude marker "!" and no filter definition after that.
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

TEST_F( ResourceFilterTest, FilterResourceFilter_CheckFailure_MissingOpeningIncludeBracketAtStart )
{
	try
	{
		// This test filter is missing the opening bracket for the include filter.
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

TEST_F( ResourceFilterTest, FilterResourceFilter_CheckFailure_MissingOpeningIncludeBracketForSecondIncludeFilter )
{
	try
	{
		// This test filter is missing the opening bracket for the second include filter,
		// after a valid first include filter.
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

TEST_F( ResourceFilterTest, FilterResourceFilter_CheckFailure_MissingClosingIncludeBracket )
{
	try
	{
		// This test filter is missing the closing bracket for the first and only include filter.
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

TEST_F( ResourceFilterTest, FilterResourceFilter_CheckFailure_MissingClosingIncludeBracketForSecondIncludeFilter )
{
	try
	{
		// This test filter is missing the closing bracket for the second include filter.
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

TEST_F( ResourceFilterTest, FilterResourceFilter_CheckFailure_MissingClosingIncludeBracketForMiddleIncludeFilter )
{
	try
	{
		// This test filter is missing the closing bracket for the middle include filter.
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

TEST_F( ResourceFilterTest, FilterResourceFilter_ApplyRule_UseCondensedValidIncludeExcludeIncludeFilters )
{
	// This test filter combines multiple include and exclude filters without extra whitespaces.
	// Done to test the logic of adding/removing filter elements and that the parsing logic is not dependent on whitespaces.
	ResourceTools::FilterResourceFilter filter( "[inToken1 inToken2]![exToken1 exToken2][inToken3]" );
	const auto& includes = filter.GetIncludeFilter();
	const auto& excludes = filter.GetExcludeFilter();

	std::vector<std::string> expectedIncludes = { "inToken1", "inToken2", "inToken3" };
	std::vector<std::string> expectedExcludes = { "exToken1", "exToken2" };
	EXPECT_EQ( includes, expectedIncludes );
	EXPECT_EQ( excludes, expectedExcludes );
}

TEST_F( ResourceFilterTest, FilterResourceFilter_ApplyRule_UseCondensedValidExcludeIncludeExcludeFilters )
{
	// This test filter combines multiple include and exclude filters without extra whitespaces.
	// Done to test the logic of adding/removing filter elements and that the parsing logic is not dependent on whitespaces.
	ResourceTools::FilterResourceFilter filter( "![exToken1][inToken1 inToken2]![exToken2][inToken3]" );
	const auto& includes = filter.GetIncludeFilter();
	const auto& excludes = filter.GetExcludeFilter();

	std::vector<std::string> expectedIncludes = { "inToken1", "inToken2", "inToken3" };
	std::vector<std::string> expectedExcludes = { "exToken1", "exToken2" };
	EXPECT_EQ( includes, expectedIncludes );
	EXPECT_EQ( excludes, expectedExcludes );
}

TEST_F( ResourceFilterTest, FilterResourceFilter_ApplyRule_EmptyTopLevelFilter )
{
	// When the top-level "filter" attribute of a NamedSection is empty,
	// a wild-card "*" should be added to the include filter by default.
	// Exclude filter should be empty though.
	ResourceTools::FilterResourceFilter filter( "", true );
	const auto& includes = filter.GetIncludeFilter();
	const auto& excludes = filter.GetExcludeFilter();

	// Wildcard "*" should be added when no include filter is specified for top-level "filter" attribute
	EXPECT_EQ( includes.size(), 1 );
	EXPECT_EQ( includes[0], "*" );

	EXPECT_TRUE( excludes.empty() );
}

TEST_F( ResourceFilterTest, FilterResourceFilter_ApplyRule_EmptyInLineFilter )
{
	// When an in-line filter of (respaths/resfile line) is empty (the default behavior),
	// no wildcard "*" should be added to the include filter.
	ResourceTools::FilterResourceFilter filter( "" );
	const auto& includes = filter.GetIncludeFilter();
	const auto& excludes = filter.GetExcludeFilter();

	// No wildcard should be added to an in-line include filter
	EXPECT_EQ( includes.size(), 0 );
	EXPECT_TRUE( includes.empty() );

	EXPECT_TRUE( excludes.empty() );
}

// -----------------------------------------

TEST_F( ResourceFilterTest, FilterPrefixMap_Validate_SinglePrefixWithMultiplePathsIsAllowed )
{
	// This test validates that a single prefix (prefix1) with multiple different paths
	// is correctly parsed and stored in the map.
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

TEST_F( ResourceFilterTest, FilterPrefixMap_Validate_MultipleDifferentPrefixesAreAllowed )
{
	// This test validates that multiple different prefixes (prefix1 & prefix2)
	// with their associated paths are correctly parsed and stored in the map.
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

TEST_F( ResourceFilterTest, FilterPrefixMap_Validate_DuplicateSamePrefixWithPathsInDifferentOrderIsAllowed )
{
	// This test validates that if the same prefix (prefix1) is defined multiple times,
	// with same paths but in different order (path1+path2 & path2+path1).
	// That the paths are combined and stored in the map without duplicates.
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

TEST_F( ResourceFilterTest, FilterPrefixMap_Validate_MultipleSamePrefixesCanAppendToPaths )
{
	// This test validates that if the same prefix (prefix1) is defined multiple times (first and last),
	// with different paths (path2+path1 & path3+path1), that the paths are combined and stored in the map without duplicates.
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

TEST_F( ResourceFilterTest, FilterPrefixMap_Validate_DifferentWhitespacesBetweenPrefixesAreAllowed )
{
	// This test validates that different whitespaces (space, tab, new line)
	// between prefix definitions are handled correctly.
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

TEST_F( ResourceFilterTest, FilterPrefixMap_CheckFailure_MissingColonAfterPrefixBeforePaths )
{
	// This test validates that if a prefix definition is missing a colon ":"
	// after the prefix and before the paths section, that an exception is thrown.
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

TEST_F( ResourceFilterTest, FilterPrefixMap_CheckFailure_MissingPrefixBeforePaths )
{
	// This test validates that if a prefix definition is missing the prefix itself
	// (lhs of colon) before the paths section, that an exception is thrown.
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

TEST_F( ResourceFilterTest, FilterPrefixMap_CheckFailure_MissingPathsAfterPrefix )
{
	// This test validates that if a prefix definition is missing the paths section
	// (rhs of colon) after the prefix, that an exception is thrown.
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

TEST_F( ResourceFilterTest, FilterPrefixMapEntry_CheckFailure_PrefixMissingInMapWhenAppendingPath )
{
	// This test validates that when trying to append paths to a FilterPrefixMapEntry
	// with a different prefix than the existing one of the mapEntry that an exception is thrown.
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

TEST_F( ResourceFilterTest, FilterPrefixMapEntry_CheckFailure_EmptyPathWhenAppendingPathToPrefix )
{
	// This test validates that when trying to append an empty path to a FilterPrefixMapEntry
	// that an exception is thrown, since empty paths are not allowed.
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

TEST_F( ResourceFilterTest, FilterDefaultSection_Validate_DefaultSectionWithMultiplePrefixesIsAllowed )
{
	// This test validates that a FilterDefaultSection can be initialized
	// with multiple different prefixes and their associated paths.
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

TEST_F( ResourceFilterTest, FilterDefaultSection_CheckFailure_InitializeWithMissingColonInPrefixmap )
{
	// This test validates that when trying to initialize a FilterDefaultSection
	// with a prefixmap string missing a colon ":" after the prefix definition
	// that an exception is thrown.
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

TEST_F( ResourceFilterTest, FilterDefaultSection_CheckFailure_InitializeWithEmptyPrefixInPrefixmap )
{
	// This test validates that when trying to initialize a FilterDefaultSection
	// with a prefixmap string missing the prefix definition (empty prefix)
	// that an exception is thrown.
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

// -------------------------------------------------------------
// Description:
//   Helper function, for tests in this file, to verify that all
//   expected paths are present in the resolved map.
// Arguments:
//   allExpectedPaths - Set of all expected paths to be found in the map
//   resolvedMap      - Map of resolved paths
//   messagePrefix    - Optional prefix to add to the failure message
// Return Value:
//   None (void)
// -------------------------------------------------------------
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

// -------------------------------------------------------------
// Description:
//   Helper function, for tests in this file, to validate that the resolved
//   path map contains all expected paths with the correct include/exclude filters.
// Arguments:
//   expectedPaths    - Set of expected paths to be found in the map
//   resolvedPathMap  - Map of resolved paths with their filters
//   expectedIncludes - Vector of expected include filters for each path
//   expectedExcludes - Vector of expected exclude filters for each path
//   messagePrefix    - Optional prefix to add to the failure message
// Return Value:
//   None (void)
// -------------------------------------------------------------
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

// -----------------------------------------

TEST_F( ResourceFilterTest, FilterResourcePathFile_Validate_SingleLineAttribute_WithNoInlineFilterIsAllowed )
{
	// This test validates that a FilterResourcePathFile can be initialized with a single line attribute
	// that contains a prefix and path, but no in-line filter.
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

TEST_F( ResourceFilterTest, FilterResourcePathFile_Validate_SingleLineAttribute_WithInlineFilterNoOverridesIsAllowed )
{
	// This test validates that a FilterResourcePathFile can be initialized with a single line attribute
	// that contains a prefix and path, with an in-line filter that does not override any of the parent filters.
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

TEST_F( ResourceFilterTest, FilterResourcePathFile_Validate_SingleLineAttribute_WithInlineFilterOverridingParentFilterIsAllowed )
{
	// This test validates that a FilterResourcePathFile can be initialized with a single
	// line attribute with an in-line filter that overrides some of the parent filters by
	// moving some filters from include to exclude and vice versa.
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

TEST_F( ResourceFilterTest, FilterResourcePathFile_Validate_MultiLineAttribute_WithMixedInlineFilterOverridesIsAllowed )
{
	// This test validates that a FilterResourcePathFile can be initialized with a multi-line attribute
	// that contains multiple prefixes and paths.
	// The in-line filters may override parent filters in different ways.
	// Some with no overrides, others with include/exclude filters switched around, etc.
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

TEST_F( ResourceFilterTest, FilterResourcePathFile_Validate_SingleLineAttribute_WithDuplicateIncludeExcludeInlineOverridesIsAllowed )
{
	// This test validates that a FilterResourcePathFile can be initialized with a single line attribute.
	// With an in-line filter that has duplicate include and exclude entries (some same as parent filters).
	// The final resolved filters should not contain any duplicates.
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

TEST_F( ResourceFilterTest, FilterResourcePathFile_CheckFailure_MissingPrefixThrowsException )
{
	// This test validates that when trying to initialize a FilterResourcePathFile with an
	// invalid raw attribute string (missing prefix), that an exception is thrown.
	std::string prefixMapStr = "prefix1:/path1;../path2 prefix2:/path3";
	std::string parentFilterStr = "[ .in1 .in2 ] ![ .ex1 ]";
	ResourceTools::FilterPrefixMap prefixMap( prefixMapStr );
	ResourceTools::FilterResourceFilter parentFilter( parentFilterStr );

	std::string rawPathFileAttrib = "/foo/bar"; // respath is missing the prefix:
	EXPECT_THROW( ResourceTools::FilterResourcePathFile pathFile( rawPathFileAttrib, prefixMap, parentFilter ), std::invalid_argument );
}

TEST_F( ResourceFilterTest, FilterResourcePathFile_CheckFailure_UnknownPrefixThrowsException )
{
	// This test validates that when trying to initialize a FilterResourcePathFile with an
	// invalid raw attribute string (unknown prefix, not in the prefix map), that an exception is thrown.
	std::string prefixMapStr = "prefix1:/path1;../path2 prefix2:/path3";
	std::string parentFilterStr = "[ .in1 .in2 ] ![ .ex1 ]";
	ResourceTools::FilterPrefixMap prefixMap( prefixMapStr );
	ResourceTools::FilterResourceFilter parentFilter( parentFilterStr );

	std::string rawPathFileAttrib = "prefixNotInPrefixMap:/foo/bar"; // unknown prefix
	EXPECT_THROW( ResourceTools::FilterResourcePathFile pathFile( rawPathFileAttrib, prefixMap, parentFilter ), std::invalid_argument );
}

TEST_F( ResourceFilterTest, FilterResourcePathFile_CheckFailure_MalformedInlineFilterThrowsException )
{
	// This test validates that when trying to initialize a FilterResourcePathFile with an
	// invalid raw attribute string (malformed in-line filter), that an exception is thrown.
	std::string prefixMapStr = "prefix1:/path1;../path2 prefix2:/path3";
	std::string parentFilterStr = "[ .in1 .in2 ] ![ .ex1 ]";
	ResourceTools::FilterPrefixMap prefixMap( prefixMapStr );
	ResourceTools::FilterResourceFilter parentFilter( parentFilterStr );

	std::string rawPathFileAttrib = "prefix1:/foo/bar [ .yaml "; // missing closing bracket of inline include filter
	EXPECT_THROW( ResourceTools::FilterResourcePathFile pathFile( rawPathFileAttrib, prefixMap, parentFilter ), std::invalid_argument );
}

// -----------------------------------------

TEST_F( ResourceFilterTest, FilterNamedSection_Validate_SingleLineRespathIsAllowed )
{
	// This test validates that a FilterNamedSection can be initialized with a single line respath.
	std::string sectionName = "FilterNamedSection_Validate_SingleLineRespathIsAllowed";
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

TEST_F( ResourceFilterTest, FilterNamedSection_Validate_EmptyTopLevelFilterIsAllowed )
{
	// This test validates that a FilterNamedSection can be initialized with an empty filter at top-level,
	// which should add a wildcard "*" include filter.
	std::string sectionName = "FilterNamedSection_Validate_EmptyTopLevelFilterIsAllowed";
	std::string defaultParentPrefixMapStr = "testPrefix:/myPath";
	std::string filter = ""; // Empty filter string at top-level should add wildcard include
	std::string respaths = "testPrefix:/foo/bar";

	ResourceTools::FilterPrefixMap defaultPrefixMap( defaultParentPrefixMapStr );
	ResourceTools::FilterNamedSection namedSection( sectionName, filter, respaths, "", defaultPrefixMap );

	// Expected values:
	std::set<std::string> expectedPaths = { "/myPath/foo/bar" };
	std::vector<std::string> expectedIncludes = { "*" };
	std::vector<std::string> expectedExcludes = {};

	const auto& resolvedRespathMap = namedSection.GetResolvedRespathsMap();
	const auto& resolvedResfileMap = namedSection.GetResolvedResfileMap();
	const auto& combinedMap = namedSection.GetCombinedResolvedPathMap();

	ValidatePathMap( expectedPaths, resolvedRespathMap, expectedIncludes, expectedExcludes, "ResolvedRespathsMap" );
	EXPECT_TRUE( resolvedResfileMap.empty() );
	ValidatePathMap( expectedPaths, combinedMap, expectedIncludes, expectedExcludes, "CombinedResolvedPathMap" );
}

TEST_F( ResourceFilterTest, FilterNamedSection_Validate_OnlyTopLevelExcludeFilterIsAllowed )
{
	// This test validates that a FilterNamedSection can be initialized with only an exclude filter at top-level,
	// which should add a wildcard "*" include filter.
	std::string sectionName = "FilterNamedSection_Validate_OnlyTopLevelExcludeFilterIsAllowed";
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

TEST_F( ResourceFilterTest, FilterNamedSection_Validate_MultiLineRespathWithSomeInlineOverridesIsAllowed )
{
	// This test validates that a FilterNamedSection can be initialized with a multi-line respath attribute.
	// Some lines may have in-line filters that override the top-level filter, while others just use the top-level filter as-is.
	std::string sectionName = "FilterNamedSection_Validate_MultiLineRespathWithSomeInlineOverridesIsAllowed";
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

TEST_F( ResourceFilterTest, FilterNamedSection_Validate_CombinedSingleLineRespathAndResfileAttributesIsAllowed )
{
	// This test validates that a FilterNamedSection can be initialized with both respath and resfile attributes.
	// The combined resolved map contains entries from both attributes.
	std::string sectionName = "FilterNamedSection_Validate_CombinedSingleLineRespathAndResfileAttributesIsAllowed";
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


TEST_F( ResourceFilterTest, FilterNamedSection_Validate_RespathWithoutResfileAttributeIsAllowed )
{
	// This test validates that a FilterNamedSection can be initialized with only
	// a respath attribute and no resfile attribute.
	std::string sectionName = "FilterNamedSection_Validate_RespathWithoutResfileAttributeIsAllowed";
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

TEST_F( ResourceFilterTest, FilterNamedSection_CheckFailure_MissingRespathAttributeThrowsException )
{
	// This test validates that when trying to initialize a FilterNamedSection
	// with a missing respath attribute, that an exception is thrown.
	std::string sectionName = "FilterNamedSection_CheckFailure_MissingRespathAttributeThrowsException";
	std::string defaultParentPrefixMapStr = "prefix1:/path1";
	std::string filter = "[ .in1 ]";
	std::string resfile = "prefix1:/foo/bar";

	ResourceTools::FilterPrefixMap defaultPrefixMap( defaultParentPrefixMapStr );

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

TEST_F( ResourceFilterTest, FilterNamedSection_Validate_CombinedResolvedMapOnSamePrefixIsAllowed )
{
	// This test validates that a FilterNamedSection can be initialized with both
	// respath and resfile attributes using the same prefix, and that the combined
	// resolved map contains entries from both attributes.
	std::string sectionName = "FilterNamedSection_Validate_CombinedResolvedMapOnSamePrefixIsAllowed";
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

TEST_F( ResourceFilterTest, FilterNamedSection_Validate_CombinedResolvedMapWithEmptyTopLevelFilterIsAllowed )
{
	// This test validates that a FilterNamedSection can be initialized with both
	// respath and resfile attributes using the same prefix, and an empty top-level filter.
	// The combined resolved map contains entries from both attributes, with wildcard "*" include filter.
	std::string sectionName = "FilterNamedSection_Validate_CombinedResolvedMapWithEmptyTopLevelFilterIsAllowed";
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

TEST_F( ResourceFilterTest, FilterNamedSection_Validate_CombinedResolvedMapWithOnlyExcludeTopLevelFilterIsAllowed )
{
	// This test validates that a FilterNamedSection can be initialized with both
	// respath and resfile attributes using the same prefix, and only an exclude filter at top-level.
	// The combined resolved map contains entries from both attributes, with wildcard "*" include filter.
	std::string sectionName = "FilterNamedSection_Validate_CombinedResolvedMapWithOnlyExcludeTopLevelFilterIsAllowed";
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

TEST_F( ResourceFilterTest, FilterNamedSection_Validate_EmptyTopLevelFilterWithRespathOverrideIsAllowed )
{
	// This test validates that a FilterNamedSection can be initialized with both
	// respath and resfile attributes using the same prefix, and an empty top-level filter.
	// The respath has in-line filters that override the top-level filter while the
	// resfile uses the default "*" wildcard top-level filter.
	std::string sectionName = "FilterNamedSection_Validate_EmptyTopLevelFilterWithRespathOverrideIsAllowed";
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

TEST_F( ResourceFilterTest, FilterNamedSection_Validate_EmptyTopLevelFilterWithResfileOverrideIsAllowed )
{
	// This test validates that a FilterNamedSection can be initialized with both
	// respath and resfile attributes using the same prefix, and an empty top-level filter.
	// The resfile has in-line filters that override the top-level filter while the
	// respath uses the default "*" wildcard top-level filter.
	std::string sectionName = "FilterNamedSection_Validate_EmptyTopLevelFilterWithResfileOverrideIsAllowed";
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

TEST_F( ResourceFilterTest, FilterNamedSection_Validate_OnlyExcludeTopLevelFilterWithRespathOverrideIsAllowed )
{
	// This test validates that a FilterNamedSection can be initialized with both
	// respath and resfile attributes using the same prefix, and only an exclude filter at top-level.
	// The respath has in-line filters that override the top-level filter while the
	// resfile uses the default "*" wildcard top-level filter.
	std::string sectionName = "FilterNamedSection_Validate_OnlyExcludeTopLevelFilterWithRespathOverrideIsAllowed";
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

TEST_F( ResourceFilterTest, FilterNamedSection_Validate_OnlyExcludeTopLevelFilterWithResfileOverrideIsAllowed )
{
	// This test validates that a FilterNamedSection can be initialized with both
	// respath and resfile attributes using the same prefix, and only an exclude filter at top-level.
	// The resfile has in-line filters that override the top-level filter while the
	// respath uses the default "*" wildcard top-level filter.
	std::string sectionName = "FilterNamedSection_Validate_OnlyExcludeTopLevelFilterWithResfileOverrideIsAllowed";
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

TEST_F( ResourceFilterTest, FilterNamedSection_Validate_CombinedMapWithResfileOverrideIsAllowed )
{
	// This test validates that a FilterNamedSection can be initialized with both
	// respath and resfile attributes using the same prefix, and that the combined
	// resolved map contains entries from both attributes, with the resfile filters
	// overriding the top-level filters.
	std::string sectionName = "FilterNamedSection_Validate_CombinedMapWithResfileOverrideIsAllowed";
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

TEST_F( ResourceFilterTest, FilterNamedSection_Validate_NoTopLevelFilterSameCombinedMapWithResfileOverrideIsAllowed )
{
	// This test validates that a FilterNamedSection can be initialized with both
	// respath and resfile attributes using the same prefix and only default "*" top-level filter.
	// The combined resolved map contains entries from both attributes, with the resfile
	// filters overriding the top-level filter.
	std::string sectionName = "FilterNamedSection_Validate_NoTopLevelFilterSameCombinedMapWithResfileOverrideIsAllowed";
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

TEST_F( ResourceFilterTest, FilterNamedSection_Validate_NoTopLevelFilterSameCombinedMapWithRespathOverrideIsAllowed )
{
	// This test validates that a FilterNamedSection can be initialized with both
	// respath and resfile attributes using the same prefix and only default "*" top-level filter.
	// The combined resolved map contains entries from both attributes, with the respath
	// filters overriding the top-level filter.
	std::string sectionName = "FilterNamedSection_Validate_NoTopLevelFilterSameCombinedMapWithRespathOverrideIsAllowed";
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

TEST_F( ResourceFilterTest, FilterNamedSection_Validate_OnlyExcludeFilterSameCombinedMapWithResfileOverrideIsAllowed )
{
	// This test validates that a FilterNamedSection can be initialized with both respath and resfile
	// attributes using the same prefix, and only an exclude filter at top-level (and default "*" include).
	// The combined resolved map contains entries from both attributes, with the resfile filters
	// overriding the top-level filter.
	std::string sectionName = "FilterNamedSection_Validate_OnlyExcludeFilterSameCombinedMapWithResfileOverrideIsAllowed";
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

TEST_F( ResourceFilterTest, FilterNamedSection_Validate_OnlyExcludeFilterSameCombinedMapWithRespathsOverrideIsAllowed )
{
	// This test validates that a FilterNamedSection can be initialized with both respath and resfile
	// attributes using the same prefix, and only an exclude filter at top-level (and default "*" include).
	// The combined resolved map contains entries from both attributes, with the respaths filters
	// overriding the top-level filter.
	std::string sectionName = "FilterNamedSection_Validate_OnlyExcludeFilterSameCombinedMapWithRespathsOverrideIsAllowed";
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

TEST_F( ResourceFilterTest, FilterResourceFile_ValidateSuccessfulFileLoad_example1_ini )
{
	// This test validates that the example1.ini file can be loaded successfully
	// and that the resolved paths match the expected values.
	const std::filesystem::path iniPath = GetTestFileFileAbsolutePath( "ExampleIniFiles/example1.ini" );
	try
	{
		ResourceTools::FilterResourceFile resourceFile( iniPath.generic_string() );
		const auto& iniFilePathMap = resourceFile.GetIniFileResolvedPathMap();

		// Validate the paths:
		std::set<std::string> expectedPaths = {
			// From the respaths attribute:
			"./Indicies/firstLine/...",           // "res:/firstLine/..."
			"./resourcesOnBranch/firstLine/...",  // "res:/firstLine/..."
			"./Indicies/secondLine/*",            // "res:/secondLine/*"
			"./resourcesOnBranch/secondLine/*",   // "res:/secondLine/*"
			"./ResourceGroups/thirdLine/...",     // "res2:/thirdLine/..."
			// From the resfile attribute:
			"./Indicies/binaryFileIndex_v0_0_0.txt",         // "res:/binaryFileIndex_v0_0_0.txt"
			"./resourcesOnBranch/binaryFileIndex_v0_0_0.txt" // "res:/binaryFileIndex_v0_0_0.txt"
		};
		std::vector<std::string> expectedIncludes = { ".yaml" };
		std::vector<std::string> expectedExcludes = {};

		MapContainsPaths( expectedPaths, iniFilePathMap, "FullResolvedPathMap from example1.ini" );
		ValidatePathMap( expectedPaths, iniFilePathMap, expectedIncludes, expectedExcludes, "FullResolvedPathMap from example1.ini" );
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

TEST_F( ResourceFilterTest, FilterResourceFile_ConfirmFileLoadFailure_invalidMissingDefaultSection_ini )
{
	// This test validates that loading an ini file missing the required [DEFAULT]
	// section throws an exception.
	const std::filesystem::path iniPath = GetTestFileFileAbsolutePath( "ExampleIniFiles/invalidMissingDefaultSection.ini" );
	try
	{
		ResourceTools::FilterResourceFile resourceFile( iniPath.generic_string() );
		FAIL() << "Expected std::invalid_argument when loading ini file missing [DEFAULT] section";
	}
	catch( const std::invalid_argument& e )
	{
		std::string expectedError = "Missing [DEFAULT] section in INI file: " + iniPath.generic_string();
		EXPECT_STREQ( e.what(), expectedError.c_str() );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument when loading ini file missing [DEFAULT] section";
	}
}

TEST_F( ResourceFilterTest, FilterResourceFile_ConfirmFileLoadFailure_invalidMissingNamedSection_ini )
{
	// This test validates that loading an ini file missing any named sections
	// throws an exception since at least one named section is required.
	const std::filesystem::path iniPath = GetTestFileFileAbsolutePath( "ExampleIniFiles/invalidMissingNamedSection.ini" );
	try
	{
		ResourceTools::FilterResourceFile resourceFile( iniPath.generic_string() );
		FAIL() << "Expected std::invalid_argument when loading ini file missing [NamedSection] section";
	}
	catch( const std::invalid_argument& e )
	{
		std::string expectedError = "No namedSections defined in INI file: " + iniPath.generic_string();
		EXPECT_STREQ( e.what(), expectedError.c_str() );
	}
	catch( ... )
	{
		FAIL() << "Expected std::invalid_argument when loading ini file missing [NamedSection] section";
	}
}

TEST_F( ResourceFilterTest, FilterResourceFile_ConfirmFileLoadFailure_iniFileDoesNotExist )
{
	// This test validates that loading a non-existent ini file throws an exception.
	const std::filesystem::path iniPath = GetTestFileFileAbsolutePath( "ExampleIniFiles/iniFileNotFound.ini" );
	try
	{
		ResourceTools::FilterResourceFile resourceFile( iniPath.generic_string() );
		FAIL() << "Expected std::runtime_error when loading non-existent ini file";
	}
	catch( const std::runtime_error& e )
	{
		std::string expectedError = "Failed to parse INI file: " + iniPath.generic_string() + " - unable to open file";
		EXPECT_STREQ( e.what(), expectedError.c_str() );
	}
	catch( ... )
	{
		FAIL() << "Expected std::runtime_error when loading non-existent ini file";
	}
}

TEST_F( ResourceFilterTest, FilterResourceFile_ConfirmFileLoadFailure_invalidPrefixmap_ini )
{
	// This test validates that loading an ini file with an invalid prefixmap format throws an exception.
	const std::filesystem::path iniPath = GetTestFileFileAbsolutePath( "ExampleIniFiles/invalidPrefixmap.ini" );
	try
	{
		ResourceTools::FilterResourceFile resourceFile( iniPath.generic_string() );
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

TEST_F( ResourceFilterTest, FilterResourceFile_ConfirmFileLoadFailure_invalidSectionFilter_ini )
{
	// This test validates that loading an ini file with an invalid section filter format throws an exception.
	const std::filesystem::path iniPath = GetTestFileFileAbsolutePath( "ExampleIniFiles/invalidSectionFilter.ini" );
	try
	{
		ResourceTools::FilterResourceFile resourceFile( iniPath.generic_string() );
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

TEST_F( ResourceFilterTest, FilterResourceFile_ConfirmFileLoadFailure_invalidInlineFilter_ini )
{
	// This test validates that loading an ini file with an invalid inline filter format throws an exception.
	const std::filesystem::path iniPath = GetTestFileFileAbsolutePath( "ExampleIniFiles/invalidInlineFilter.ini" );
	try
	{
		ResourceTools::FilterResourceFile resourceFile( iniPath.generic_string() );
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

TEST_F( ResourceFilterTest, FilterResourceFile_ConfirmFileLoadFailure_invalidPrefixMismatch_ini )
{
	// This test validates that loading an ini file with a prefix in the respaths/resfile
	// attributes that does not match any prefix in the prefixmap throws an exception.
	const std::filesystem::path iniPath = GetTestFileFileAbsolutePath( "ExampleIniFiles/invalidPrefixMismatch.ini" );
	try
	{
		ResourceTools::FilterResourceFile resourceFile( iniPath.generic_string() );
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

// ------------------------------------------

TEST_F( ResourceFilterTest, ResourceFilter_ConfirmFileLoadFailure_SingleIniFileThatDoesNotExist )
{
	// This test validates that initializing a ResourceFilter with a non-existent ini file throws an exception.
	const std::filesystem::path iniPath = GetTestFileFileAbsolutePath( "ExampleIniFiles/noSuchFile.ini" );
	std::vector<std::filesystem::path> paths = { iniPath };
	ResourceTools::ResourceFilter resourceFilter;
	try
	{
		resourceFilter.Initialize( paths );
		FAIL() << "Expected this test to fail: ResourceFilter_Load_SingleFile_ThatDoesNotExist";
	}
	catch( const std::exception& e )
	{
		std::string errorMessage = e.what();
		ASSERT_NE( errorMessage.find( "Unable to create ResourceFilter for:" ), std::string::npos );
		ASSERT_NE( errorMessage.find( "Failed to parse INI file" ), std::string::npos );
		ASSERT_NE( errorMessage.find( "unable to open file" ), std::string::npos );
	}
	catch( ... )
	{
		FAIL() << "Unknown exception thrown while initializing ResourceFilter with example1.ini";
	}
}

TEST_F( ResourceFilterTest, ResourceFilter_ConfirmFileLoadFailure_MultipleFilesOneThatDoesNotExist )
{
	// This test validates that initializing a ResourceFilter with multiple ini files
	// where one is valid and another does not exist, results in an exception being thrown.
	const std::filesystem::path iniPath1 = GetTestFileFileAbsolutePath( "ExampleIniFiles/example1.ini" );
	const std::filesystem::path iniPath2 = GetTestFileFileAbsolutePath( "ExampleIniFiles/noSuchFile.ini" );
	std::vector<std::filesystem::path> paths = { iniPath1, iniPath2 };
	ResourceTools::ResourceFilter resourceFilter;
	try
	{
		resourceFilter.Initialize( paths );
		FAIL() << "Expected this test to fail: ResourceFilter_Load_MultipleFiles_OneThatDoesNotExist";
	}
	catch( const std::exception& e )
	{
		std::string errorMessage = e.what();
		ASSERT_NE( errorMessage.find( "Unable to create ResourceFilter for:" ), std::string::npos );
		ASSERT_NE( errorMessage.find( "Failed to parse INI file" ), std::string::npos );
		ASSERT_NE( errorMessage.find( "unable to open file" ), std::string::npos );
	}
	catch( ... )
	{
		FAIL() << "Unknown exception thrown while initializing ResourceFilter with example1.ini";
	}
}

TEST_F( ResourceFilterTest, ResourceFilter_ConfirmSuccessfulFileLoad_example1_ini )
{
	// This test validates that initializing a ResourceFilter with a valid ini file (example1.ini)
	// successfully loads without throwing any exceptions.
	const std::filesystem::path iniPath1 = GetTestFileFileAbsolutePath( "ExampleIniFiles/example1.ini" );
	std::vector<std::filesystem::path> paths = { iniPath1 };
	ResourceTools::ResourceFilter resourceFilter;
	try
	{
		resourceFilter.Initialize( paths );
		ASSERT_TRUE( true ); // If we got here, the file loaded successfully without any exceptions
	}
	catch( const std::exception& e )
	{
		FAIL() << "Exception in test, should not have failed: " << e.what();
	}
	catch( ... )
	{
		FAIL() << "Unknown exception thrown while initializing ResourceFilter with example1.ini";
	}
}

TEST_F( ResourceFilterTest, ResourceFilter_Validate_RaiiClassCurrentWorkingDirectoryChanger_ChangesWorkingDirectoryForDurationOfTest )
{
	// This test validates that the RAII CurrentWorkingDirectoryChanger class correctly changes
	// the current working directory for the duration of the test.
	std::filesystem::path pathBeforeChange = std::filesystem::current_path();
	std::filesystem::path testDataPath = TEST_DATA_BASE_PATH;

	{
		// RAII class to change the current working directory for the duration of this test
		// Needed so both relative paths in the .ini file resolve correctly,
		// based on paths to the location of the example .ini files
		CurrentWorkingDirectoryChanger cwdRAII( TEST_DATA_BASE_PATH );

		const std::filesystem::path iniPath1 = "ExampleIniFiles/example1.ini";
		std::vector<std::filesystem::path> paths = { iniPath1 };
		ResourceTools::ResourceFilter resourceFilter;
		try
		{
			resourceFilter.Initialize( paths );
			ASSERT_EQ( resourceFilter.HasFilters(), true );
			ASSERT_EQ( resourceFilter.GetFullResolvedPathMap().size(), 7 );

			// Check that the "binaryFileIndex_v0_0_0.txt" file (and it's path) is included correctly
			std::filesystem::path oneValidRelativePath = "./Indicies/binaryFileIndex_v0_0_0.txt";
			ASSERT_EQ( resourceFilter.ShouldInclude( oneValidRelativePath ), true );
		}
		catch( const std::exception& e )
		{
			FAIL() << "Exception in test, should not have failed: " << e.what();
		}
		catch( ... )
		{
			FAIL() << "Unknown exception thrown while initializing ResourceFilter with example1.ini";
		}

		// Make sure the working directory is set to the testDataPath and not the original path
		ASSERT_EQ( std::filesystem::current_path().lexically_normal().string(), testDataPath.lexically_normal().string() ) << "Current working directory should be the TEST_DATA_BASE_PATH";
		ASSERT_NE( std::filesystem::current_path().lexically_normal().string(), pathBeforeChange.lexically_normal().string() ) << "Current working directory should not be same as at start of test";
	}

	// After the RAII object goes out of scope, the working directory should be restored to the original path
	ASSERT_EQ( std::filesystem::current_path().lexically_normal().string(), pathBeforeChange.lexically_normal().string() ) << "Current working directory should have been restored";
	ASSERT_NE( std::filesystem::current_path().lexically_normal().string(), testDataPath.lexically_normal().string() ) << "Current working directory should not be the TEST_DATA_BASE_PATH";
}

TEST_F( ResourceFilterTest, ResourceFilter_ValidateSuccessfulFileLoadUsingRelativePaths_validSimpleExample1_ini )
{
	// This test validates that initializing a ResourceFilter with a valid ini file (validSimpleExample1.ini),
	// using relative paths, loads successfully and the expected paths and filters are present.

	// Alter the current working directory for the duration of this test
	CurrentWorkingDirectoryChanger cwdRAII( TEST_DATA_BASE_PATH );
	try
	{
		const std::filesystem::path iniPath1 = "ExampleIniFiles/validSimpleExample1.ini";
		std::vector<std::filesystem::path> paths = { iniPath1 };
		ResourceTools::ResourceFilter resourceFilter;
		resourceFilter.Initialize( paths );

		// Validate correct included paths via the resourceFilter:
		std::set<std::filesystem::path> validResolvedRelativePaths = {
			"resourcesOnBranch/introMovie.txt",
			"resourcesOnBranch/videoCardCategories.yaml"
		};

		ASSERT_EQ( resourceFilter.HasFilters(), true );
		for( const auto& resolvedRelativePath : validResolvedRelativePaths )
		{
			ASSERT_EQ( resourceFilter.ShouldInclude( resolvedRelativePath ), true ) << "Should have included relative path: " << resolvedRelativePath.generic_string();
		}

		// Additional check to make sure the FullResolvedPathMap contains correct data (either include or exclude):
		const auto& fullPathMap = resourceFilter.GetFullResolvedPathMap();
		ASSERT_EQ( fullPathMap.size(), 1 );

		std::set<std::string> expectedPaths = {
			"./resourcesOnBranch/*"
		};
		std::vector<std::string> expectedIncludes = { ".yaml", ".txt" };
		std::vector<std::string> expectedExcludes = {};

		MapContainsPaths( expectedPaths, fullPathMap, "FullResolvedPathMap from validSimpleExample1.ini" );
		ValidatePathMap( expectedPaths, fullPathMap, expectedIncludes, expectedExcludes, "FullResolvedPathMap from validSimpleExample1.ini" );
	}
	catch( ... )
	{
		FAIL() << "Test [ResourceFilter_Load_validSimpleExample1_ini] failed when it should have passed.";
	}
}

TEST_F( ResourceFilterTest, ResourceFilter_ValidateSuccessfulFileLoadUsingAbsolutePaths_validSimpleExample1_ini )
{
	// This test validates that initializing a ResourceFilter with a valid ini file (validSimpleExample1.ini),
	// using absolute paths, loads successfully and the expected paths and filters are present.

	// Alter the current working directory for the duration of this test
	CurrentWorkingDirectoryChanger cwdRAII( TEST_DATA_BASE_PATH );
	try
	{
		const std::filesystem::path iniPath1 = "ExampleIniFiles/validSimpleExample1.ini";
		std::filesystem::path iniPath1Abs = std::filesystem::absolute( iniPath1 );
		std::vector<std::filesystem::path> paths = { iniPath1Abs };
		ResourceTools::ResourceFilter resourceFilter;
		resourceFilter.Initialize( paths );

		// Validate correct included paths via the resourceFilter:
		std::set<std::filesystem::path> validResolvedAbsolutePaths = {
			std::filesystem::absolute( "resourcesOnBranch/introMovie.txt" ),
			std::filesystem::absolute( "resourcesOnBranch/videoCardCategories.yaml" )
		};

		ASSERT_EQ( resourceFilter.HasFilters(), true );
		for( const auto& resolvedAbsPath : validResolvedAbsolutePaths )
		{
			ASSERT_EQ( resourceFilter.ShouldInclude( resolvedAbsPath ), true ) << "Should have included absolute path: " << resolvedAbsPath.generic_string();
		}

		// Additional check to make sure the FullResolvedPathMap contains correct data (either include or exclude):
		const auto& fullPathMap = resourceFilter.GetFullResolvedPathMap();
		ASSERT_EQ( fullPathMap.size(), 1 );

		std::set<std::string> expectedPaths = {
			"./resourcesOnBranch/*"
		};
		std::vector<std::string> expectedIncludes = { ".yaml", ".txt" };
		std::vector<std::string> expectedExcludes = {};

		MapContainsPaths( expectedPaths, fullPathMap, "FullResolvedPathMap from validSimpleExample1.ini" );
		ValidatePathMap( expectedPaths, fullPathMap, expectedIncludes, expectedExcludes, "FullResolvedPathMap from validSimpleExample1.ini" );
	}
	catch( ... )
	{
		FAIL() << "Test [ResourceFilter_Load_validSimpleExample1_ini_usingAbsolutePaths] failed when it should have passed.";
	}
}

TEST_F( ResourceFilterTest, ResourceFilter_ValidateSuccessfulFileLoadUsingRelativePaths_validComplexExample1_ini )
{
	// This test validates that initializing a ResourceFilter with a valid ini file (validComplexExample1.ini),
	// using relative paths, loads successfully and the expected paths and filters are present.

	// Alter the current working directory for the duration of this test
	CurrentWorkingDirectoryChanger cwdRAII( TEST_DATA_BASE_PATH );
	try
	{
		const std::filesystem::path iniPath1 = "ExampleIniFiles/validComplexExample1.ini";
		std::vector<std::filesystem::path> paths = { iniPath1 };
		ResourceTools::ResourceFilter resourceFilter;
		resourceFilter.Initialize( paths );

		// Validate correct included paths via the resourceFilter:
		std::set<std::filesystem::path> validResolvedRelativePaths = {
			//"PatchWithInputChunk/NextBuildResources/introMovie.txt", // resRoot:/PatchWithInputChunk/... ![ Movie ]
			"PatchWithInputChunk/NextBuildResources/introMoviePrefixed.txt", // resRoot:/PatchWithInputChunk/... ![ Movie ] + resLocalCDN:/../NextBuildResources/introMoviePrefixed.txt
			//"PatchWithInputChunk/NextBuildResources/introMovieSomewhatChanged.txt", // resRoot:/PatchWithInputChunk/... ![ Movie ]
			"PatchWithInputChunk/NextBuildResources/testResource2.txt",
			"PatchWithInputChunk/NextBuildResources/videoCardCategories.yaml",
			"PatchWithInputChunk/PreviousBuildResources/introMovie.txt", // resRoot:/PatchWithInputChunk/... ![ Movie ] + resPrevious:/* [ Movie ]
			"PatchWithInputChunk/PreviousBuildResources/introMoviePrefixed.txt", // resRoot:/PatchWithInputChunk/... ![ Movie ] + resPrevious:/* [ Movie ]
			"PatchWithInputChunk/PreviousBuildResources/introMovieSomewhatChanged.txt", // resRoot:/PatchWithInputChunk/... ![ Movie ] + resPrevious:/* [ Movie ]
			//"PatchWithInputChunk/PreviousBuildResources/testResource.txt", // resRoot:/PatchWithInputChunk/PreviousBuildResources/* ![ testResource.txt ]
			"PatchWithInputChunk/PreviousBuildResources/videoCardCategories.yaml",
			"PatchWithInputChunk/PatchResourceGroup_previousBuild_latestBuild.yaml",
			"PatchWithInputChunk/resFileIndexShort_build_next.txt",
			"PatchWithInputChunk/resFileIndexShort_build_previous.txt",
		};

		ASSERT_EQ( resourceFilter.HasFilters(), true );
		for( const auto& resolvedRelativePath : validResolvedRelativePaths )
		{
			ASSERT_EQ( resourceFilter.ShouldInclude( resolvedRelativePath ), true ) << "Should have included relative path: " << resolvedRelativePath.generic_string();
		}

		// Additional check to make sure the FullResolvedPathMap contains correct data (either include or exclude):
		std::set<std::string> expectedPaths = {
			"PatchWithInputChunk/...",
			"./PatchWithInputChunk/...",
			"PatchWithInputChunk/PreviousBuildResources/*",
			"PatchWithInputChunk/LocalCDNPatches/../NextBuildResources/introMoviePrefixed.txt",
			"./PatchWithInputChunk/PreviousBuildResources/*"
		};
		const auto& fullPathMap = resourceFilter.GetFullResolvedPathMap();
		ASSERT_EQ( fullPathMap.size(), expectedPaths.size() );
		MapContainsPaths( expectedPaths, fullPathMap, "FullResolvedPathMap from validSimpleExample1.ini" );

		// Manually validate the fullPathMap, as it has several different prefixPathCombos + some inline filter overrides
		for( const auto& kv : fullPathMap )
		{
			if( kv.first == "PatchWithInputChunk/..." )
			{
				EXPECT_EQ( kv.second.GetIncludeFilter().size(), 2 );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".yaml" ) != kv.second.GetIncludeFilter().end() );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".txt" ) != kv.second.GetIncludeFilter().end() );

				EXPECT_EQ( kv.second.GetExcludeFilter().size(), 0 );
			}
			else if( kv.first == "./PatchWithInputChunk/..." )
			{
				EXPECT_EQ( kv.second.GetIncludeFilter().size(), 2 );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".yaml" ) != kv.second.GetIncludeFilter().end() );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".txt" ) != kv.second.GetIncludeFilter().end() );

				EXPECT_EQ( kv.second.GetExcludeFilter().size(), 1 );
				EXPECT_EQ( kv.second.GetExcludeFilter()[0], "Movie" );
			}
			else if( kv.first == "PatchWithInputChunk/PreviousBuildResources/*" )
			{
				EXPECT_EQ( kv.second.GetIncludeFilter().size(), 3 );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".yaml" ) != kv.second.GetIncludeFilter().end() );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".txt" ) != kv.second.GetIncludeFilter().end() );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), "Movie" ) != kv.second.GetIncludeFilter().end() );

				EXPECT_EQ( kv.second.GetExcludeFilter().size(), 0 );
			}
			else if( kv.first == "PatchWithInputChunk/LocalCDNPatches/../NextBuildResources/introMoviePrefixed.txt" )
			{
				EXPECT_EQ( kv.second.GetIncludeFilter().size(), 2 );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".yaml" ) != kv.second.GetIncludeFilter().end() );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".txt" ) != kv.second.GetIncludeFilter().end() );

				EXPECT_EQ( kv.second.GetExcludeFilter().size(), 0 );
			}
			else if( kv.first == "./PatchWithInputChunk/PreviousBuildResources/*" )
			{
				EXPECT_EQ( kv.second.GetIncludeFilter().size(), 2 );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".yaml" ) != kv.second.GetIncludeFilter().end() );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".txt" ) != kv.second.GetIncludeFilter().end() );

				EXPECT_EQ( kv.second.GetExcludeFilter().size(), 1 );
				EXPECT_EQ( kv.second.GetExcludeFilter()[0], "testResource.txt" );
			}
			else
			{
				FAIL() << "Unexpected path found in FullResolvedPathMap: " << kv.first;
			}
		}
	}
	catch( const std::exception& e )
	{
		FAIL() << "Test [ResourceFilter_Load_validComplexExample1_ini_usingRelativePaths] failed with: " << e.what();
	}
	catch( ... )
	{
		FAIL() << "Test [ResourceFilter_Load_validComplexExample1_ini_usingRelativePaths] failed when it should have passed.";
	}
}

TEST_F( ResourceFilterTest, ResourceFilter_ValidateSuccessfulFileLoadUsingAbsolutePath_validComplexExample1_ini )
{
	// This test validates that initializing a ResourceFilter with a valid ini file (validComplexExample1.ini),
	// using absolute paths, loads successfully and the expected paths and filters are present.

	// Alter the current working directory for the duration of this test
	CurrentWorkingDirectoryChanger cwdRAII( TEST_DATA_BASE_PATH );
	try
	{
		const std::filesystem::path iniPath1 = "ExampleIniFiles/validComplexExample1.ini";
		std::vector<std::filesystem::path> pathsAbs = { std::filesystem::absolute( iniPath1 ) };
		ResourceTools::ResourceFilter resourceFilter;
		resourceFilter.Initialize( pathsAbs );

		// Validate correct included paths via the resourceFilter:
		std::set<std::filesystem::path> validResolvedRelativePaths = {
			//"PatchWithInputChunk/NextBuildResources/introMovie.txt", // resRoot:/PatchWithInputChunk/... ![ Movie ]
			std::filesystem::absolute( "PatchWithInputChunk/NextBuildResources/introMoviePrefixed.txt" ), // resRoot:/PatchWithInputChunk/... ![ Movie ] + resLocalCDN:/../NextBuildResources/introMoviePrefixed.txt
			//"PatchWithInputChunk/NextBuildResources/introMovieSomewhatChanged.txt", // resRoot:/PatchWithInputChunk/... ![ Movie ]
			std::filesystem::absolute( "PatchWithInputChunk/NextBuildResources/testResource2.txt" ),
			std::filesystem::absolute( "PatchWithInputChunk/NextBuildResources/videoCardCategories.yaml" ),
			std::filesystem::absolute( "PatchWithInputChunk/PreviousBuildResources/introMovie.txt" ), // resRoot:/PatchWithInputChunk/... ![ Movie ] + resPrevious:/* [ Movie ]
			std::filesystem::absolute( "PatchWithInputChunk/PreviousBuildResources/introMoviePrefixed.txt" ), // resRoot:/PatchWithInputChunk/... ![ Movie ] + resPrevious:/* [ Movie ]
			std::filesystem::absolute( "PatchWithInputChunk/PreviousBuildResources/introMovieSomewhatChanged.txt" ), // resRoot:/PatchWithInputChunk/... ![ Movie ] + resPrevious:/* [ Movie ]
			//"PatchWithInputChunk/PreviousBuildResources/testResource.txt", // resRoot:/PatchWithInputChunk/PreviousBuildResources/* ![ testResource.txt ]
			std::filesystem::absolute( "PatchWithInputChunk/PreviousBuildResources/videoCardCategories.yaml" ),
			std::filesystem::absolute( "PatchWithInputChunk/PatchResourceGroup_previousBuild_latestBuild.yaml" ),
			std::filesystem::absolute( "PatchWithInputChunk/resFileIndexShort_build_next.txt" ),
			std::filesystem::absolute( "PatchWithInputChunk/resFileIndexShort_build_previous.txt" ),
		};

		ASSERT_EQ( resourceFilter.HasFilters(), true );
		for( const auto& resolvedAbsPath : validResolvedRelativePaths )
		{
			ASSERT_EQ( resourceFilter.ShouldInclude( resolvedAbsPath ), true ) << "Should have included relative path: " << resolvedAbsPath.generic_string();
		}

		// Additional check to make sure the FullResolvedPathMap contains correct data (either include or exclude):
		std::set<std::string> expectedPaths = {
			"PatchWithInputChunk/...",
			"./PatchWithInputChunk/...",
			"PatchWithInputChunk/PreviousBuildResources/*",
			"PatchWithInputChunk/LocalCDNPatches/../NextBuildResources/introMoviePrefixed.txt",
			"./PatchWithInputChunk/PreviousBuildResources/*"
		};
		const auto& fullPathMap = resourceFilter.GetFullResolvedPathMap();
		ASSERT_EQ( fullPathMap.size(), expectedPaths.size() );
		MapContainsPaths( expectedPaths, fullPathMap, "FullResolvedPathMap from validSimpleExample1.ini" );

		// Manually validate the fullPathMap, as it has several different prefixPathCombos + some inline filter overrides
		for( const auto& kv : fullPathMap )
		{
			if( kv.first == "PatchWithInputChunk/..." )
			{
				EXPECT_EQ( kv.second.GetIncludeFilter().size(), 2 );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".yaml" ) != kv.second.GetIncludeFilter().end() );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".txt" ) != kv.second.GetIncludeFilter().end() );

				EXPECT_EQ( kv.second.GetExcludeFilter().size(), 0 );
			}
			else if( kv.first == "./PatchWithInputChunk/..." )
			{
				EXPECT_EQ( kv.second.GetIncludeFilter().size(), 2 );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".yaml" ) != kv.second.GetIncludeFilter().end() );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".txt" ) != kv.second.GetIncludeFilter().end() );

				EXPECT_EQ( kv.second.GetExcludeFilter().size(), 1 );
				EXPECT_EQ( kv.second.GetExcludeFilter()[0], "Movie" );
			}
			else if( kv.first == "PatchWithInputChunk/PreviousBuildResources/*" )
			{
				EXPECT_EQ( kv.second.GetIncludeFilter().size(), 3 );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".yaml" ) != kv.second.GetIncludeFilter().end() );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".txt" ) != kv.second.GetIncludeFilter().end() );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), "Movie" ) != kv.second.GetIncludeFilter().end() );

				EXPECT_EQ( kv.second.GetExcludeFilter().size(), 0 );
			}
			else if( kv.first == "PatchWithInputChunk/LocalCDNPatches/../NextBuildResources/introMoviePrefixed.txt" )
			{
				EXPECT_EQ( kv.second.GetIncludeFilter().size(), 2 );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".yaml" ) != kv.second.GetIncludeFilter().end() );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".txt" ) != kv.second.GetIncludeFilter().end() );

				EXPECT_EQ( kv.second.GetExcludeFilter().size(), 0 );
			}
			else if( kv.first == "./PatchWithInputChunk/PreviousBuildResources/*" )
			{
				EXPECT_EQ( kv.second.GetIncludeFilter().size(), 2 );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".yaml" ) != kv.second.GetIncludeFilter().end() );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".txt" ) != kv.second.GetIncludeFilter().end() );

				EXPECT_EQ( kv.second.GetExcludeFilter().size(), 1 );
				EXPECT_EQ( kv.second.GetExcludeFilter()[0], "testResource.txt" );
			}
			else
			{
				FAIL() << "Unexpected path found in FullResolvedPathMap: " << kv.first;
			}
		}
	}
	catch( const std::exception& e )
	{
		FAIL() << "Test [ResourceFilter_Load_validComplexExample1_ini_usingRelativePaths] failed with: " << e.what();
	}
	catch( ... )
	{
		FAIL() << "Test [ResourceFilter_Load_validComplexExample1_ini_usingRelativePaths] failed when it should have passed.";
	}
}

TEST_F( ResourceFilterTest, ResourceFilter_ValidateSuccessfulLoadOf2IniFiles_validComplexExample1_and_validSimpleExample1 )
{
	// This test validates that initializing a ResourceFilter with two valid ini files
	// (validComplexExample1.ini and validSimpleExample1.ini), using relative paths,
	// loads successfully and the expected paths and filters from both ini files are
	// present in the resulting ResourceFilter.

	// Alter the current working directory for the duration of this test
	CurrentWorkingDirectoryChanger cwdRAII( TEST_DATA_BASE_PATH );
	try
	{
		std::vector<std::filesystem::path> paths = {
			"ExampleIniFiles/validComplexExample1.ini",
			"ExampleIniFiles/validSimpleExample1.ini"
		};
		ResourceTools::ResourceFilter resourceFilter;
		resourceFilter.Initialize( paths );

		// Validate correct included paths via the resourceFilter:
		std::set<std::filesystem::path> validResolvedRelativePaths = {
			// From validComplexExample1:
			//"PatchWithInputChunk/NextBuildResources/introMovie.txt", // resRoot:/PatchWithInputChunk/... ![ Movie ]
			"PatchWithInputChunk/NextBuildResources/introMoviePrefixed.txt", // resRoot:/PatchWithInputChunk/... ![ Movie ] + resLocalCDN:/../NextBuildResources/introMoviePrefixed.txt
			//"PatchWithInputChunk/NextBuildResources/introMovieSomewhatChanged.txt", // resRoot:/PatchWithInputChunk/... ![ Movie ]
			"PatchWithInputChunk/NextBuildResources/testResource2.txt",
			"PatchWithInputChunk/NextBuildResources/videoCardCategories.yaml",
			"PatchWithInputChunk/PreviousBuildResources/introMovie.txt", // resRoot:/PatchWithInputChunk/... ![ Movie ] + resPrevious:/* [ Movie ]
			"PatchWithInputChunk/PreviousBuildResources/introMoviePrefixed.txt", // resRoot:/PatchWithInputChunk/... ![ Movie ] + resPrevious:/* [ Movie ]
			"PatchWithInputChunk/PreviousBuildResources/introMovieSomewhatChanged.txt", // resRoot:/PatchWithInputChunk/... ![ Movie ] + resPrevious:/* [ Movie ]
			//"PatchWithInputChunk/PreviousBuildResources/testResource.txt", // resRoot:/PatchWithInputChunk/PreviousBuildResources/* ![ testResource.txt ]
			"PatchWithInputChunk/PreviousBuildResources/videoCardCategories.yaml",
			"PatchWithInputChunk/PatchResourceGroup_previousBuild_latestBuild.yaml",
			"PatchWithInputChunk/resFileIndexShort_build_next.txt",
			"PatchWithInputChunk/resFileIndexShort_build_previous.txt",
			// From validSimpleExample1.ini:
			"resourcesOnBranch/introMovie.txt",
			"resourcesOnBranch/videoCardCategories.yaml"
		};

		ASSERT_EQ( resourceFilter.HasFilters(), true );
		for( const auto& resolvedRelativePath : validResolvedRelativePaths )
		{
			ASSERT_EQ( resourceFilter.ShouldInclude( resolvedRelativePath ), true ) << "Should have included relative path: " << resolvedRelativePath.generic_string();
		}

		// Additional check to make sure the FullResolvedPathMap contains correct data (either include or exclude):
		std::set<std::string> expectedPaths = {
			"PatchWithInputChunk/...",
			"./PatchWithInputChunk/...",
			"PatchWithInputChunk/PreviousBuildResources/*",
			"PatchWithInputChunk/LocalCDNPatches/../NextBuildResources/introMoviePrefixed.txt",
			"./PatchWithInputChunk/PreviousBuildResources/*",
			"./resourcesOnBranch/*"
		};
		const auto& fullPathMap = resourceFilter.GetFullResolvedPathMap();
		ASSERT_EQ( fullPathMap.size(), expectedPaths.size() );
		MapContainsPaths( expectedPaths, fullPathMap, "FullResolvedPathMap from two ini files" );

		// Manually validate the fullPathMap, as it has several different prefixPathCombos + some inline filter overrides
		for( const auto& kv : fullPathMap )
		{
			if( kv.first == "PatchWithInputChunk/..." )
			{
				EXPECT_EQ( kv.second.GetIncludeFilter().size(), 2 );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".yaml" ) != kv.second.GetIncludeFilter().end() );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".txt" ) != kv.second.GetIncludeFilter().end() );

				EXPECT_EQ( kv.second.GetExcludeFilter().size(), 0 );
			}
			else if( kv.first == "./PatchWithInputChunk/..." )
			{
				EXPECT_EQ( kv.second.GetIncludeFilter().size(), 2 );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".yaml" ) != kv.second.GetIncludeFilter().end() );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".txt" ) != kv.second.GetIncludeFilter().end() );

				EXPECT_EQ( kv.second.GetExcludeFilter().size(), 1 );
				EXPECT_EQ( kv.second.GetExcludeFilter()[0], "Movie" );
			}
			else if( kv.first == "PatchWithInputChunk/PreviousBuildResources/*" )
			{
				EXPECT_EQ( kv.second.GetIncludeFilter().size(), 3 );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".yaml" ) != kv.second.GetIncludeFilter().end() );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".txt" ) != kv.second.GetIncludeFilter().end() );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), "Movie" ) != kv.second.GetIncludeFilter().end() );

				EXPECT_EQ( kv.second.GetExcludeFilter().size(), 0 );
			}
			else if( kv.first == "PatchWithInputChunk/LocalCDNPatches/../NextBuildResources/introMoviePrefixed.txt" )
			{
				EXPECT_EQ( kv.second.GetIncludeFilter().size(), 2 );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".yaml" ) != kv.second.GetIncludeFilter().end() );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".txt" ) != kv.second.GetIncludeFilter().end() );

				EXPECT_EQ( kv.second.GetExcludeFilter().size(), 0 );
			}
			else if( kv.first == "./PatchWithInputChunk/PreviousBuildResources/*" )
			{
				EXPECT_EQ( kv.second.GetIncludeFilter().size(), 2 );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".yaml" ) != kv.second.GetIncludeFilter().end() );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".txt" ) != kv.second.GetIncludeFilter().end() );

				EXPECT_EQ( kv.second.GetExcludeFilter().size(), 1 );
				EXPECT_EQ( kv.second.GetExcludeFilter()[0], "testResource.txt" );
			}
			else if( kv.first == "./resourcesOnBranch/*" )
			{
				EXPECT_EQ( kv.second.GetIncludeFilter().size(), 2 );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".yaml" ) != kv.second.GetIncludeFilter().end() );
				EXPECT_TRUE( std::find( kv.second.GetIncludeFilter().begin(), kv.second.GetIncludeFilter().end(), ".txt" ) != kv.second.GetIncludeFilter().end() );

				EXPECT_EQ( kv.second.GetExcludeFilter().size(), 0 );
			}
			else
			{
				FAIL() << "Unexpected path found in FullResolvedPathMap: " << kv.first;
			}
		}
	}
	catch( const std::exception& e )
	{
		FAIL() << "Test [ResourceFilter_Load2iniFiles_validComplexExample1_and_validSimpleExample1] failed with: " << e.what();
	}
	catch( ... )
	{
		FAIL() << "Test [ResourceFilter_Load2iniFiles_validComplexExample1_and_validSimpleExample1] failed when it should have passed.";
	}
}