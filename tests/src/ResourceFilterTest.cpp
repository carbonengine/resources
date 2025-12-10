// Copyright © 2025 CCP ehf.

#include "ResourceFilterTest.h"
#include <INIReader.h>
#include <FilterResourceFilter.h>

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
	ASSERT_TRUE( reader.HasSection( "testyamlfilesovermultilinerespaths" ) );
	EXPECT_EQ( reader.Get( "testyamlfilesovermultilinerespaths", "filter", "" ), "[ .yaml ]" );
	EXPECT_EQ( reader.Get( "testyamlfilesovermultilinerespaths", "resfile", "" ), "res:/binaryFileIndex_v0_0_0.txt" );
	EXPECT_EQ( reader.Get( "testyamlfilesovermultilinerespaths", "respaths", "" ), "res:/...\nres2:/..." );
	EXPECT_EQ( reader.Keys( "testyamlfilesovermultilinerespaths" ).size(), 3 );
}

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