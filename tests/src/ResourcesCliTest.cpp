// Copyright © 2025 CCP ehf.

#include "CliTestFixture.h"

struct ResourcesCliTest : public CliTestFixture
{
};

TEST_F( ResourcesCliTest, RunWithoutArguments )
{
	std::vector<std::string> arguments;

	int res = RunCli( arguments );

	// Expect 4 which indicates failed with no command specified
	ASSERT_EQ( res, 4 );
}

TEST_F( ResourcesCliTest, RunWithNonesenseArguments )
{
	std::vector<std::string> arguments;

	arguments.push_back( "Nonesense" );

	int res = RunCli( arguments );

	// Expect 3 which indicates failed due to invalid operation
	ASSERT_EQ( res, 3 );
}

TEST_F( ResourcesCliTest, RunCreateGroupWithNoArguments )
{
	std::vector<std::string> arguments;

	arguments.push_back( "create-group" );

	int res = RunCli( arguments );

	// Expect 2 which failed due to invalid operation arguments
	ASSERT_EQ( res, 2 );
}

TEST_F( ResourcesCliTest, RunCreatePatchWithNoArguments )
{
	std::vector<std::string> arguments;

	arguments.push_back( "create-patch" );

	int res = RunCli( arguments );

	// Expect 2 which failed due to invalid operation arguments
	ASSERT_EQ( res, 2 );
}

TEST_F( ResourcesCliTest, RunCreateBundleWithNoArguments )
{
	std::vector<std::string> arguments;

	arguments.push_back( "create-bundle" );

	int res = RunCli( arguments );

	// Expect 2 which failed due to invalid operation arguments
	ASSERT_EQ( res, 2 );
}

#ifdef DEV_FEATURES

TEST_F( ResourcesCliTest, RunApplyPatchWithNoArguments )
{
	std::vector<std::string> arguments;

	arguments.push_back( "apply-patch" );

	int res = RunCli( arguments );

	// Expect 2 which failed due to invalid operation arguments
	ASSERT_EQ( res, 2 );
}

TEST_F( ResourcesCliTest, RunUnpackBundleWithNoArguments )
{
	std::vector<std::string> arguments;

	arguments.push_back( "unpack-bundle" );

	int res = RunCli( arguments );

	// Expect 2 which failed due to invalid operation arguments
	ASSERT_EQ( res, 2 );
}

TEST_F( ResourcesCliTest, CreateOperationWithInvalidInput )
{
	std::vector<std::string> arguments;

	arguments.push_back( "create-group" );

	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "-1" );

	std::filesystem::path inputDirectory = "INVALID_PATH";
	arguments.push_back( inputDirectory.string() );

	int res = RunCli( arguments );

	// Expect return 1 indicating failed during valid operation
	ASSERT_EQ( res, 1 );
}

#endif
TEST_F( ResourcesCliTest, CreateResourceGroupFromDirectory )
{
	std::string errorOutput;
	std::vector<std::string> arguments;

	arguments.push_back( "create-group" );

	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "-1" );

	std::filesystem::path inputDirectory = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceFiles" );
	arguments.push_back( inputDirectory.string() );

	arguments.push_back( "--output-file" );
	std::filesystem::path outputFile = "GroupOut/ResourceGroup.yaml";
	arguments.push_back( outputFile.string() );

	int res = RunCli( arguments, nullptr, &errorOutput );

	ASSERT_EQ( res, 0 ) << "CLI operation failed, errorOutput: " << errorOutput;

#if _WIN64
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceGroupWindows.yaml" );
#elif __APPLE__
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceGroupMacOS.yaml" );
#else
#error Unsupported platform
#endif
	EXPECT_TRUE( FilesMatch( goldFile, outputFile ) );
}

TEST_F( ResourcesCliTest, CreateResourceGroupFromDirectoryExportResources )
{
	std::string errorOutput;
	std::vector<std::string> arguments;

	arguments.push_back( "create-group" );

	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "-1" );

	arguments.push_back( "--export-resources" );

	arguments.push_back( "--export-resources-destination-type" );
	arguments.push_back( "LOCAL_RELATIVE" );

	arguments.push_back( "--export-resources-destination-path" );
	std::string exportOutputPath = "ExportedResources";
	arguments.push_back( exportOutputPath );

	std::filesystem::path inputDirectory = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceFiles" );
	arguments.push_back( inputDirectory.string() );

	arguments.push_back( "--output-file" );
	std::filesystem::path outputFile = "GroupOut/ResourceGroup.yaml";
	arguments.push_back( outputFile.string() );

	int res = RunCli( arguments, nullptr, &errorOutput );

	ASSERT_EQ( res, 0 ) << "CLI operation failed, errorOutput: " << errorOutput;

#if _WIN64
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceGroupWindows.yaml" );
#elif __APPLE__
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceGroupMacOS.yaml" );
#else
#error Unsupported platform
#endif
	EXPECT_TRUE( FilesMatch( goldFile, outputFile ) );

	EXPECT_TRUE( DirectoryIsSubset( exportOutputPath, inputDirectory ) );
}

TEST_F( ResourcesCliTest, CreateResourceGroupFromDirectoryWithSkipCompression )
{
	std::string errorOutput;
	std::vector<std::string> arguments;

	arguments.push_back( "create-group" );

	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "-1" );

	arguments.push_back( "--skip-compression" );

	std::filesystem::path inputDirectory = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceFiles" );
	arguments.push_back( inputDirectory.string() );

	arguments.push_back( "--output-file" );
	std::filesystem::path outputFile = "GroupOut/ResourceGroup.yaml";
	arguments.push_back( outputFile.string() );

	int res = RunCli( arguments, nullptr, &errorOutput );

	ASSERT_EQ( res, 0 ) << "CLI operation failed, errorOutput: " << errorOutput;

#if _WIN64
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceGroupSkipCompressionWindows.yaml" );
#elif __APPLE__
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceGroupSkipCompressionMacOS.yaml" );
#else
#error Unsupported platform
#endif
	EXPECT_TRUE( FilesMatch( goldFile, outputFile ) );
}

TEST_F( ResourcesCliTest, CreateResourceGroupFromDirectoryOldDocumentFormat )
{
	std::string errorOutput;
	std::vector<std::string> arguments;

	arguments.push_back( "create-group" );

	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "-1" );

	std::filesystem::path inputDirectory = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceFiles" );
	arguments.push_back( inputDirectory.string() );

	arguments.push_back( "--output-file" );
	std::filesystem::path outputFile = "GroupOut/ResourceGroup.csv";
	arguments.push_back( outputFile.string() );

	arguments.push_back( "--document-version" );
	arguments.push_back( "0.0.0" );

	int res = RunCli( arguments, nullptr, &errorOutput );

	ASSERT_EQ( res, 0 ) << "CLI operation failed, errorOutput: " << errorOutput;

#if _WIN64
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceGroupWindows.csv" );
#elif __APPLE__
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceGroupMacOS.csv" );
#else
#error Unsupported platform
#endif
	EXPECT_TRUE( FilesMatch( goldFile, outputFile ) );
}

TEST_F( ResourcesCliTest, CreateResourceGroupFromDirectoryOldDocumentFormatWithPrefix )
{
	std::string errorOutput;
	std::vector<std::string> arguments;

	arguments.push_back( "create-group" );

	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "-1" );

	std::filesystem::path inputDirectory = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceFiles" );
	arguments.push_back( inputDirectory.string() );

	arguments.push_back( "--output-file" );
	std::filesystem::path outputFile = "GroupOut/ResourceGroupPrefixed.csv";
	arguments.push_back( outputFile.string() );

	arguments.push_back( "--document-version" );
	arguments.push_back( "0.0.0" );

	arguments.push_back( "--resource-prefix" );
	arguments.push_back( "test" );

	int res = RunCli( arguments, nullptr, &errorOutput );

	ASSERT_EQ( res, 0 ) << "CLI operation failed, errorOutput: " << errorOutput;

#if _WIN64
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceGroupWindowsPrefixed.csv" );
#elif __APPLE__
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceGroupMacOSPrefixed.csv" );
#else
#error Unsupported platform
#endif
	EXPECT_TRUE( FilesMatch( goldFile, outputFile ) );
}

//---------------------------------------

TEST_F( ResourcesCliTest, CreateGroup_UsingFilter_validSimpleExample1_YamlOutput )
{
	// Setup test parameters
	std::string output;
	std::string errorOutput;
	std::vector<std::string> arguments;
	std::filesystem::path inputDirectoryPath = GetTestFileFileAbsolutePath( "" ); // The base testData directory
	std::filesystem::path outputFilePath = std::filesystem::absolute( "CliFilterCreateGroupOut/CreateGroup_UsingFilter_validSimpleExample1.yaml" );
	std::filesystem::path filterIniFilePath = "ExampleIniFiles/validSimpleExample1.ini";

	// Ensure any previous test output files are removed
	RemoveFiles( { outputFilePath } );

	arguments.push_back( "create-group" );
	arguments.push_back( inputDirectoryPath.lexically_normal().string() );
	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "3" );
	arguments.push_back( "--filter-file" );
	arguments.push_back( filterIniFilePath.lexically_normal().string() );
	arguments.push_back( "--filter-file-basepath" );
	arguments.push_back( TEST_DATA_BASE_PATH );
	arguments.push_back( "--output-file" );
	arguments.push_back( outputFilePath.lexically_normal().string() );
	arguments.push_back( "--document-version" );
	arguments.push_back( "0.1.0" );  // This is default YAML document version (setting it explicitly for clarity in test)

	int res = RunCli( arguments, &output, &errorOutput );
	std::cout << "--- RunCli() output: ---" << std::endl;
	std::cout << output << std::endl;
	std::cout << "------------------------" << std::endl;

	ASSERT_EQ( res, 0 ) << "CLI operation failed, errorOutput: " << errorOutput;

	// Check expected outcome
#if _WIN64
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "ExpectedTestOutputFiles/CreateGroup_UsingFilter_validSimpleExample1_Windows.yaml" );
#elif __APPLE__
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "ExpectedTestOutputFiles/CreateGroup_UsingFilter_validSimpleExample1_macOS.yaml" );
#else
#error Unsupported platform
#endif
	EXPECT_TRUE( FilesMatch( goldFile, outputFilePath ) ) << " Output file does not match expected gold file.";
}

TEST_F( ResourcesCliTest, CreateGroup_UsingFilter_validSimpleExample1_CsvTxtOutput )
{
	// Setup test parameters
	std::string output;
	std::string errorOutput;
	std::vector<std::string> arguments;
	std::filesystem::path inputDirectoryPath = GetTestFileFileAbsolutePath( "" ); // The base testData directory
	std::filesystem::path outputFilePath = std::filesystem::absolute( "CliFilterCreateGroupOut/CreateGroup_UsingFilter_validSimpleExample1.txt" );
	std::filesystem::path filterIniFilePath = "ExampleIniFiles/validSimpleExample1.ini";

	// Ensure any previous test output files are removed
	RemoveFiles( { outputFilePath } );

	arguments.push_back( "create-group" );
	arguments.push_back( inputDirectoryPath.lexically_normal().string() );
	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "3" );
	arguments.push_back( "--filter-file" );
	arguments.push_back( filterIniFilePath.lexically_normal().string() );
	arguments.push_back( "--filter-file-basepath" );
	arguments.push_back( TEST_DATA_BASE_PATH );
	arguments.push_back( "--output-file" );
	arguments.push_back( outputFilePath.lexically_normal().string() );
	arguments.push_back( "--document-version" );
	arguments.push_back( "0.0.0" );  // This is the "old style" csv (txt) document version

	int res = RunCli( arguments, &output, &errorOutput );
	std::cout << "--- RunCli() output: ---" << std::endl;
	std::cout << output << std::endl;
	std::cout << "------------------------" << std::endl;

	ASSERT_EQ( res, 0 ) << "CLI operation failed, errorOutput: " << errorOutput;

	// Check expected outcome
#if _WIN64
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "ExpectedTestOutputFiles/CreateGroup_UsingFilter_validSimpleExample1_Windows.txt" );
#elif __APPLE__
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "ExpectedTestOutputFiles/CreateGroup_UsingFilter_validSimpleExample1_macOS.txt" );
#else
#error Unsupported platform
#endif
	EXPECT_TRUE( FilesMatch( goldFile, outputFilePath ) ) << " Output file does not match expected gold file.";
}

TEST_F( ResourcesCliTest, CreateGroup_ConfirmWorks_UsingFilter_validSimpleExample1_WithRelativeFilterFilePathAndBasePathSet )
{
	// Setup test parameters
	std::string output;
	std::string errorOutput;
	std::vector<std::string> arguments;
	std::filesystem::path inputDirectoryPath = GetTestFileFileAbsolutePath( "" ); // The base testData directory
	std::filesystem::path outputFilePath = std::filesystem::absolute( "CliFilterCreateGroupOut/CreateGroup_ConfirmWorks_UsingFilter_validSimpleExample1_WithRelativeFilterFilePathAndBasePathSet.txt" );
	std::filesystem::path filterIniFilePath = "ExampleIniFiles/validSimpleExample1.ini";

	// Ensure any previous test output files are removed
	RemoveFiles( { outputFilePath } );

	arguments.push_back( "create-group" );
	arguments.push_back( inputDirectoryPath.lexically_normal().string() );
	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "3" );
	arguments.push_back( "--filter-file" );
	arguments.push_back( filterIniFilePath.lexically_normal().string() );
	arguments.push_back( "--filter-file-basepath" );
	arguments.push_back( TEST_DATA_BASE_PATH );
	arguments.push_back( "--output-file" );
	arguments.push_back( outputFilePath.lexically_normal().string() );
	arguments.push_back( "--document-version" );
	arguments.push_back( "0.0.0" );  // This is the "old style" csv (txt) document version

	int res = RunCli( arguments, &output, &errorOutput );
	std::cout << "--- RunCli() output: ---" << std::endl;
	std::cout << output << std::endl;
	std::cout << "------------------------" << std::endl;

	ASSERT_EQ( res, 0 ) << "CLI operation failed, errorOutput: " << errorOutput;

	// Check expected outcome
#if _WIN64
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "ExpectedTestOutputFiles/CreateGroup_UsingFilter_validSimpleExample1_Windows.txt" );
#elif __APPLE__
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "ExpectedTestOutputFiles/CreateGroup_UsingFilter_validSimpleExample1_macOS.txt" );
#else
#error Unsupported platform
#endif
	EXPECT_TRUE( FilesMatch( goldFile, outputFilePath ) ) << " Output file does not match expected gold file.";
}

TEST_F( ResourcesCliTest, CreateGroup_ConfirmWorks_UsingFilter_validSimpleExample1_WithAbsoluteFilterFilePathAndBasePathSet )
{
	// Setup test parameters
	std::string output;
	std::string errorOutput;
	std::vector<std::string> arguments;
	std::filesystem::path inputDirectoryPath = GetTestFileFileAbsolutePath( "" ); // The base testData directory
	std::filesystem::path outputFilePath = std::filesystem::absolute( "CliFilterCreateGroupOut/CreateGroup_ConfirmWorks_UsingFilter_validSimpleExample1_WithAbsoluteFilterFilePathAndBasePathSet.txt" );
	std::filesystem::path filterIniFilePath = GetTestFileFileAbsolutePath( "ExampleIniFiles/validSimpleExample1.ini" );

	// Ensure any previous test output files are removed
	RemoveFiles( { outputFilePath } );

	arguments.push_back( "create-group" );
	arguments.push_back( inputDirectoryPath.lexically_normal().string() );
	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "3" );
	arguments.push_back( "--filter-file" );
	arguments.push_back( filterIniFilePath.lexically_normal().string() );
	arguments.push_back( "--filter-file-basepath" );
	arguments.push_back( TEST_DATA_BASE_PATH );
	arguments.push_back( "--output-file" );
	arguments.push_back( outputFilePath.lexically_normal().string() );
	arguments.push_back( "--document-version" );
	arguments.push_back( "0.0.0" );  // This is the "old style" csv (txt) document version

	int res = RunCli( arguments, &output, &errorOutput );
	std::cout << "--- RunCli() output: ---" << std::endl;
	std::cout << output << std::endl;
	std::cout << "------------------------" << std::endl;

	ASSERT_EQ( res, 0 ) << "CLI operation failed, errorOutput: " << errorOutput;

	// Check expected outcome
#if _WIN64
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "ExpectedTestOutputFiles/CreateGroup_UsingFilter_validSimpleExample1_Windows.txt" );
#elif __APPLE__
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "ExpectedTestOutputFiles/CreateGroup_UsingFilter_validSimpleExample1_macOS.txt" );
#else
#error Unsupported platform
#endif
	EXPECT_TRUE( FilesMatch( goldFile, outputFilePath ) ) << " Output file does not match expected gold file.";
}

TEST_F( ResourcesCliTest, CreateGroup_ConfirmFails_UsingFilter_validSimpleExample1_WithRelativeFilterFilePathAndWrongBasePathSet )
{
	// Setup test parameters
	std::string output;
	std::string errorOutput;
	std::vector<std::string> arguments;
	std::filesystem::path inputDirectoryPath = GetTestFileFileAbsolutePath( "" ); // The base testData directory
	std::filesystem::path outputFilePath = std::filesystem::absolute( "CliFilterCreateGroupOut/CreateGroup_ConfirmFails_UsingFilter_validSimpleExample1_WithRelativeFilterFilePathAndWrongBasePathSet.txt" );
	std::filesystem::path filterIniFilePath = "ExampleIniFiles/validSimpleExample1.ini";

	// Ensure any previous test output files are removed
	RemoveFiles( { outputFilePath } );

	arguments.push_back( "create-group" );
	arguments.push_back( inputDirectoryPath.lexically_normal().string() );
	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "3" );
	arguments.push_back( "--filter-file" );
	arguments.push_back( filterIniFilePath.lexically_normal().string() );
	arguments.push_back( "--filter-file-basepath" );
	arguments.push_back( "Some/Incorrect/Base/Path" );
	arguments.push_back( "--output-file" );
	arguments.push_back( outputFilePath.lexically_normal().string() );
	arguments.push_back( "--document-version" );
	arguments.push_back( "0.0.0" );  // This is the "old style" csv (txt) document version

	int res = RunCli( arguments, &output, &errorOutput );
	std::cout << "--- RunCli() output: ---" << std::endl;
	std::cout << output << std::endl;
	std::cout << "------------------------" << std::endl;

	// Should fail, expecting non-zero exit code
	ASSERT_EQ( res, 1 ) << "CLI operation should fail for a reading relative filter .ini files with wrong --filter-base-path parameter - with resultCode=1";
	// Check for expected error message
	EXPECT_TRUE( errorOutput.find( "[ERROR: Failed to initialize ResourceFilter from .ini file]" ) != std::string::npos )
		<< "Expected generic (top-level) error message about failure to initialize ResourceFilter from .ini file. Actual error: " << errorOutput;
}

TEST_F( ResourcesCliTest, CreateGroup_ConfirmFails_UsingFilter_validSimpleExample1_WithAbsoluteFilterFilePathAndWrongBasePathSet )
{
	// Setup test parameters
	std::string output;
	std::string errorOutput;
	std::vector<std::string> arguments;
	std::filesystem::path inputDirectoryPath = GetTestFileFileAbsolutePath( "" ); // The base testData directory
	std::filesystem::path outputFilePath = std::filesystem::absolute( "CliFilterCreateGroupOut/CreateGroup_ConfirmFails_UsingFilter_validSimpleExample1_WithAbsoluteFilterFilePathAndWrongBasePathSet.txt" );
	std::filesystem::path filterIniFilePath = GetTestFileFileAbsolutePath( "ExampleIniFiles/validSimpleExample1.ini" );

	// Ensure any previous test output files are removed
	RemoveFiles( { outputFilePath } );

	arguments.push_back( "create-group" );
	arguments.push_back( inputDirectoryPath.lexically_normal().string() );
	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "3" );
	arguments.push_back( "--filter-file" );
	arguments.push_back( filterIniFilePath.lexically_normal().string() );
	arguments.push_back( "--filter-file-basepath" );
	arguments.push_back( "Some/Incorrect/Base/Path" );
	arguments.push_back( "--output-file" );
	arguments.push_back( outputFilePath.lexically_normal().string() );
	arguments.push_back( "--document-version" );
	arguments.push_back( "0.0.0" );  // This is the "old style" csv (txt) document version

	int res = RunCli( arguments, &output, &errorOutput );
	std::cout << "--- RunCli() output: ---" << std::endl;
	std::cout << output << std::endl;
	std::cout << "------------------------" << std::endl;

	ASSERT_EQ( res, 0 ) << "CLI operation failed, errorOutput: " << errorOutput;

	// Because of the absolute path to the .ini file, the operation will be successful even though the
	// basepath parameter is wrong (the .ini file can still be found and read successfully).
	// However, the rules applied from the filter .ini file will resolve to absolute paths that do not exist.
	// Therefore, no files will match the filter criteria and the output file will be empty.
	EXPECT_TRUE( std::filesystem::exists( outputFilePath ) ) << " Empty output file [" << outputFilePath.generic_string() << "] was not created, when it should have been.";
	EXPECT_TRUE( std::filesystem::is_empty( outputFilePath ) ) << " Output file should be empty due to filter .ini file rules not matching any files because of wrong basepath.";
}

TEST_F( ResourcesCliTest, CreateGroup_ConfirmFails_UsingFilter_validSimpleExample1_WithRelativeFilterFilePathAndNoBasePath )
{
	// Setup test parameters
	std::string output;
	std::string errorOutput;
	std::vector<std::string> arguments;
	std::filesystem::path inputDirectoryPath = GetTestFileFileAbsolutePath( "" ); // The base testData directory
	std::filesystem::path outputFilePath = std::filesystem::absolute( "CliFilterCreateGroupOut/CreateGroup_ConfirmFails_UsingFilter_validSimpleExample1_WithRelativeFilterFilePathAndNoBasePath.txt" );
	std::filesystem::path filterIniFilePath = "ExampleIniFiles/validSimpleExample1.ini";

	// Ensure any previous test output files are removed
	RemoveFiles( { outputFilePath } );

	arguments.push_back( "create-group" );
	arguments.push_back( inputDirectoryPath.lexically_normal().string() );
	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "3" );
	arguments.push_back( "--filter-file" );
	arguments.push_back( filterIniFilePath.lexically_normal().string() );
	arguments.push_back( "--output-file" );
	arguments.push_back( outputFilePath.lexically_normal().string() );
	arguments.push_back( "--document-version" );
	arguments.push_back( "0.0.0" );  // This is the "old style" csv (txt) document version

	int res = RunCli( arguments, &output, &errorOutput );
	std::cout << "--- RunCli() output: ---" << std::endl;
	std::cout << output << std::endl;
	std::cout << "------------------------" << std::endl;

	// Should fail, expecting non-zero exit code
	ASSERT_EQ( res, 1 ) << "CLI operation should fail for a reading relative filter .ini files with wrong --filter-base-path parameter - with resultCode=1";
	// Check for expected error message
	EXPECT_TRUE( errorOutput.find( "[ERROR: Failed to initialize ResourceFilter from .ini file]" ) != std::string::npos )
		<< "Expected generic (top-level) error message about failure to initialize ResourceFilter from .ini file. Actual error: " << errorOutput;
}

TEST_F( ResourcesCliTest, CreateGroup_ConfirmFails_UsingFilter_validSimpleExample1_WithAbsoluteFilterFilePathAndNoBasePath )
{
	// Setup test parameters
	std::string output;
	std::string errorOutput;
	std::vector<std::string> arguments;
	std::filesystem::path inputDirectoryPath = GetTestFileFileAbsolutePath( "" ); // The base testData directory
	std::filesystem::path outputFilePath = std::filesystem::absolute( "CliFilterCreateGroupOut/CreateGroup_ConfirmFails_UsingFilter_validSimpleExample1_WithAbsoluteFilterFilePathAndNoBasePath.txt" );
	std::filesystem::path filterIniFilePath = GetTestFileFileAbsolutePath( "ExampleIniFiles/validSimpleExample1.ini" );

	// Ensure any previous test output files are removed
	RemoveFiles( { outputFilePath } );

	arguments.push_back( "create-group" );
	arguments.push_back( inputDirectoryPath.lexically_normal().string() );
	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "3" );
	arguments.push_back( "--filter-file" );
	arguments.push_back( filterIniFilePath.lexically_normal().string() );
	arguments.push_back( "--output-file" );
	arguments.push_back( outputFilePath.lexically_normal().string() );
	arguments.push_back( "--document-version" );
	arguments.push_back( "0.0.0" );  // This is the "old style" csv (txt) document version

	int res = RunCli( arguments, &output, &errorOutput );
	std::cout << "--- RunCli() output: ---" << std::endl;
	std::cout << output << std::endl;
	std::cout << "------------------------" << std::endl;

	ASSERT_EQ( res, 0 ) << "CLI operation failed, errorOutput: " << errorOutput;

	// Because of the absolute path to the .ini file, the operation will be successful even though the
	// basepath parameter is wrong (the .ini file can still be found and read successfully).
	// However, the rules applied from the filter .ini file will resolve to absolute paths that do not exist.
	// Therefore, no files will match the filter criteria and the output file will be empty.
	EXPECT_TRUE( std::filesystem::exists( outputFilePath ) ) << " Empty output file [" << outputFilePath.generic_string() << "] was not created, when it should have been.";
	EXPECT_TRUE( std::filesystem::is_empty( outputFilePath ) ) << " Output file should be empty due to filter .ini file rules not matching any files because of wrong basepath.";
}

TEST_F( ResourcesCliTest, CreateGroup_UsingFilter_validComplexExample1_YamlOutput )
{
	// Setup test parameters
	std::string output;
	std::string errorOutput;
	std::vector<std::string> arguments;
	std::filesystem::path inputDirectoryPath = GetTestFileFileAbsolutePath( "" ); // The base testData directory
	std::filesystem::path outputFilePath = std::filesystem::absolute( "CliFilterCreateGroupOut/CreateGroup_UsingFilter_validComplexExample1.yaml" );
	std::filesystem::path filterIniFilePath = "ExampleIniFiles/validComplexExample1.ini";

	// Ensure any previous test output files are removed
	RemoveFiles( { outputFilePath } );

	arguments.push_back( "create-group" );
	arguments.push_back( inputDirectoryPath.lexically_normal().string() );
	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "3" );
	arguments.push_back( "--filter-file" );
	arguments.push_back( filterIniFilePath.lexically_normal().string() );
	arguments.push_back( "--filter-file-basepath" );
	arguments.push_back( TEST_DATA_BASE_PATH );
	arguments.push_back( "--output-file" );
	arguments.push_back( outputFilePath.lexically_normal().string() );
	arguments.push_back( "--document-version" );
	arguments.push_back( "0.1.0" );  // This is default YAML document version (setting it explicitly for clarity in test)

	int res = RunCli( arguments, &output, &errorOutput );
	std::cout << "--- RunCli() output: ---" << std::endl;
	std::cout << output << std::endl;
	std::cout << "------------------------" << std::endl;

	ASSERT_EQ( res, 0 ) << "CLI operation failed, errorOutput: " << errorOutput;

	// Check expected outcome
#if _WIN64
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "ExpectedTestOutputFiles/CreateGroup_UsingFilter_validComplexExample1_Windows.yaml" );
#elif __APPLE__
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "ExpectedTestOutputFiles/CreateGroup_UsingFilter_validComplexExample1_macOS.yaml" );
#else
#error Unsupported platform
#endif
	EXPECT_TRUE( FilesMatch( goldFile, outputFilePath ) ) << " Output file does not match expected gold file.";
}

TEST_F( ResourcesCliTest, CreateGroup_UsingFilter_validComplexExample1_CsvTxtOutput )
{
	// Setup test parameters
	std::string output;
	std::string errorOutput;
	std::vector<std::string> arguments;
	std::filesystem::path inputDirectoryPath = GetTestFileFileAbsolutePath( "" ); // The base testData directory
	std::filesystem::path outputFilePath = std::filesystem::absolute( "CliFilterCreateGroupOut/CreateGroup_UsingFilter_validComplexExample1.txt" );
	std::filesystem::path filterIniFilePath = "ExampleIniFiles/validComplexExample1.ini";

	// Ensure any previous test output files are removed
	RemoveFiles( { outputFilePath } );

	arguments.push_back( "create-group" );
	arguments.push_back( inputDirectoryPath.lexically_normal().string() );
	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "3" );
	arguments.push_back( "--filter-file" );
	arguments.push_back( filterIniFilePath.lexically_normal().string() );
	arguments.push_back( "--filter-file-basepath" );
	arguments.push_back( TEST_DATA_BASE_PATH );
	arguments.push_back( "--output-file" );
	arguments.push_back( outputFilePath.lexically_normal().string() );
	arguments.push_back( "--document-version" );
	arguments.push_back( "0.0.0" );  // This is the "old style" csv (txt) document version

	int res = RunCli( arguments, &output, &errorOutput );
	std::cout << "--- RunCli() output: ---" << std::endl;
	std::cout << output << std::endl;
	std::cout << "------------------------" << std::endl;

	ASSERT_EQ( res, 0 ) << "CLI operation failed, errorOutput: " << errorOutput;

	// Check expected outcome
#if _WIN64
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "ExpectedTestOutputFiles/CreateGroup_UsingFilter_validComplexExample1_Windows.txt" );
#elif __APPLE__
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "ExpectedTestOutputFiles/CreateGroup_UsingFilter_validComplexExample1_macOS.txt" );
#else
#error Unsupported platform
#endif
	EXPECT_TRUE( FilesMatch( goldFile, outputFilePath ) ) << " Output file does not match expected gold file.";
}

TEST_F( ResourcesCliTest, CreateGroup_UsingFilter_validSimpleAndComplexExample1_YamlOutput )
{
	// Setup test parameters
	std::string output;
	std::string errorOutput;
	std::vector<std::string> arguments;
	std::filesystem::path inputDirectoryPath = GetTestFileFileAbsolutePath( "" ); // The base testData directory
	std::filesystem::path outputFilePath = std::filesystem::absolute( "CliFilterCreateGroupOut/CreateGroup_UsingFilter_validSimpleAndComplexExample1.yaml" );
	std::vector<std::filesystem::path> filterIniFilePaths = {
		"ExampleIniFiles/validSimpleExample1.ini",
		"ExampleIniFiles/validComplexExample1.ini"
	};

	// Ensure any previous test output files are removed
	RemoveFiles( { outputFilePath } );

	arguments.push_back( "create-group" );
	arguments.push_back( inputDirectoryPath.lexically_normal().string() );
	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "3" );
	for( auto filterFilePath : filterIniFilePaths )
	{
		arguments.push_back( "--filter-file" );
		arguments.push_back( filterFilePath.lexically_normal().string() );
	}
	arguments.push_back( "--filter-file-basepath" );
	arguments.push_back( TEST_DATA_BASE_PATH );
	arguments.push_back( "--output-file" );
	arguments.push_back( outputFilePath.lexically_normal().string() );
	arguments.push_back( "--document-version" );
	arguments.push_back( "0.1.0" );  // This is default YAML document version (setting it explicitly for clarity in test)

	int res = RunCli( arguments, &output, &errorOutput );
	std::cout << "--- RunCli() output: ---" << std::endl;
	std::cout << output << std::endl;
	std::cout << "------------------------" << std::endl;

	ASSERT_EQ( res, 0 ) << "CLI operation failed, errorOutput: " << errorOutput;

	// Check expected outcome
#if _WIN64
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "ExpectedTestOutputFiles/CreateGroup_UsingFilter_validSimpleAndComplexExample1_Windows.yaml" );
#elif __APPLE__
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "ExpectedTestOutputFiles/CreateGroup_UsingFilter_validSimpleAndComplexExample1_macOS.yaml" );
#else
#error Unsupported platform
#endif
	EXPECT_TRUE( FilesMatch( goldFile, outputFilePath ) ) << " Output file does not match expected gold file.";
}

TEST_F( ResourcesCliTest, CreateGroup_UsingFilter_validSimpleAndComplexExample1_CsvTxtOutput )
{
	// Setup test parameters
	std::string output;
	std::string errorOutput;
	std::vector<std::string> arguments;
	std::filesystem::path inputDirectoryPath = GetTestFileFileAbsolutePath( "" ); // The base testData directory
	std::filesystem::path outputFilePath = std::filesystem::absolute( "CliFilterCreateGroupOut/CreateGroup_UsingFilter_validSimpleAndComplexExample1.txt" );
	std::vector<std::filesystem::path> filterIniFilePaths = {
		"ExampleIniFiles/validSimpleExample1.ini",
		"ExampleIniFiles/validComplexExample1.ini"
	};

	// Ensure any previous test output files are removed
	RemoveFiles( { outputFilePath } );

	arguments.push_back( "create-group" );
	arguments.push_back( inputDirectoryPath.lexically_normal().string() );
	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "3" );
	for( auto filterFilePath : filterIniFilePaths )
	{
		arguments.push_back( "--filter-file" );
		arguments.push_back( filterFilePath.lexically_normal().string() );
	}
	arguments.push_back( "--filter-file-basepath" );
	arguments.push_back( TEST_DATA_BASE_PATH );
	arguments.push_back( "--output-file" );
	arguments.push_back( outputFilePath.lexically_normal().string() );
	arguments.push_back( "--document-version" );
	arguments.push_back( "0.0.0" );  // This is the "old style" csv (txt) document version

	int res = RunCli( arguments, &output, &errorOutput );
	std::cout << "--- RunCli() output: ---" << std::endl;
	std::cout << output << std::endl;
	std::cout << "------------------------" << std::endl;

	ASSERT_EQ( res, 0 ) << "CLI operation failed, errorOutput: " << errorOutput;

	// Check expected outcome
#if _WIN64
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "ExpectedTestOutputFiles/CreateGroup_UsingFilter_validSimpleAndComplexExample1_Windows.txt" );
#elif __APPLE__
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "ExpectedTestOutputFiles/CreateGroup_UsingFilter_validSimpleAndComplexExample1_macOS.txt" );
#else
#error Unsupported platform
#endif
	EXPECT_TRUE( FilesMatch( goldFile, outputFilePath ) ) << " Output file does not match expected gold file.";
}

TEST_F( ResourcesCliTest, CreateGroup_ConfirmFailureParsingWronglyFormattedIniFile_UsingFilter_invalidMissingNamedSection_ini )
{
	// Test parameters:
	std::string output;
	std::string errorOutput;
	std::vector<std::string> arguments;
	std::filesystem::path inputDirectoryPath = GetTestFileFileAbsolutePath( "" ); // The base testData directory
	std::filesystem::path outputFilePath = std::filesystem::absolute( "CliFilterCreateGroupOut/CreateGroup_UsingFilter_invalidMissingNamedSection.yaml" );
	std::filesystem::path filterIniFilePath = "ExampleIniFiles/invalidMissingNamedSection.ini";

	// Ensure any previous test output files are removed
	RemoveFiles( { outputFilePath } );

	arguments.push_back( "create-group" );
	arguments.push_back( inputDirectoryPath.lexically_normal().string() );
	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "3" );
	arguments.push_back( "--filter-file" );
	arguments.push_back( filterIniFilePath.lexically_normal().string() );
	arguments.push_back( "--filter-file-basepath" );
	arguments.push_back( TEST_DATA_BASE_PATH );
	arguments.push_back( "--output-file" );
	arguments.push_back( outputFilePath.lexically_normal().string() );

	int res = RunCli( arguments, &output, &errorOutput );
	std::cout << "--- RunCli() output: ---" << std::endl;
	std::cout << output << std::endl;
	std::cout << "errorOutput: " << errorOutput << std::endl;
	std::cout << "------------------------" << std::endl;

	// Should fail, expecting non-zero exit code
	ASSERT_EQ( res, 1 ) << "CLI operation should fail for a filter .ini file with missing named section - with resultCode=1";
	// Check for expected error message
	EXPECT_TRUE( errorOutput.find( "[ERROR: Failed to initialize ResourceFilter from .ini file]" ) != std::string::npos )
		<< "Expected generic (top-level) error message about failure to initialize ResourceFilter from .ini file. Actual error: " << errorOutput;
}

TEST_F( ResourcesCliTest, CreateGroup_ConfirmFailureUsingNoExistentFilterFile_iniFileDoesNotExist )
{
	// Test parameters:
	std::string output;
	std::string errorOutput;
	std::vector<std::string> arguments;
	std::filesystem::path inputDirectoryPath = GetTestFileFileAbsolutePath( "" ); // The base testData directory
	std::filesystem::path outputFilePath = std::filesystem::absolute( "CliFilterCreateGroupOut/CreateGroup_UsingFilter_iniFileDoesNotExist.yaml" );
	std::filesystem::path filterIniFilePath = "ExampleIniFiles/iniFileNotFound.ini";

	// Ensure any previous test output files are removed
	RemoveFiles( { outputFilePath } );

	arguments.push_back( "create-group" );
	arguments.push_back( inputDirectoryPath.lexically_normal().string() );
	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "3" );
	arguments.push_back( "--filter-file" );
	arguments.push_back( filterIniFilePath.lexically_normal().string() );
	arguments.push_back( "--filter-file-basepath" );
	arguments.push_back( TEST_DATA_BASE_PATH );
	arguments.push_back( "--output-file" );
	arguments.push_back( outputFilePath.lexically_normal().string() );

	int res = RunCli( arguments, &output, &errorOutput );
	std::cout << "--- RunCli() output: ---" << std::endl;
	std::cout << output << std::endl;
	std::cout << "errorOutput: " << errorOutput << std::endl;
	std::cout << "------------------------" << std::endl;

	// Should fail, expecting non-zero exit code
	ASSERT_EQ( res, 1 ) << "CLI operation should fail for a non-existent filter .ini file - with resultCode=1";
	// Check for expected error message
	EXPECT_TRUE( errorOutput.find( "[ERROR: Failed to initialize ResourceFilter from .ini file]" ) != std::string::npos )
		<< "Expected generic (top-level) error message about failure to initialize ResourceFilter from .ini file. Actual error: " << errorOutput;
}

//---------------------------------------

TEST_F( ResourcesCliTest, CreateBundle )
{
	std::string errorOutput;
	std::vector<std::string> arguments;

	arguments.push_back( "create-bundle" );

	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "-1" );

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

	arguments.push_back( "--chunk-size" );
	arguments.push_back( "1000" );


	int res = RunCli( arguments, nullptr, &errorOutput );

	EXPECT_EQ( res, 0 ) << "CLI operation failed, errorOutput: " << errorOutput;

	// Check expected outcome
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "CreateBundle/BundleResourceGroup.yaml" );
	EXPECT_TRUE( FilesMatch( goldFile, "BundleOut/BundleResourceGroup.yaml" ) );

	std::filesystem::path goldDirectory = GetTestFileFileAbsolutePath( "CreateBundle/CreateBundleOut" );
	EXPECT_TRUE( DirectoryIsSubset( goldDirectory, "CreateBundleOut" ) );
}

TEST_F( ResourcesCliTest, RemoveResourcesWithUnknownResourceIgnoreOnResourceNotFound )
{
	std::string errorOutput;
	std::vector<std::string> arguments;

	arguments.push_back( "remove-resources" );

	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "-1" );

	std::string resourceGroupPath = GetTestFileFileAbsolutePath( "RemoveResource/BaseResourceGroup.yaml" ).string();

	arguments.push_back( resourceGroupPath );

	std::string resourcesToRemoveFile = GetTestFileFileAbsolutePath( "RemoveResource/ResourcesToRemoveListWithUnknownResource.txt" ).string();

	arguments.push_back( resourcesToRemoveFile );

	arguments.push_back( "--output-resource-group-path" );

	std::filesystem::path resourceGroupAfterRemovePath = "RemoveResource/ResourceGroup.yaml";

	arguments.push_back( resourceGroupAfterRemovePath.string() );

	arguments.push_back( "--ignore-missing-resources" );

	int res = RunCli( arguments, nullptr, &errorOutput );

	EXPECT_EQ( res, 0 ) << "CLI operation failed, errorOutput: " << errorOutput;
}

TEST_F( ResourcesCliTest, RemoveResourcesWithUnknownResourceWithInvalidPathToResourcesFile )
{
	std::vector<std::string> arguments;

	arguments.push_back( "remove-resources" );

	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "-1" );

	std::string resourceGroupPath = GetTestFileFileAbsolutePath( "RemoveResource/BaseResourceGroup.yaml" ).string();

	arguments.push_back( resourceGroupPath );

	std::string resourcesToRemoveFile = GetTestFileFileAbsolutePath( "INVALID_PATH" ).string();

	arguments.push_back( resourcesToRemoveFile );

	arguments.push_back( "--output-resource-group-path" );

	std::filesystem::path resourceGroupAfterRemovePath = "RemoveResource/ResourceGroup.yaml";

	arguments.push_back( resourceGroupAfterRemovePath.string() );

	int res = RunCli( arguments );

	EXPECT_EQ( res, 1 );
}

TEST_F( ResourcesCliTest, RemoveResourcesWithUnknownResource )
{
	std::vector<std::string> arguments;

	arguments.push_back( "remove-resources" );

	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "-1" );

	std::string resourceGroupPath = GetTestFileFileAbsolutePath( "RemoveResource/BaseResourceGroup.yaml" ).string();

	arguments.push_back( resourceGroupPath );

	std::string resourcesToRemoveFile = GetTestFileFileAbsolutePath( "RemoveResource/ResourcesToRemoveListWithUnknownResource.txt" ).string();

	arguments.push_back( resourcesToRemoveFile );

	arguments.push_back( "--output-resource-group-path" );

	std::filesystem::path resourceGroupAfterRemovePath = "RemoveResource/ResourceGroup.yaml";

	arguments.push_back( resourceGroupAfterRemovePath.string() );

	int res = RunCli( arguments );

	EXPECT_EQ( res, 1 );
}

TEST_F( ResourcesCliTest, RemoveResources )
{
	std::string errorOutput;
	std::vector<std::string> arguments;

	arguments.push_back( "remove-resources" );

	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "-1" );

	std::string resourceGroupPath = GetTestFileFileAbsolutePath( "RemoveResource/BaseResourceGroup.yaml" ).string();

	arguments.push_back( resourceGroupPath );

	std::string resourcesToRemoveFile = GetTestFileFileAbsolutePath( "RemoveResource/ResourcesToRemoveList.txt" ).string();

	arguments.push_back( resourcesToRemoveFile );

	arguments.push_back( "--output-resource-group-path" );

	std::filesystem::path resourceGroupAfterRemovePath = "RemoveResource/ResourceGroup.yaml";

	arguments.push_back( resourceGroupAfterRemovePath.string() );

	int res = RunCli( arguments, nullptr, &errorOutput );

	EXPECT_EQ( res, 0 ) << "CLI operation failed, errorOutput: " << errorOutput;

	// Check output matches expected
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "RemoveResource/ResourceGroupAfterRemove.yaml" );

	EXPECT_TRUE( FilesMatch( goldFile, resourceGroupAfterRemovePath ) );
}

TEST_F( ResourcesCliTest, DiffResourceGroupsWithTwoAdditions )
{
	std::string errorOutput;
	std::vector<std::string> arguments;

	arguments.push_back( "diff-group" );

	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "-1" );

	std::string baseResourceGroupPath = GetTestFileFileAbsolutePath( "DiffGroups/resFileIndex.txt" ).string();

	arguments.push_back( baseResourceGroupPath );

	std::string diffResourceGroupPath = GetTestFileFileAbsolutePath( "DiffGroups/resFileIndexWithAdditions.txt" ).string();

	arguments.push_back( diffResourceGroupPath );

	arguments.push_back( "--diff-output-path" );

	std::filesystem::path outputPath = "DiffWithTwoAdditions.txt";

	arguments.push_back( outputPath.string() );

	int res = RunCli( arguments, nullptr, &errorOutput );

	EXPECT_EQ( res, 0 ) << "CLI operation failed, errorOutput: " << errorOutput;

	// Check output matches expected
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "DiffGroups/ExpectedDiffWithAdditions.txt" );

	EXPECT_TRUE( FileExists( goldFile ) );

	EXPECT_TRUE( FileExists( outputPath ) );

	EXPECT_TRUE( FilesMatch( goldFile, outputPath ) );
}

TEST_F( ResourcesCliTest, DiffResourceGroupsWithTwoChanges )
{
	std::string errorOutput;
	std::vector<std::string> arguments;

	arguments.push_back( "diff-group" );

	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "-1" );

	std::string baseResourceGroupPath = GetTestFileFileAbsolutePath( "DiffGroups/resFileIndex.txt" ).string();

	arguments.push_back( baseResourceGroupPath );

	std::string diffResourceGroupPath = GetTestFileFileAbsolutePath( "DiffGroups/resFileIndexWithChanges.txt" ).string();

	arguments.push_back( diffResourceGroupPath );

	arguments.push_back( "--diff-output-path" );

	std::filesystem::path outputPath = "DiffWithTwoChanges.txt";

	arguments.push_back( outputPath.string() );

	int res = RunCli( arguments, nullptr, &errorOutput );

	EXPECT_EQ( res, 0 ) << "CLI operation failed, errorOutput: " << errorOutput;

	// Check output matches expected
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "DiffGroups/ExpectedDiffWithChanges.txt" );

	EXPECT_TRUE( FileExists( goldFile ) );

	EXPECT_TRUE( FileExists( outputPath ) );

	EXPECT_TRUE( FilesMatch( goldFile, outputPath ) );
}

TEST_F( ResourcesCliTest, DiffResourceGroupsWithTwoSubtractions )
{
	std::string errorOutput;
	std::vector<std::string> arguments;

	arguments.push_back( "diff-group" );

	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "-1" );

	std::string baseResourceGroupPath = GetTestFileFileAbsolutePath( "DiffGroups/resFileIndex.txt" ).string();

	arguments.push_back( baseResourceGroupPath );

	std::string diffResourceGroupPath = GetTestFileFileAbsolutePath( "DiffGroups/resFileIndexWithSubtractions.txt" ).string();

	arguments.push_back( diffResourceGroupPath );

	arguments.push_back( "--diff-output-path" );

	std::filesystem::path outputPath = "DiffWithTwoSubtractions.txt";

	arguments.push_back( outputPath.string() );

	int res = RunCli( arguments, nullptr, &errorOutput );

	EXPECT_EQ( res, 0 ) << "CLI operation failed, errorOutput: " << errorOutput;

	// Check output matches expected
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "DiffGroups/ExpectedDiffWithSubtractions.txt" );

	EXPECT_TRUE( FileExists( goldFile ) );

	EXPECT_TRUE( FileExists( outputPath ) );

	EXPECT_TRUE( FilesMatch( goldFile, outputPath ) );
}

TEST_F( ResourcesCliTest, MergeGroup )
{
	std::string errorOutput;

	std::vector<std::string> arguments;

	arguments.push_back( "merge-group" );

	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "-1" );

	std::string baseResourceGroupPath = GetTestFileFileAbsolutePath( "MergeGroups/YamlAdditive/BaseResourceGroup.yaml" ).string();

	arguments.push_back( baseResourceGroupPath );

	std::string mergeResourceGroupPath = GetTestFileFileAbsolutePath( "MergeGroups/YamlAdditive/MergeResourceGroup.yaml" ).string();

	arguments.push_back( mergeResourceGroupPath );

	arguments.push_back( "--merge-output-resource-group-path" );

	std::filesystem::path mergedOutputPath = "Merge/mergedResourceGroup.yaml";

	arguments.push_back( mergedOutputPath.string() );

	int res = RunCli( arguments, nullptr, &errorOutput );

	EXPECT_EQ( res, 0 ) << "CLI operation failed, errorOutput: " << errorOutput;

	// Check output matches expected
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "MergeGroups/YamlAdditive/ExpectedMergedResourceGroup.yaml" );

	EXPECT_TRUE( FilesMatch( goldFile, mergedOutputPath ) );
}

TEST_F( ResourcesCliTest, CreatePatch )
{
	std::string errorOutput;
	std::vector<std::string> arguments;

	arguments.push_back( "create-patch" );

	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "-1" );

	std::string previousResourceGroupPath = GetTestFileFileAbsolutePath( "Patch/resfileindexShort_build_previous.txt" ).string();

	arguments.push_back( previousResourceGroupPath );

	std::string nextResourceGroupPath = GetTestFileFileAbsolutePath( "Patch/resfileindexShort_build_next.txt" ).string();

	arguments.push_back( nextResourceGroupPath );

	arguments.push_back( "--resource-source-type-previous" );
	arguments.push_back( "LOCAL_RELATIVE" );

	std::string nextResourcesLocation = GetTestFileFileAbsolutePath( "Patch/NextBuildResources" ).string();

	arguments.push_back( "--resource-source-base-path-next" );
	arguments.push_back( nextResourcesLocation );

	std::string previousResourcesLocation = GetTestFileFileAbsolutePath( "Patch/PreviousBuildResources" ).string();

	arguments.push_back( "--resource-source-base-path-previous" );
	arguments.push_back( previousResourcesLocation );

	arguments.push_back( "--patch-resourcegroup-destination-path" );
	arguments.push_back( "PatchOut" );

	arguments.push_back( "--patch-destination-base-path" );
	arguments.push_back( "Patchout/Patches" );

	arguments.push_back( "--patch-destination-type" );
	arguments.push_back( "LOCAL_CDN" );

	arguments.push_back( "--chunk-size" );
	arguments.push_back( "50000000" );

	int res = RunCli( arguments, nullptr, &errorOutput );

	EXPECT_EQ( res, 0 ) << "CLI operation failed, errorOutput: " << errorOutput;

	// Check expected outcome
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "Patch/PatchResourceGroup.yaml" );
	EXPECT_TRUE( FilesMatch( goldFile, "PatchOut/PatchResourceGroup.yaml" ) );

	std::filesystem::path goldDirectory = GetTestFileFileAbsolutePath( "Patch/LocalCDNPatches" );
	EXPECT_TRUE( DirectoryIsSubset( goldDirectory, "PatchOut/Patches" ) );
}

TEST_F( ResourcesCliTest, CreateGroup )
{
	std::string errorOutput;
	std::vector<std::string> arguments;

	arguments.push_back( "create-group" );

	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "-1" );

	std::string directoryIn = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceFiles" ).string();

	arguments.push_back( directoryIn );

	arguments.push_back( "--output-file" );

	std::string outputFilename = "ResourcesCliTestResourceGroup.yaml";

	arguments.push_back( outputFilename );

	int res = RunCli( arguments, nullptr, &errorOutput );

	EXPECT_EQ( res, 0 ) << "CLI operation failed, errorOutput: " << errorOutput;

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

TEST_F( ResourcesCliTest, ApplyPatch )
{
	std::string errorOutput;
	std::vector<std::string> arguments;

	arguments.push_back( "apply-patch" );

	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "-1" );

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

	int res = RunCli( arguments, nullptr, &errorOutput );

	EXPECT_EQ( res, 0 ) << "CLI operation failed, errorOutput: " << errorOutput;

	// Check expected outcome
	std::filesystem::path goldDirectory = GetTestFileFileAbsolutePath( "Patch/NextBuildResources" );
	EXPECT_TRUE( DirectoryIsSubset( outputBasePath, goldDirectory ) );
}

TEST_F( ResourcesCliTest, UnpackBundle )
{
	std::string errorOutput;
	std::vector<std::string> arguments;

	arguments.push_back( "unpack-bundle" );

	arguments.push_back( "--verbosity-level" );
	arguments.push_back( "-1" );

	std::string directoryIn = GetTestFileFileAbsolutePath( "Bundle/BundleResourceGroup.yaml" ).string();

	arguments.push_back( directoryIn );

	arguments.push_back( "--chunk-source-base-path" );

	std::string chunkSourceBasePath = GetTestFileFileAbsolutePath( "Bundle/LocalRemoteChunks/" ).string();

	arguments.push_back( chunkSourceBasePath );

	arguments.push_back( "--resource-destination-type" );

	arguments.push_back( "LOCAL_RELATIVE" );

	int res = RunCli( arguments, nullptr, &errorOutput );

	EXPECT_EQ( res, 0 ) << "CLI operation failed, errorOutput: " << errorOutput;

	// Check expected outcome
	EXPECT_TRUE( DirectoryIsSubset( GetTestFileFileAbsolutePath( "Bundle/Res" ), "UnpackBundleOut" ) );

	EXPECT_TRUE( std::filesystem::exists( "UnpackBundleOut/ResourceGroup.yaml" ) );
}

#endif