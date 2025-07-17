// Copyright © 2025 CCP ehf.

#include "CliTestFixture.h"

struct CarbonResourcesCliTest : public CliTestFixture{};

TEST_F( CarbonResourcesCliTest, RunWithoutArguments )
{
	std::string output;

	std::vector<std::string> arguments;

	int res = RunCli( arguments, output );

    // Expect 4 which indicates failed with no command specified
    ASSERT_EQ( res, 4 );
}

TEST_F( CarbonResourcesCliTest, RunWithNonesenseArguments )
{
	std::string output;

	std::vector<std::string> arguments;

    arguments.push_back( "Nonesense" );

	int res = RunCli( arguments, output );

    // Expect 3 which indicates failed due to invalid operation
	ASSERT_EQ( res, 3 );
}

TEST_F( CarbonResourcesCliTest, RunCreateGroupWithNoArguments )
{
	std::string output;

	std::vector<std::string> arguments;

	arguments.push_back( "create-group" );

	int res = RunCli( arguments, output );

    // Expect 2 which failed due to invalid operation arguments
	ASSERT_EQ( res, 2 );
}

TEST_F( CarbonResourcesCliTest, RunCreatePatchWithNoArguments )
{
	std::string output;

	std::vector<std::string> arguments;

	arguments.push_back( "create-patch" );

	int res = RunCli( arguments, output );

    // Expect 2 which failed due to invalid operation arguments
	ASSERT_EQ( res, 2 );
}

TEST_F( CarbonResourcesCliTest, RunCreateBundleWithNoArguments )
{
	std::string output;

	std::vector<std::string> arguments;

	arguments.push_back( "create-bundle" );

	int res = RunCli( arguments, output );

    // Expect 2 which failed due to invalid operation arguments
	ASSERT_EQ( res, 2 );
}

#ifdef DEV_FEATURES

TEST_F( CarbonResourcesCliTest, RunApplyPatchWithNoArguments )
{
	std::string output;

	std::vector<std::string> arguments;

	arguments.push_back( "apply-patch" );

	int res = RunCli( arguments, output );

    // Expect 2 which failed due to invalid operation arguments
	ASSERT_EQ( res, 2 );
}

TEST_F( CarbonResourcesCliTest, RunUnpackBundleWithNoArguments )
{
	std::string output;

	std::vector<std::string> arguments;

	arguments.push_back( "unpack-bundle" );

	int res = RunCli( arguments, output );

    // Expect 2 which failed due to invalid operation arguments
	ASSERT_EQ( res, 2 );
}

TEST_F( CarbonResourcesCliTest, CreateOperationWithInvalidInput )
{

	std::string output;

	std::vector<std::string> arguments;

	arguments.push_back( "create-group" );

	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "3" );

	std::filesystem::path inputDirectory = "INVALID_PATH";
	arguments.push_back( inputDirectory.string() );

	int res = RunCli( arguments, output );

    // Expect return 1 indicating failed during valid operation
	ASSERT_EQ( res, 1 );

}

#endif
TEST_F( CarbonResourcesCliTest, CreateResourceGroupFromDirectory )
{
	std::string output;

	std::vector<std::string> arguments;

	arguments.push_back( "create-group" );

	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "3" );

	std::filesystem::path inputDirectory = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceFiles" );
	arguments.push_back( inputDirectory.string() );

	arguments.push_back( "--output-file" );
	std::filesystem::path outputFile = "GroupOut/ResourceGroup.yaml";
	arguments.push_back( outputFile.string() );

	int res = RunCli( arguments, output );

    ASSERT_EQ( res, 0 );

#if _WIN64
    std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceGroupWindows.yaml" );
#elif __APPLE__
    std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceGroupMacOS.yaml" );
#else
#error Unsupported platform
#endif
    EXPECT_TRUE( FilesMatch( goldFile, "GroupOut/ResourceGroup.yaml" ) );
}

TEST_F( CarbonResourcesCliTest, CreateResourceGroupFromDirectoryOldDocumentFormat )
{
	std::string output;

	std::vector<std::string> arguments;

	arguments.push_back( "create-group" );

	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "3" );

	std::filesystem::path inputDirectory = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceFiles" );
	arguments.push_back( inputDirectory.string() );

	arguments.push_back( "--output-file" );
	std::filesystem::path outputFile = "GroupOut/ResourceGroup.csv";
	arguments.push_back( outputFile.string() );

	arguments.push_back( "--document-version" );
	arguments.push_back( "0.0.0" );

	int res = RunCli( arguments, output );

    ASSERT_EQ( res, 0 );

#if _WIN64
    std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceGroupWindows.csv" );
#elif __APPLE__
    std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceGroupMacOS.csv" );
#else
#error Unsupported platform
#endif
    EXPECT_TRUE( FilesMatch( goldFile, "GroupOut/ResourceGroup.csv" ) );
}

TEST_F( CarbonResourcesCliTest, CreateResourceGroupFromDirectoryOldDocumentFormatWithPrefix )
{
	std::string output;

	std::vector<std::string> arguments;

	arguments.push_back( "create-group" );

	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "3" );

	std::filesystem::path inputDirectory = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceFiles" );
	arguments.push_back( inputDirectory.string() );

	arguments.push_back( "--output-file" );
	std::filesystem::path outputFile = "GroupOut/ResourceGroupPrefixed.csv";
	arguments.push_back( outputFile.string() );

	arguments.push_back( "--document-version" );
	arguments.push_back( "0.0.0" );

	arguments.push_back( "--resource-prefix" );
	arguments.push_back( "test" );

	int res = RunCli( arguments, output );

	ASSERT_EQ( res, 0 );

#if _WIN64
    std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceGroupWindowsPrefixed.csv" );
#elif __APPLE__
    std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceGroupMacOSPrefixed.csv" );
#else
#error Unsupported platform
#endif
    EXPECT_TRUE( FilesMatch( goldFile, "GroupOut/ResourceGroupPrefixed.csv" ) );
}

TEST_F( CarbonResourcesCliTest, CreateBundle )
{
	std::string output;

    std::vector<std::string> arguments;

    arguments.push_back( "create-bundle" );

	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "3" );

	arguments.push_back( GetTestFileFileAbsolutePath( "Bundle/resfileindexShort.txt" ).string() );

	arguments.push_back( "--resource-source-path" );
	arguments.push_back( GetTestFileFileAbsolutePath( "Bundle/Res" ).string() );

	arguments.push_back( "--bundle-resourcegroup-relative-path" );
	arguments.push_back( "BundleResourceGroup.yaml" );

    arguments.push_back( "--bundle-resourcegroup-destination-path" );
	arguments.push_back( "BundleOut/" );

	arguments.push_back( "--bundle-resourcegroup-destination-type" );
	arguments.push_back( "LOCAL_RELATIVE" );

	arguments.push_back( "--chunk-destination-path" );
	arguments.push_back( "CreateBundleOut" );

	arguments.push_back( "--chunk-destination-type" );
	arguments.push_back( "LOCAL_CDN" );

	arguments.push_back( "--chunk-size");
	arguments.push_back( "1000" );


	int res = RunCli( arguments, output );

	EXPECT_EQ( res, 0 );

	// Check expected outcome
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "CreateBundle/BundleResourceGroup.yaml" );
	EXPECT_TRUE( FilesMatch( goldFile, "BundleOut/BundleResourceGroup.yaml" ) );

	std::filesystem::path goldDirectory = GetTestFileFileAbsolutePath( "CreateBundle/CreateBundleOut" );
	EXPECT_TRUE( DirectoryIsSubset( goldDirectory, "CreateBundleOut" ) );
}

TEST_F( CarbonResourcesCliTest, CreatePatch )
{
	std::string output;

	std::vector<std::string> arguments;

	arguments.push_back( "create-patch" );

	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "3" );

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

	arguments.push_back( "--patch-resourcegroup-destination-path");
	arguments.push_back( "PatchOut" );

	arguments.push_back( "--patch-destination-base-path" );
	arguments.push_back( "Patchout/Patches" );

	arguments.push_back( "--patch-destination-type" );
	arguments.push_back( "LOCAL_CDN" );

	arguments.push_back( "--chunk-size" );
	arguments.push_back( "50000000" );

	int res = RunCli( arguments, output );

	EXPECT_EQ( res, 0 );

	// Check expected outcome
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "Patch/PatchResourceGroup.yaml" );
	EXPECT_TRUE( FilesMatch( goldFile, "PatchOut/PatchResourceGroup.yaml" ) );

	std::filesystem::path goldDirectory = GetTestFileFileAbsolutePath( "Patch/LocalCDNPatches" );
	EXPECT_TRUE( DirectoryIsSubset( goldDirectory, "PatchOut/Patches" ) );
}

TEST_F( CarbonResourcesCliTest, CreateGroup )
{
	std::string output;

	std::vector<std::string> arguments;

	arguments.push_back( "create-group" );

	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "3" );

	std::string directoryIn = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceFiles" ).string();

	arguments.push_back( directoryIn );

    arguments.push_back( "--output-file" );

    std::string outputFilename = "CarbonResourcesCliTestResourceGroup.yaml";

    arguments.push_back( outputFilename );

	int res = RunCli( arguments, output );

	EXPECT_EQ( res, 0 );

    // Check expected outcome
    #if _WIN64
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceGroupWindows.yaml" );
#elif __APPLE__
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceGroupMacOS.yaml" );
#else
#error Unsupported platform
#endif
	EXPECT_TRUE( FilesMatch( goldFile, outputFilename ) );
}

#ifdef DEV_FEATURES

TEST_F( CarbonResourcesCliTest, ApplyPatch )
{
	std::string output;

	std::vector<std::string> arguments;

	arguments.push_back( "apply-patch" );

	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "3" );

	std::string directoryIn = GetTestFileFileAbsolutePath( "Patch/PatchResourceGroup.yaml" ).string();

	arguments.push_back( directoryIn );

    arguments.push_back( "--patch-binaries-base-path" );
    
    std::string patchBinariesBasePath = GetTestFileFileAbsolutePath( "Patch/LocalCDNPatches/" ).string();
	
    arguments.push_back( patchBinariesBasePath );

    arguments.push_back( "--resources-to-patch-base-path" );
    
    std::string resourcesToPatchBasePath = GetTestFileFileAbsolutePath( "Patch/PreviousBuildResources/" ).string();

    arguments.push_back( resourcesToPatchBasePath );

    arguments.push_back( "--next-resources-base-path" );

    std::string nextResourcesBasePath = GetTestFileFileAbsolutePath( "Patch/NextBuildResources/" ).string();

    arguments.push_back( nextResourcesBasePath );

    arguments.push_back( "--output-base-path" );

    std::string outputBasePath = "ApplyPatchOut";

    arguments.push_back( outputBasePath );
    
    if( std::filesystem::exists( outputBasePath ) )
	{
		std::filesystem::remove_all( outputBasePath );
	}

    std::filesystem::copy( resourcesToPatchBasePath, outputBasePath );

	int res = RunCli( arguments, output );

	EXPECT_EQ( res, 0 );

    // Check expected outcome
	std::filesystem::path goldDirectory = GetTestFileFileAbsolutePath( "Patch/NextBuildResources" );
	EXPECT_TRUE( DirectoryIsSubset( outputBasePath, goldDirectory ) );
}

TEST_F( CarbonResourcesCliTest, UnpackBundle )
{
	std::string output;

	std::vector<std::string> arguments;

	arguments.push_back( "unpack-bundle" );

	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "3" );

	std::string directoryIn = GetTestFileFileAbsolutePath( "Bundle/BundleResourceGroup.yaml" ).string();

	arguments.push_back( directoryIn );

    arguments.push_back( "--chunk-source-base-path" );

    std::string chunkSourceBasePath = GetTestFileFileAbsolutePath( "Bundle/LocalRemoteChunks/" ).string();

    arguments.push_back( chunkSourceBasePath );

    arguments.push_back( "--resource-destination-type" );

    arguments.push_back( "LOCAL_RELATIVE" );

	int res = RunCli( arguments, output );

	EXPECT_EQ( res, 0 );

	// Check expected outcome
	EXPECT_TRUE( DirectoryIsSubset( GetTestFileFileAbsolutePath( "Bundle/Res" ), "UnpackBundleOut" ) );

	EXPECT_TRUE( std::filesystem::exists( "UnpackBundleOut/ResourceGroup.yaml" ) );
}

#endif