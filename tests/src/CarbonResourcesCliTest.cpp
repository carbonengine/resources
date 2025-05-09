#include "CliTestFixture.h"

struct CarbonResourcesCliTest : public CliTestFixture{};

//TODO: Implement
TEST_F( CarbonResourcesCliTest, CreateResourceGroupFromDirectory )
{
	std::string output;

	std::vector<std::string> arguments;

	arguments.push_back( "create-group" );

	arguments.push_back( "PATH_TO_RESOURCELIST_TO_BUNDLE" );

	int res = RunCli( arguments, output );

	EXPECT_TRUE( BundleIsValid() );
}

//TODO: Implement
TEST_F( CarbonResourcesCliTest, CreateBundle )
{
	std::string output;

    std::vector<std::string> arguments;

    arguments.push_back( "create-bundle" );

	arguments.push_back( "PATH_TO_RESOURCELIST_TO_BUNDLE" );

	int res = RunCli( arguments, output );

    EXPECT_TRUE( BundleIsValid() );
}

TEST_F( CarbonResourcesCliTest, CreatePatch )
{
	std::string output;

	std::vector<std::string> arguments;

	arguments.push_back( "create-patch" );

	arguments.push_back( "-VVV" );

	std::string previousResourceGroupPath = GetTestFileFileAbsolutePath( "Patch/resfileindexShort_build_previous.txt" ).string();

	arguments.push_back( previousResourceGroupPath );

	std::string nextResourceGroupPath = GetTestFileFileAbsolutePath( "Patch/resfileindexShort_build_next.txt" ).string();

	arguments.push_back( nextResourceGroupPath );

	arguments.push_back( "--resource-source-type-previous" );
	arguments.push_back( "LOCAL_RELATIVE" );

	std::string nextResourcesLocation = GetTestFileFileAbsolutePath( "Patch/NextBuildResources" ).string();

	arguments.push_back( "--resource-source-base-path-next");
	arguments.push_back( nextResourcesLocation );

	std::string previousResourcesLocation = GetTestFileFileAbsolutePath( "Patch/PreviousBuildResources" ).string();

	arguments.push_back( "--resource-source-base-path-previous");
	arguments.push_back( previousResourcesLocation );

	int res = RunCli( arguments, output );

	EXPECT_EQ( res, 0 );

	// Check expected outcome
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "Patch/PatchResourceGroup.yaml" );
	EXPECT_TRUE( FilesMatch( goldFile, "PatchOut/PatchResourceGroup.yaml" ) );

	std::filesystem::path goldDirectory = GetTestFileFileAbsolutePath( "Patch/LocalCDNPatches" );
	EXPECT_TRUE( DirectoryIsSubset( goldDirectory, "PatchOut/Patches" ) );
}