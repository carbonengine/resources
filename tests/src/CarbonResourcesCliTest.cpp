#include "CliTestFixture.h"

struct CarbonResourcesCliTest : public CliTestFixture{};

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

	arguments.push_back( "PATH_TO_RESOURCELIST_TO_BUNDLE" );

	int res = RunCli( arguments, output );

	EXPECT_TRUE( PatchIsValid() );
}