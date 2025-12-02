#include "ResourceFilterTest.h"
#include <INIReader.h>

TEST_F(ResourceFilterTestFixture, Example1IniParsing_v1)
{
	// Path to the example ini file
	const std::string iniPath = "../../../tests/testData/ExampleIniFiles/example1.ini";
	INIReader reader(iniPath);
	ASSERT_EQ(reader.ParseError(), 0) << "Failed to parse example1.ini";

	// Check [default] section
	ASSERT_TRUE(reader.HasSection("default"));
	EXPECT_EQ(reader.Get("default", "prefixmap", ""), "res:../../../tests/testData/Indicies;../../../tests/testData/resourcesOnBranch res2:../../../tests/testData/ResourceGroups");
	EXPECT_EQ(reader.Get("default", "version", ""), "1.2");

	// Check [testyamlfilesovermultilinerespaths] section
	ASSERT_TRUE(reader.HasSection("testyamlfilesovermultilinerespaths"));
	EXPECT_EQ(reader.Get("testyamlfilesovermultilinerespaths", "filter", ""), "[ .yaml ]");
	EXPECT_EQ(reader.Get("testyamlfilesovermultilinerespaths", "respaths", ""), "res:/...\nres2:/...");
	EXPECT_EQ(reader.Get("testyamlfilesovermultilinerespaths", "resfile", ""), "res:/binaryFileIndex_v0_0_0.txt");
}

TEST_F(ResourceFilterTestFixture, Example1IniParsing_v2)
{
	// Next step.
	// Do the same test as above, but now:
	// - Change the class to inherit from ResourcesTestFixture and make use of the helper functions there for file paths
	// - Get a list of all the sections and check if they match with the example1.ini file. Check the count. Do lowercase comparison on section names.
	// - For each section, get the list of keys and check if they match. Check the count. Do lowercase comparison on key names.
	// - For each key, check the value matches expected.
	//   - Where there are multiple lines or list of items, add them to a sorted vector and then compare.

}
