// Copyright © 2025 CCP ehf.

#include "ResourceFilterTest.h"
#include <INIReader.h>

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
