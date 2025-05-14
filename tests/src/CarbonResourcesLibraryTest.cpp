#include <ResourceGroup.h>
#include <BundleResourceGroup.h>
#include <PatchResourceGroup.h>

#include "CarbonResourcesTestFixture.h"

#include <gtest/gtest.h>

#include <FileDataStreamOut.h>

// TODO I think it would be good if the output files of the tests were put in folders which match the test name
// The DLL handles the case where the dll is a newer version that what was used to compile against, find a way to test this
// Tests need to cover all bad parameter entry input
// eg. not providing the correct parameters, or passing bad paths etc

struct CarbonResourcesLibraryTest : public CarbonResourcesTestFixture{};

// Import ResourceGroup V0.0.0
// Export should output as V0.1.0
// This is the only instance that exporting a file of a lower version results in a version bump
TEST_F( CarbonResourcesLibraryTest, BinaryGroupImportExport_V_0_0_0_To_V_0_1_0 )
{

	CarbonResources::ResourceGroup binaryResourceGroup;

	CarbonResources::ResourceGroupImportFromFileParams importParams;

    importParams.filename = GetTestFileFileAbsolutePath( "Indicies/binaryFileIndex_v0_0_0.txt" );

	EXPECT_EQ(binaryResourceGroup.ImportFromFile( importParams ).type,CarbonResources::ResultType::SUCCESS);

	CarbonResources::ResourceGroupExportToFileParams exportParams;

    exportParams.filename = "resPath/BinaryResourceGroup_v0_1_0.yaml";

	EXPECT_EQ(binaryResourceGroup.ExportToFile( exportParams ).type,CarbonResources::ResultType::SUCCESS);

    std::filesystem::path goldStandardFilename = GetTestFileFileAbsolutePath( "Indicies/BinaryResourceGroup_v0_1_0.yaml" );

	EXPECT_TRUE( FilesMatch( exportParams.filename, goldStandardFilename ) );
}

// Import a BinaryResourceGroup v0.1.0 and export it again checking input == output
TEST_F( CarbonResourcesLibraryTest, BinaryGroupImportExport_V_0_1_0 )
{
	CarbonResources::ResourceGroup binaryResourceGroup;

	CarbonResources::ResourceGroupImportFromFileParams importParams;

    importParams.filename = GetTestFileFileAbsolutePath( "Indicies/BinaryResourceGroup_v0_1_0.yaml" );

	EXPECT_EQ(binaryResourceGroup.ImportFromFile( importParams ).type,CarbonResources::ResultType::SUCCESS);

	CarbonResources::ResourceGroupExportToFileParams exportParams;

    exportParams.filename = "resPath/BinaryResourceGroup_v0_1_0.yaml";

	EXPECT_EQ(binaryResourceGroup.ExportToFile( exportParams ).type,CarbonResources::ResultType::SUCCESS);

	EXPECT_TRUE( FilesMatch( importParams.filename, exportParams.filename ) );
}

// Import ResourceGroup V0.0.0 
// Export should output as V0.1.0
// This is the only instance that exporting a file of a lower version results in a version bump
TEST_F( CarbonResourcesLibraryTest, ResourceGroupImportExport_V_0_0_0_To_V_0_1_0 )
{
	CarbonResources::ResourceGroup resourceGroup;

	CarbonResources::ResourceGroupImportFromFileParams importParams;

    importParams.filename = GetTestFileFileAbsolutePath( "Indicies/resFileIndex_v0_0_0.txt" );

	EXPECT_EQ(resourceGroup.ImportFromFile( importParams ).type,CarbonResources::ResultType::SUCCESS);

	CarbonResources::ResourceGroupExportToFileParams exportParams;

    exportParams.filename = "resPath/ResourceGroup_v0_1_0.yaml";

	EXPECT_EQ(resourceGroup.ExportToFile( exportParams ).type,CarbonResources::ResultType::SUCCESS);

    std::filesystem::path goldStandardFilename = GetTestFileFileAbsolutePath( "Indicies/ResourceGroup_v0_1_0.yaml" );

	EXPECT_TRUE( FilesMatch( exportParams.filename, goldStandardFilename ) );
}

TEST_F( CarbonResourcesLibraryTest, ImportEmptyResourceGroup )
{
	CarbonResources::ResourceGroup resourceGroup;
	CarbonResources::ResourceGroupImportFromFileParams importParams;
	importParams.filename = GetTestFileFileAbsolutePath( "ResourceGroups/EmptyResourceGroup.yaml" );
	EXPECT_EQ(resourceGroup.ImportFromFile( importParams ).type,CarbonResources::ResultType::SUCCESS);
}

void CreateEmptyResourceGroupWithMissingParameter(std::filesystem::path emptyResourceGroupPath, std::filesystem::path outPath, const std::string& missingTagName)
{
	std::filesystem::path testDataFolder( TEST_DATA_BASE_PATH );
	std::ifstream fileIn( emptyResourceGroupPath );
	create_directories( outPath.parent_path() );
	std::ofstream fileOut( outPath, std::ios::out);
	ASSERT_TRUE( fileOut.is_open() );
	std::string line;
	while(std::getline( fileIn, line ))
	{
		std::string formattedTag = missingTagName + ":";
		std::string startOfLine = line.substr( 0, missingTagName.size() + 1 );
		if( formattedTag != startOfLine )
		{
			fileOut << line << std::endl;
		}
	}
}

CarbonResources::Result AttemptImportResourceGroupMissingParameter( const std::string& tag, std::filesystem::path emptyResourceGroupPath )
{
	std::filesystem::path outPath(std::filesystem::current_path() / "ResourceGroups" / ("Missing" + tag + ".yaml"));
	CreateEmptyResourceGroupWithMissingParameter( emptyResourceGroupPath, outPath, tag );
	CarbonResources::ResourceGroup resourceGroup;
	CarbonResources::ResourceGroupImportFromFileParams importParams;
	importParams.filename = outPath;
	return resourceGroup.ImportFromFile( importParams );
}

// Import a ResourceGroup with missing parameters that are expected for the version provided
// This should fail gracefully and give appropriate feedback to user
TEST_F( CarbonResourcesLibraryTest, ResourceGroupHandleImportMissingParametersForVersion )
{
	std::filesystem::path emptyResourceGroupPath = GetTestFileFileAbsolutePath( "ResourceGroups/EmptyResourceGroup.yaml" );
	EXPECT_EQ( AttemptImportResourceGroupMissingParameter( "Version", emptyResourceGroupPath ).type, CarbonResources::ResultType::MALFORMED_RESOURCE_GROUP );
	EXPECT_EQ( AttemptImportResourceGroupMissingParameter( "Type", emptyResourceGroupPath ).type, CarbonResources::ResultType::MALFORMED_RESOURCE_GROUP );
	EXPECT_EQ( AttemptImportResourceGroupMissingParameter( "NumberOfResources", emptyResourceGroupPath ).type, CarbonResources::ResultType::MALFORMED_RESOURCE_GROUP );
	EXPECT_EQ( AttemptImportResourceGroupMissingParameter( "TotalResourcesSizeCompressed", emptyResourceGroupPath ).type, CarbonResources::ResultType::MALFORMED_RESOURCE_GROUP );
	EXPECT_EQ( AttemptImportResourceGroupMissingParameter( "TotalResourcesSizeUnCompressed", emptyResourceGroupPath ).type, CarbonResources::ResultType::MALFORMED_RESOURCE_GROUP );
	EXPECT_EQ( AttemptImportResourceGroupMissingParameter( "Resources", emptyResourceGroupPath ).type, CarbonResources::ResultType::MALFORMED_RESOURCE_GROUP );
}

// Import a malformed ResourceGroup
// This should fail gracefully and give appropriate feedback to user
TEST_F( CarbonResourcesLibraryTest, ResourceGroupHandleImportIncorrectlyFormattedInput )
{
	CarbonResources::ResourceGroup resourceGroup;
	CarbonResources::ResourceGroupImportFromFileParams importParams;
	importParams.filename = GetTestFileFileAbsolutePath( "ResourcesRemote/a9/a9d1721dd5cc6d54_4d7a8d216f4c8c5c6379476c0668fe84" );
	EXPECT_EQ( resourceGroup.ImportFromFile( importParams ).type, CarbonResources::ResultType::UNSUPPORTED_FILE_FORMAT );
}

// Import a ResourceGroup with version greater than current document minor version specified in enums.h
// This should open ignoring anything extra added in the future version
// The version of the imported ResourceGroup should be set at the max supported version in enums.h
TEST_F( CarbonResourcesLibraryTest, ResourceGroupImportNewerMinorVersion )
{
	CarbonResources::ResourceGroup resourceGroup;
	CarbonResources::ResourceGroupImportFromFileParams importParams;
	importParams.filename = GetTestFileFileAbsolutePath( "ResourceGroups/HigherMinorVersion.yaml" );
	EXPECT_EQ( resourceGroup.ImportFromFile( importParams ).type, CarbonResources::ResultType::SUCCESS );
}

// Import a ResourceGroup with version greater than current document major version specified in enums.h
// This should gracefully fail to open
TEST_F( CarbonResourcesLibraryTest, ResourceGroupImportNewerMajorVersion )
{
	CarbonResources::ResourceGroup resourceGroup;
	CarbonResources::ResourceGroupImportFromFileParams importParams;
	importParams.filename = GetTestFileFileAbsolutePath( "ResourceGroups/HigherMajorVersion.yaml" );
	EXPECT_EQ( resourceGroup.ImportFromFile( importParams ).type, CarbonResources::ResultType::DOCUMENT_VERSION_UNSUPPORTED );
}

// Import a ResourceGroup that doesn't exist
// This should gracefully fail
TEST_F( CarbonResourcesLibraryTest, ResourceGroupImportNonExistantFile )
{
	CarbonResources::ResourceGroup resourceGroup;

	CarbonResources::ResourceGroupImportFromFileParams importParams;

	importParams.filename = GetTestFileFileAbsolutePath( "NonexistentFiles/ResourceGroup_which_does_not_exist.yaml" );

	EXPECT_EQ( resourceGroup.ImportFromFile( importParams ).type, CarbonResources::ResultType::FAILED_TO_OPEN_FILE );
}

// Import a ResourceGroup v0.1.0 and export it again checking input == output
TEST_F( CarbonResourcesLibraryTest, ResourceGroupImportExport_V_0_1_0 )
{

	CarbonResources::ResourceGroup resourceGroup;

	CarbonResources::ResourceGroupImportFromFileParams importParams;

    importParams.filename = GetTestFileFileAbsolutePath( "Indicies/ResourceGroup_v0_1_0.yaml" );

	EXPECT_EQ(resourceGroup.ImportFromFile( importParams ).type,CarbonResources::ResultType::SUCCESS);

	CarbonResources::ResourceGroupExportToFileParams exportParams;

    exportParams.filename = "resPath/ResourceGroup_v0_1_0.yaml";

	EXPECT_EQ(resourceGroup.ExportToFile( exportParams ).type,CarbonResources::ResultType::SUCCESS);

	EXPECT_TRUE( FilesMatch( importParams.filename, exportParams.filename ) );
}

TEST_F( CarbonResourcesLibraryTest, UnpackBundle )
{
	// Load the bundle file
	CarbonResources::BundleResourceGroup bundleResourceGroup;

	CarbonResources::ResourceGroupImportFromFileParams importParamsPrevious;

	importParamsPrevious.filename = GetTestFileFileAbsolutePath( "Bundle/BundleResourceGroup.yaml" );

	EXPECT_EQ( bundleResourceGroup.ImportFromFile( importParamsPrevious ).type, CarbonResources::ResultType::SUCCESS );


    // Unpack the bundle
	CarbonResources::BundleUnpackParams bundleUnpackParams;
	
	bundleUnpackParams.chunkSourceSettings.sourceType = CarbonResources::ResourceSourceType::LOCAL_CDN;

	bundleUnpackParams.chunkSourceSettings.basePath = GetTestFileFileAbsolutePath( "Bundle/LocalRemoteChunks/" );

	bundleUnpackParams.resourceDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_RELATIVE;

	bundleUnpackParams.resourceDestinationSettings.basePath = "UnpackBundleOut/";

	EXPECT_EQ( bundleResourceGroup.Unpack( bundleUnpackParams ).type, CarbonResources::ResultType::SUCCESS );

	// TODO test the output of the applied patches
    

}
TEST_F( CarbonResourcesLibraryTest, CreateBundle )
{
	// Import ResourceGroup
	CarbonResources::ResourceGroup resourceGroup;

	CarbonResources::ResourceGroupImportFromFileParams importParams;

	importParams.filename = GetTestFileFileAbsolutePath( "Bundle/resfileindexShort.txt" );

	EXPECT_EQ( resourceGroup.ImportFromFile( importParams ).type, CarbonResources::ResultType::SUCCESS );


    // Create a bundle from the ResourceGroup
	CarbonResources::BundleCreateParams bundleCreateParams;

    bundleCreateParams.resourceGroupRelativePath = "ResourceGroup.yaml";

	bundleCreateParams.resourceGroupBundleRelativePath = "BundleResourceGroup.yaml";

	bundleCreateParams.resourceSourceSettings.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;

    bundleCreateParams.resourceSourceSettings.basePath = GetTestFileFileAbsolutePath( "Bundle/Res/" );

    bundleCreateParams.chunkDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_CDN;

    bundleCreateParams.chunkDestinationSettings.basePath = "CreateBundleOut";

    bundleCreateParams.resourceBundleResourceGroupDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_RELATIVE;

	bundleCreateParams.resourceBundleResourceGroupDestinationSettings.basePath = "resPath";

    bundleCreateParams.chunkSize = 1000;

	EXPECT_EQ(resourceGroup.CreateBundle( bundleCreateParams ).type,CarbonResources::ResultType::SUCCESS);

	EXPECT_TRUE( FilesMatch( "resPath/BundleResourceGroup.yaml" , GetTestFileFileAbsolutePath( "CreateBundle/BundleResourceGroup.yaml" ) ) );
	EXPECT_TRUE( DirectoryIsSubset( "CreateBundleOut", GetTestFileFileAbsolutePath( "CreateBundle/CreateBundleOut" ) ) );

}

TEST_F( CarbonResourcesLibraryTest, ApplyPatch )
{
	// Load the patch file
	CarbonResources::PatchResourceGroup patchResourceGroup;

    CarbonResources::ResourceGroupImportFromFileParams importParamsPrevious;

    importParamsPrevious.filename = GetTestFileFileAbsolutePath( "Patch/PatchResourceGroup.yaml" );

	EXPECT_EQ( patchResourceGroup.ImportFromFile( importParamsPrevious ).type, CarbonResources::ResultType::SUCCESS );


    // Apply the patch
	CarbonResources::PatchApplyParams patchApplyParams;

    patchApplyParams.newBuildResourcesSourceSettings.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;

	patchApplyParams.newBuildResourcesSourceSettings.basePath = GetTestFileFileAbsolutePath( "Patch/NextBuildResources/" );

    patchApplyParams.patchBinarySourceSettings.sourceType = CarbonResources::ResourceSourceType::LOCAL_CDN;

    patchApplyParams.patchBinarySourceSettings.basePath = GetTestFileFileAbsolutePath( "Patch/LocalCDNPatches/" );

    patchApplyParams.resourcesToPatchSourceSettings.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;

    patchApplyParams.resourcesToPatchSourceSettings.basePath = GetTestFileFileAbsolutePath( "Patch/PreviousBuildResources/" );

    patchApplyParams.resourcesToPatchDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_RELATIVE;

    patchApplyParams.resourcesToPatchDestinationSettings.basePath = "ApplyPatchOut";

    patchApplyParams.temporaryFilePath = "tempFile.resource";

    EXPECT_EQ(patchResourceGroup.Apply( patchApplyParams ).type,CarbonResources::ResultType::SUCCESS);

    // Check Expected Outcome
	std::filesystem::path goldDirectory = GetTestFileFileAbsolutePath( "Patch/NextBuildResources" );
	EXPECT_TRUE( DirectoryIsSubset( patchApplyParams.resourcesToPatchDestinationSettings.basePath , goldDirectory ));

}

// TODO create a patch with identical input ResourceGroups to ensure it is handled well

TEST_F( CarbonResourcesLibraryTest, CreatePatch )
{
    // Previous ResourceGroup
	CarbonResources::ResourceGroup resourceGroupPrevious;

	CarbonResources::ResourceGroupImportFromFileParams importParamsPrevious;

    importParamsPrevious.filename = GetTestFileFileAbsolutePath( "Patch/resfileindexShort_build_previous.txt" );

	EXPECT_EQ(resourceGroupPrevious.ImportFromFile( importParamsPrevious ).type,CarbonResources::ResultType::SUCCESS);


    // Latest ResourceGroup
	CarbonResources::ResourceGroup resourceGroupLatest;

	CarbonResources::ResourceGroupImportFromFileParams importParamsLatest;

    importParamsLatest.filename = GetTestFileFileAbsolutePath( "Patch/resfileindexShort_build_next.txt" );

	EXPECT_EQ(resourceGroupLatest.ImportFromFile( importParamsLatest ).type,CarbonResources::ResultType::SUCCESS);

    // Create a patch from the subtraction index
	CarbonResources::PatchCreateParams patchCreateParams;

    patchCreateParams.resourceGroupRelativePath = "ResourceGroup.yaml";

    patchCreateParams.resourceGroupPatchRelativePath = "PatchResourceGroup.yaml";

    patchCreateParams.resourceSourceSettingsFrom.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;

    patchCreateParams.resourceSourceSettingsFrom.basePath = GetTestFileFileAbsolutePath( "Patch/PreviousBuildResources" );

    patchCreateParams.resourceSourceSettingsTo.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;

    patchCreateParams.resourceSourceSettingsTo.basePath = GetTestFileFileAbsolutePath( "Patch/NextBuildResources" );

    patchCreateParams.resourcePatchBinaryDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_CDN;

    patchCreateParams.resourcePatchBinaryDestinationSettings.basePath = "SharedCache";

    patchCreateParams.resourcePatchResourceGroupDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_RELATIVE;

    patchCreateParams.resourcePatchResourceGroupDestinationSettings.basePath = "resPath";

    patchCreateParams.patchFileRelativePathPrefix = "Patches/Patch";

    patchCreateParams.previousResourceGroup = &resourceGroupPrevious;

    patchCreateParams.maxInputFileChunkSize = 50000000;
    
	EXPECT_EQ(resourceGroupLatest.CreatePatch( patchCreateParams ).type,CarbonResources::ResultType::SUCCESS);

    // Check expected outcome
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "Patch/PatchResourceGroup.yaml" );
	EXPECT_TRUE( FilesMatch( goldFile, patchCreateParams.resourcePatchResourceGroupDestinationSettings.basePath / "PatchResourceGroup.yaml" ) );

	std::filesystem::path goldDirectory = GetTestFileFileAbsolutePath( "Patch/LocalCDNPatches" );
	EXPECT_TRUE( DirectoryIsSubset( goldDirectory, patchCreateParams.resourcePatchBinaryDestinationSettings.basePath ) );
    
}

TEST_F( CarbonResourcesLibraryTest, ApplyPatchWithChunking )
{
	// Load the patch file
	CarbonResources::PatchResourceGroup patchResourceGroup;

	CarbonResources::ResourceGroupImportFromFileParams importParamsPrevious;

	importParamsPrevious.filename = GetTestFileFileAbsolutePath( "PatchWithInputChunk/PatchResourceGroup_previousBuild_latestBuild.yaml" );

	EXPECT_EQ( patchResourceGroup.ImportFromFile( importParamsPrevious ).type, CarbonResources::ResultType::SUCCESS );


	// Apply the patch
	CarbonResources::PatchApplyParams patchApplyParams;

	patchApplyParams.newBuildResourcesSourceSettings.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;

	patchApplyParams.newBuildResourcesSourceSettings.basePath = GetTestFileFileAbsolutePath( "PatchWithInputChunk/NextBuildResources/" );

	patchApplyParams.patchBinarySourceSettings.sourceType = CarbonResources::ResourceSourceType::LOCAL_CDN;

	patchApplyParams.patchBinarySourceSettings.basePath = GetTestFileFileAbsolutePath( "PatchWithInputChunk/LocalCDNPatches/" );

	patchApplyParams.resourcesToPatchSourceSettings.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;

	patchApplyParams.resourcesToPatchSourceSettings.basePath = GetTestFileFileAbsolutePath( "PatchWithInputChunk/PreviousBuildResources/" );

	patchApplyParams.resourcesToPatchDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_RELATIVE;

	patchApplyParams.resourcesToPatchDestinationSettings.basePath = "ApplyPatchWithChunkingOut";

	patchApplyParams.temporaryFilePath = "tempFile.resource";

	EXPECT_EQ( patchResourceGroup.Apply( patchApplyParams ).type, CarbonResources::ResultType::SUCCESS );

	std::filesystem::path nextIntroMovie = GetTestFileFileAbsolutePath( "PatchWithInputChunk/NextBuildResources/introMovie.txt" );
	EXPECT_TRUE( FilesMatch( nextIntroMovie, patchApplyParams.resourcesToPatchDestinationSettings.basePath / "introMovie.txt" ) );
	std::filesystem::path nextIntroMoviePrefixed = GetTestFileFileAbsolutePath( "PatchWithInputChunk/NextBuildResources/introMoviePrefixed.txt" );
	EXPECT_TRUE( FilesMatch( nextIntroMoviePrefixed, patchApplyParams.resourcesToPatchDestinationSettings.basePath / "introMoviePrefixed.txt" ) );
	std::filesystem::path nextTestResource = GetTestFileFileAbsolutePath( "PatchWithInputChunk/NextBuildResources/testresource2.txt" );
	EXPECT_TRUE( FilesMatch( nextTestResource, patchApplyParams.resourcesToPatchDestinationSettings.basePath / "testresource2.txt" ) );
}

TEST_F( CarbonResourcesLibraryTest, CreatePatchWithChunking )
{
	// Previous ResourceGroup
	CarbonResources::ResourceGroup resourceGroupPrevious;

	CarbonResources::ResourceGroupImportFromFileParams importParamsPrevious;

	importParamsPrevious.filename = GetTestFileFileAbsolutePath( "PatchWithInputChunk/resfileindexShort_build_previous.txt" );

	EXPECT_EQ( resourceGroupPrevious.ImportFromFile( importParamsPrevious ).type, CarbonResources::ResultType::SUCCESS );


	// Latest ResourceGroup
	CarbonResources::ResourceGroup resourceGroupLatest;

	CarbonResources::ResourceGroupImportFromFileParams importParamsLatest;

	importParamsLatest.filename = GetTestFileFileAbsolutePath( "PatchWithInputChunk/resfileindexShort_build_next.txt" );

	EXPECT_EQ( resourceGroupLatest.ImportFromFile( importParamsLatest ).type, CarbonResources::ResultType::SUCCESS );



	// Create a patch from the subtraction index
	CarbonResources::PatchCreateParams patchCreateParams;

	patchCreateParams.resourceGroupRelativePath = "ResourceGroup_previousBuild_latestBuild.yaml";

	patchCreateParams.resourceGroupPatchRelativePath = "PatchResourceGroup_previousBuild_latestBuild.yaml";

	patchCreateParams.resourceSourceSettingsFrom.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;

	patchCreateParams.resourceSourceSettingsFrom.basePath = GetTestFileFileAbsolutePath( "PatchWithInputChunk/PreviousBuildResources" );

	patchCreateParams.resourceSourceSettingsTo.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;

	patchCreateParams.resourceSourceSettingsTo.basePath = GetTestFileFileAbsolutePath( "PatchWithInputChunk/NextBuildResources" );

	patchCreateParams.resourcePatchBinaryDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_CDN;

	patchCreateParams.resourcePatchBinaryDestinationSettings.basePath = "SharedCache";

	patchCreateParams.resourcePatchResourceGroupDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_RELATIVE;

	patchCreateParams.resourcePatchResourceGroupDestinationSettings.basePath = "resPath";

	patchCreateParams.patchFileRelativePathPrefix = "Patches/Patch1";

	patchCreateParams.previousResourceGroup = &resourceGroupPrevious;

	patchCreateParams.maxInputFileChunkSize = 500;

	EXPECT_EQ( resourceGroupLatest.CreatePatch( patchCreateParams ).type, CarbonResources::ResultType::SUCCESS );

	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "PatchWithInputChunk/PatchResourceGroup_previousBuild_latestBuild.yaml" );
	EXPECT_TRUE( FilesMatch( goldFile, patchCreateParams.resourcePatchResourceGroupDestinationSettings.basePath / "PatchResourceGroup_previousBuild_latestBuild.yaml" ) );

	std::filesystem::path goldDirectory = GetTestFileFileAbsolutePath( "PatchWithInputChunk/LocalCDNPatches" );
	EXPECT_TRUE( DirectoryIsSubset(  goldDirectory, patchCreateParams.resourcePatchBinaryDestinationSettings.basePath ) );
}

TEST_F( CarbonResourcesLibraryTest, CreateResourceGroupFromDirectory )
{
	CarbonResources::ResourceGroup resourceGroup;

	CarbonResources::CreateResourceGroupFromDirectoryParams createResourceGroupParams;

	createResourceGroupParams.directory = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceFiles" );

	EXPECT_EQ( resourceGroup.CreateFromDirectory( createResourceGroupParams ).type, CarbonResources::ResultType::SUCCESS );

	CarbonResources::ResourceGroupExportToFileParams exportParams;

	exportParams.filename = "ResourceGroups/ResourceGroup.yaml";

	EXPECT_EQ( resourceGroup.ExportToFile( exportParams ).type, CarbonResources::ResultType::SUCCESS );

#if _WIN64
    std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceGroupWindows.yaml" );
#elif __APPLE__
    std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceGroupMacOS.yaml" );
#else
#error Unsupported platform
#endif
    EXPECT_TRUE( FilesMatch( goldFile, exportParams.filename ) );
}

TEST_F( CarbonResourcesLibraryTest, CreateResourceGroupFromDirectoryOutputPathIsInvalid )
{
	CarbonResources::ResourceGroup resourceGroup;

	CarbonResources::CreateResourceGroupFromDirectoryParams createResourceGroupParams;

	createResourceGroupParams.directory = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceFiles" );

	EXPECT_EQ( resourceGroup.CreateFromDirectory( createResourceGroupParams ).type, CarbonResources::ResultType::SUCCESS );

	CarbonResources::ResourceGroupExportToFileParams exportParams;

	exportParams.filename = "///";

	EXPECT_EQ( resourceGroup.ExportToFile( exportParams ).type, CarbonResources::ResultType::FAILED_TO_SAVE_FILE );

}

TEST_F( CarbonResourcesLibraryTest, CreateResourceGroupFailsWithInvalidInputDirectory )
{
	CarbonResources::ResourceGroup resourceGroup;

	CarbonResources::CreateResourceGroupFromDirectoryParams createResourceGroupParams;

	createResourceGroupParams.directory = "INVALID_PATH";

	EXPECT_EQ( resourceGroup.CreateFromDirectory( createResourceGroupParams ).type, CarbonResources::ResultType::INPUT_DIRECTORY_DOESNT_EXIST);
}

/*
// TODO 
// Actually this is probably going to be useful to others during this early dev stage
// Uncomment and fill in paths to the Vanguard test data
// I think these are useful, it would be good to have this test as a local only test somehow so one can test on real data easily locally
// To use, run in order
// Each one will create the files the next one consumes, so if we did want to adopt this style of test they need changing.
// Alternatively original idea was that this style would be tested using CLI
TEST_F( CarbonResourcesLibraryTest, CreateResourceGroupAFromDirectoryVanguard )
{
	CarbonResources::ResourceGroup resourceGroup;

    CarbonResources::CreateResourceGroupFromDirectoryParams createResourceGroupParams;

    createResourceGroupParams.directory = "C:/Users/gilbert/Documents/Notes/IncrementalBinaryPatching/VanguardTestData/ClientA/WindowsClient";

    EXPECT_EQ( resourceGroup.CreateFromDirectory( createResourceGroupParams ),CarbonResources::Result::SUCCESS);

    CarbonResources::ResourceGroupExportToFileParams exportParams;

    exportParams.filename = "VanguardData/ClientAResourceGroup.yaml";

    EXPECT_EQ( resourceGroup.ExportToFile( exportParams ), CarbonResources::Result::SUCCESS );

}

TEST_F( CarbonResourcesLibraryTest, CreateResourceGroupBFromDirectoryVanguard )
{
	CarbonResources::ResourceGroup resourceGroup;

	CarbonResources::CreateResourceGroupFromDirectoryParams createResourceGroupParams;

	createResourceGroupParams.directory = "C:/Users/gilbert/Documents/Notes/IncrementalBinaryPatching/VanguardTestData/ClientB/WindowsClient";

	EXPECT_EQ( resourceGroup.CreateFromDirectory( createResourceGroupParams ), CarbonResources::Result::SUCCESS );

	CarbonResources::ResourceGroupExportToFileParams exportParams;

	exportParams.filename = "VanguardData/ClientBResourceGroup.yaml";

	EXPECT_EQ( resourceGroup.ExportToFile( exportParams ), CarbonResources::Result::SUCCESS );
}




TEST_F( CarbonResourcesLibraryTest, CreateVanguardBundle )
{
	// Import ResourceGroup
	CarbonResources::ResourceGroup resourceGroup;

	CarbonResources::ResourceGroupImportFromFileParams importParams;

	importParams.filename = "C:/Users/gilbert/Documents/CCP/Builds/carbon-resources/tests/Debug/VanguardData/ClientBResourceGroup.yaml";

	EXPECT_EQ( resourceGroup.ImportFromFile( importParams ), CarbonResources::Result::SUCCESS );


	// Create a bundle from the ResourceGroup
	CarbonResources::BundleCreateParams bundleCreateParams;

	bundleCreateParams.resourceGroupRelativePath = "ResourceGroup.yaml"; // A copy is made of the resourceGroup

	bundleCreateParams.resourceGroupBundleRelativePath = "BundleResourceGroup.yaml";

	bundleCreateParams.resourceSourceSettings.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;

	bundleCreateParams.resourceSourceSettings.basePath = "C:/Users/gilbert/Documents/Notes/IncrementalBinaryPatching/VanguardTestData/ClientB/WindowsClient";

	bundleCreateParams.chunkDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_CDN;

	bundleCreateParams.chunkDestinationSettings.basePath = "VanguardData/ClientBBundle";

	bundleCreateParams.resourceBundleResourceGroupDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_RELATIVE;

	bundleCreateParams.resourceBundleResourceGroupDestinationSettings.basePath = "VanguardData";

	bundleCreateParams.chunkSize = 50000000;

	EXPECT_EQ( resourceGroup.CreateBundle( bundleCreateParams ), CarbonResources::Result::SUCCESS );


	// TODO test bundle output
}

TEST_F( CarbonResourcesLibraryTest, CreateVanguardUnBundle )
{
	EXPECT_TRUE( false );
}

TEST_F( CarbonResourcesLibraryTest, CreateVanguardPatchWithChunking )
{
	// Previous ResourceGroup
	CarbonResources::ResourceGroup resourceGroupPrevious;

	CarbonResources::ResourceGroupImportFromFileParams importParamsPrevious;

	importParamsPrevious.filename = "C:/Users/gilbert/Documents/CCP/Builds/carbon-resources/tests/Debug/VanguardData/ClientAResourceGroup.yaml";

	EXPECT_EQ( resourceGroupPrevious.ImportFromFile( importParamsPrevious ), CarbonResources::Result::SUCCESS );


	// Latest ResourceGroup
	CarbonResources::ResourceGroup resourceGroupLatest;

	CarbonResources::ResourceGroupImportFromFileParams importParamsLatest;

	importParamsLatest.filename = "C:/Users/gilbert/Documents/CCP/Builds/carbon-resources/tests/Debug/VanguardData/ClientBResourceGroup.yaml";

	EXPECT_EQ( resourceGroupLatest.ImportFromFile( importParamsLatest ), CarbonResources::Result::SUCCESS );



	// Create a patch from the subtraction index
	CarbonResources::PatchCreateParams patchCreateParams;

	patchCreateParams.resourceGroupRelativePath = "VanguardData/ResourceGroup_previousBuild_latestBuild.yaml";

	patchCreateParams.resourceGroupPatchRelativePath = "VanguardData/PatchResourceGroup_previousBuild_latestBuild.yaml";

	patchCreateParams.resourceSourceSettingsFrom.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;

	patchCreateParams.resourceSourceSettingsFrom.basePath = "C:/Users/gilbert/Documents/Notes/IncrementalBinaryPatching/VanguardTestData/ClientA/WindowsClient";

	patchCreateParams.resourceSourceSettingsTo.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;

	patchCreateParams.resourceSourceSettingsTo.basePath = "C:/Users/gilbert/Documents/Notes/IncrementalBinaryPatching/VanguardTestData/ClientB/WindowsClient";

	patchCreateParams.resourcePatchBinaryDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_CDN;

	patchCreateParams.resourcePatchBinaryDestinationSettings.basePath = "VanguardData/Patches/";

	patchCreateParams.resourcePatchResourceGroupDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_RELATIVE;

	patchCreateParams.resourcePatchResourceGroupDestinationSettings.basePath = "VanguardData/";

	patchCreateParams.patchFileRelativePathPrefix = "Patches/Patch1";

	patchCreateParams.previousResourceGroup = &resourceGroupPrevious;

	patchCreateParams.maxInputFileSize = 50000000;

	EXPECT_EQ( resourceGroupLatest.CreatePatch( patchCreateParams ), CarbonResources::Result::SUCCESS );


	// TODO run tests on patch create
}

TEST_F( CarbonResourcesLibraryTest, ApplyVanguardPatchWithChunking )
{
	// Load the patch file
	CarbonResources::PatchResourceGroup patchResourceGroup;

	CarbonResources::ResourceGroupImportFromFileParams importParamsPrevious;

	importParamsPrevious.filename = "C:/Users/gilbert/Documents/CCP/Builds/carbon-resources/tests/Debug/VanguardData/VanguardData/PatchResourceGroup_previousBuild_latestBuild.yaml";

	EXPECT_EQ( patchResourceGroup.ImportFromFile( importParamsPrevious ), CarbonResources::Result::SUCCESS );


	// Apply the patch
	CarbonResources::PatchApplyParams patchApplyParams;

	patchApplyParams.newBuildResourcesSourceSettings.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;

	patchApplyParams.newBuildResourcesSourceSettings.basePath = "C:/Users/gilbert/Documents/Notes/IncrementalBinaryPatching/VanguardTestData/ClientB/WindowsClient";

	patchApplyParams.patchBinarySourceSettings.sourceType = CarbonResources::ResourceSourceType::LOCAL_CDN;

	patchApplyParams.patchBinarySourceSettings.basePath = "C:/Users/gilbert/Documents/CCP/Builds/carbon-resources/tests/Debug/VanguardData/Patches";

	patchApplyParams.resourcesToPatchSourceSettings.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;

	patchApplyParams.resourcesToPatchSourceSettings.basePath = "C:/Users/gilbert/Documents/Notes/IncrementalBinaryPatching/VanguardTestData/ClientA/WindowsClient";

	patchApplyParams.resourcesToPatchDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_RELATIVE;

	patchApplyParams.resourcesToPatchDestinationSettings.basePath = "VanguardApplyPatchOut";

	patchApplyParams.temporaryFilePath = "tempFile.resource";

	EXPECT_EQ( patchResourceGroup.Apply( patchApplyParams ), CarbonResources::Result::SUCCESS );

	// TODO test the output of the applied patches
}
*/


/*
TEST_F( CarbonResourcesLibraryTest, CreateEVEPatchWithChunking )
{
	// Previous ResourceGroup
	CarbonResources::ResourceGroup resourceGroupPrevious;

	CarbonResources::ResourceGroupImportFromFileParams importParamsPrevious;

    // Can get this here: http://binaries.eveonline.com/1d/1d34143a37d4b739_bf55ef4a0a7e124bca0e569dcfdfd2bb
	importParamsPrevious.filename = "C:/Binary-Patching-Resources/LiveTQResIndex.txt";

	EXPECT_EQ( resourceGroupPrevious.ImportFromFile( importParamsPrevious ).type, CarbonResources::ResultType::SUCCESS );


	// Latest ResourceGroup
	CarbonResources::ResourceGroup resourceGroupLatest;

	CarbonResources::ResourceGroupImportFromFileParams importParamsLatest;

	importParamsLatest.filename = "C:/Binary-Patching-Resources/index/resfileindex.txt";

	EXPECT_EQ( resourceGroupLatest.ImportFromFile( importParamsLatest ).type, CarbonResources::ResultType::SUCCESS );

	// Create a patch from the subtraction index
	CarbonResources::PatchCreateParams patchCreateParams;

	patchCreateParams.resourceGroupRelativePath = "ResourceGroup_previousBuild_latestBuild.yaml";

	patchCreateParams.resourceGroupPatchRelativePath = "PatchResourceGroup_previousBuild_latestBuild.yaml";

	patchCreateParams.resourceSourceSettingsFrom.sourceType = CarbonResources::ResourceSourceType::REMOTE_CDN;

	patchCreateParams.resourceSourceSettingsFrom.basePath = "https://resources.eveonline.com";

	patchCreateParams.resourceSourceSettingsTo.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;

    // Can make some changes in the client/res folder to files that would be included. Inclusion can be inspected in the filter .ini files.
    // When I ran this there were soundback changes already anyway so patches would be created even with no changes.
	patchCreateParams.resourceSourceSettingsTo.basePath = "C:/Users/gilbert/Documents/CCP/Repos/Perforce/eve/branches/sandbox/2025-BINARY-PATCHING/eve/client/res";

	patchCreateParams.resourcePatchBinaryDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_CDN;

	patchCreateParams.resourcePatchBinaryDestinationSettings.basePath = "EVEData/Patches/";

	patchCreateParams.resourcePatchResourceGroupDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_RELATIVE;

	patchCreateParams.resourcePatchResourceGroupDestinationSettings.basePath = "EVEData/";

	patchCreateParams.patchFileRelativePathPrefix = "Patches/Patch1";

	patchCreateParams.previousResourceGroup = &resourceGroupPrevious;

	patchCreateParams.maxInputFileChunkSize = 50000000;

	EXPECT_EQ( resourceGroupLatest.CreatePatch( patchCreateParams ).type, CarbonResources::ResultType::SUCCESS );

    // TODO need to now create a bundle from the created PatchResourceGroup.
    // However first patchCreateParams.resourcePatchBinaryDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::REMOTE_CDN; needs to be changed to LOCAL_CDN
    // This way the resulting patches will not yet be compressed as bundle chunks will be compressed and we don't want to double compress.
    // As stated before the destination chunk type should be REMOTE_CDN so that the resulting chunks will be ready to be uploaded to the CDN, they are really the final patch data.
    // There is a problem in the logic of how bundles are currently chunked, more information has been written in the stub ResourceGroupImpl::CreateBundle
    // The fix is straightforward but relies on work not yet in, top of my head I think it was compression of large files.


}
	

TEST_F( CarbonResourcesLibraryTest, CreateBundleFromEVEPatch )
{
	// Now create a bundle from the patches
	// Import ResourceGroup
	CarbonResources::ResourceGroup resourceGroup;

	CarbonResources::ResourceGroupImportFromFileParams importParams;

	importParams.filename = "EVEData/PatchResourceGroup_previousBuild_latestBuild.yaml";

	EXPECT_EQ( resourceGroup.ImportFromFile( importParams ).type, CarbonResources::ResultType::SUCCESS );


	// Create a bundle from the ResourceGroup
	CarbonResources::BundleCreateParams bundleCreateParams;

	bundleCreateParams.resourceGroupRelativePath = "ResourceGroup.yaml"; // A copy is made of the resourceGroup

	bundleCreateParams.resourceGroupBundleRelativePath = "BundleResourceGroup.yaml";

	bundleCreateParams.resourceSourceSettings.sourceType = CarbonResources::ResourceSourceType::LOCAL_CDN;

	bundleCreateParams.resourceSourceSettings.basePath = "EVEData/Patches/";

	bundleCreateParams.chunkDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::REMOTE_CDN;

	bundleCreateParams.chunkDestinationSettings.basePath = "EVEData/Chunks/";

	bundleCreateParams.resourceBundleResourceGroupDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_RELATIVE;

	bundleCreateParams.resourceBundleResourceGroupDestinationSettings.basePath = "EVEData";

	bundleCreateParams.chunkSize = 50000000;

	EXPECT_EQ( resourceGroup.CreateBundle( bundleCreateParams ).type, CarbonResources::ResultType::SUCCESS );


}
*/