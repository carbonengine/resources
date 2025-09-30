// Copyright © 2025 CCP ehf.

#include <ResourceGroup.h>
#include <BundleResourceGroup.h>
#include <PatchResourceGroup.h>

#include "ResourcesTestFixture.h"

#include <gtest/gtest.h>

#include <FileDataStreamOut.h>

struct ResourcesLibraryTest : public ResourcesTestFixture
{
};

// Import ResourceGroup V0.0.0
// Export should output as V0.1.0
// This is the only instance that exporting a file of a lower version results in a version bump
TEST_F( ResourcesLibraryTest, BinaryGroupImportExport_V_0_0_0_To_V_0_1_0 )
{

	CarbonResources::ResourceGroup binaryResourceGroup;

	CarbonResources::ResourceGroupImportFromFileParams importParams;

	importParams.filename = GetTestFileFileAbsolutePath( "Indicies/binaryFileIndex_v0_0_0.txt" );

	EXPECT_EQ( binaryResourceGroup.ImportFromFile( importParams ).type, CarbonResources::ResultType::SUCCESS );

	CarbonResources::ResourceGroupExportToFileParams exportParams;

	exportParams.filename = "resPath/BinaryResourceGroup_v0_1_0.yaml";

	EXPECT_EQ( binaryResourceGroup.ExportToFile( exportParams ).type, CarbonResources::ResultType::SUCCESS );

	std::filesystem::path goldStandardFilename = GetTestFileFileAbsolutePath( "Indicies/BinaryResourceGroup_v0_1_0.yaml" );

	EXPECT_TRUE( FilesMatch( exportParams.filename, goldStandardFilename ) );
}

// Import a BinaryResourceGroup v0.1.0 and export it again checking input == output
TEST_F( ResourcesLibraryTest, BinaryGroupImportExport_V_0_1_0 )
{
	CarbonResources::ResourceGroup binaryResourceGroup;

	CarbonResources::ResourceGroupImportFromFileParams importParams;

	importParams.filename = GetTestFileFileAbsolutePath( "Indicies/BinaryResourceGroup_v0_1_0.yaml" );

	EXPECT_EQ( binaryResourceGroup.ImportFromFile( importParams ).type, CarbonResources::ResultType::SUCCESS );

	CarbonResources::ResourceGroupExportToFileParams exportParams;

	exportParams.filename = "resPath/BinaryResourceGroup_v0_1_0.yaml";

	EXPECT_EQ( binaryResourceGroup.ExportToFile( exportParams ).type, CarbonResources::ResultType::SUCCESS );

	EXPECT_TRUE( FilesMatch( importParams.filename, exportParams.filename ) );
}

// Import ResourceGroup V0.0.0
// Export should output as V0.1.0
// This is the only instance that exporting a file of a lower version results in a version bump
TEST_F( ResourcesLibraryTest, ResourceGroupImportExport_V_0_0_0_To_V_0_1_0 )
{
	CarbonResources::ResourceGroup resourceGroup;

	CarbonResources::ResourceGroupImportFromFileParams importParams;

	importParams.filename = GetTestFileFileAbsolutePath( "Indicies/resFileIndex_v0_0_0.txt" );

	EXPECT_EQ( resourceGroup.ImportFromFile( importParams ).type, CarbonResources::ResultType::SUCCESS );

	CarbonResources::ResourceGroupExportToFileParams exportParams;

	exportParams.filename = "resPath/ResourceGroup_v0_1_0.yaml";

	EXPECT_EQ( resourceGroup.ExportToFile( exportParams ).type, CarbonResources::ResultType::SUCCESS );

	std::filesystem::path goldStandardFilename = GetTestFileFileAbsolutePath( "Indicies/ResourceGroup_v0_1_0.yaml" );

	EXPECT_TRUE( FilesMatch( exportParams.filename, goldStandardFilename ) );
}

TEST_F( ResourcesLibraryTest, ImportEmptyResourceGroup )
{
	CarbonResources::ResourceGroup resourceGroup;
	CarbonResources::ResourceGroupImportFromFileParams importParams;
	importParams.filename = GetTestFileFileAbsolutePath( "ResourceGroups/EmptyResourceGroup.yaml" );
	EXPECT_EQ( resourceGroup.ImportFromFile( importParams ).type, CarbonResources::ResultType::SUCCESS );
}

TEST_F( ResourcesLibraryTest, ImportResourceGroupWithOutOfBoundsBinaryOperation )
{
	CarbonResources::ResourceGroup resourceGroup;
	CarbonResources::ResourceGroupImportFromFileParams importParams;
	importParams.filename = GetTestFileFileAbsolutePath( "Indicies/resFileIndex_v0_0_0-OutOfBoundsBinaryOp.txt" );
	EXPECT_EQ( resourceGroup.ImportFromFile( importParams ).type, CarbonResources::ResultType::MALFORMED_RESOURCE_INPUT );
}

void CreateEmptyResourceGroupWithMissingParameter( std::filesystem::path emptyResourceGroupPath, std::filesystem::path outPath, const std::string& missingTagName )
{
	std::filesystem::path testDataFolder( TEST_DATA_BASE_PATH );
	std::ifstream fileIn( emptyResourceGroupPath );
	create_directories( outPath.parent_path() );
	std::ofstream fileOut( outPath, std::ios::out );
	ASSERT_TRUE( fileOut.is_open() );
	std::string line;
	while( std::getline( fileIn, line ) )
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
	std::filesystem::path outPath( std::filesystem::current_path() / "ResourceGroups" / ( "Missing" + tag + ".yaml" ) );
	CreateEmptyResourceGroupWithMissingParameter( emptyResourceGroupPath, outPath, tag );
	CarbonResources::ResourceGroup resourceGroup;
	CarbonResources::ResourceGroupImportFromFileParams importParams;
	importParams.filename = outPath;
	return resourceGroup.ImportFromFile( importParams );
}

// Import a ResourceGroup with missing parameters that are expected for the version provided
// This should fail gracefully and give appropriate feedback to user
TEST_F( ResourcesLibraryTest, ResourceGroupHandleImportMissingParametersForVersion )
{
	std::filesystem::path emptyResourceGroupPath = GetTestFileFileAbsolutePath( "ResourceGroups/EmptyResourceGroup.yaml" );
	EXPECT_EQ( AttemptImportResourceGroupMissingParameter( "Version", emptyResourceGroupPath ).type, CarbonResources::ResultType::MALFORMED_RESOURCE_GROUP );
	EXPECT_EQ( AttemptImportResourceGroupMissingParameter( "Type", emptyResourceGroupPath ).type, CarbonResources::ResultType::MALFORMED_RESOURCE_GROUP );
	EXPECT_EQ( AttemptImportResourceGroupMissingParameter( "NumberOfResources", emptyResourceGroupPath ).type, CarbonResources::ResultType::MALFORMED_RESOURCE_GROUP );
	EXPECT_EQ( AttemptImportResourceGroupMissingParameter( "TotalResourcesSizeCompressed", emptyResourceGroupPath ).type, CarbonResources::ResultType::MALFORMED_RESOURCE_GROUP );
	EXPECT_EQ( AttemptImportResourceGroupMissingParameter( "TotalResourcesSizeUnCompressed", emptyResourceGroupPath ).type, CarbonResources::ResultType::MALFORMED_RESOURCE_GROUP );
	EXPECT_EQ( AttemptImportResourceGroupMissingParameter( "Resources", emptyResourceGroupPath ).type, CarbonResources::ResultType::MALFORMED_RESOURCE_GROUP );
}


TEST_F( ResourcesLibraryTest, ResourceGroupLoadInvalidYaml )
{
	CarbonResources::ResourceGroup resourceGroup;
	CarbonResources::ResourceGroupImportFromFileParams importParams;

	// Should try to load the group but fail to parse the yaml
	importParams.filename = GetTestFileFileAbsolutePath( "ResourcesRemote/a9/a9d1721dd5cc6d54_4d7a8d216f4c8c5c6379476c0668fe84" );
	EXPECT_EQ( resourceGroup.ImportFromFile( importParams ).type, CarbonResources::ResultType::FAILED_TO_PARSE_YAML );
}

TEST_F( ResourcesLibraryTest, ResourceGroupLoadInvalidCsv )
{

	CarbonResources::ResourceGroup resourceGroup;
	CarbonResources::ResourceGroupImportFromFileParams importParams;

	// Try and load group but fail to parse csv
	importParams.filename = GetTestFileFileAbsolutePath( "Indicies/resFileIndex_v0_0_0_NONESENSE.txt" );
	EXPECT_EQ( resourceGroup.ImportFromFile( importParams ).type, CarbonResources::ResultType::MALFORMED_RESOURCE_INPUT );
}

TEST_F( ResourcesLibraryTest, ResourceGroupLoadCsvWithInvalidCompressedSizeField )
{

	CarbonResources::ResourceGroup resourceGroup;
	CarbonResources::ResourceGroupImportFromFileParams importParams;

	// Try and load group but fail to parse csv
	importParams.filename = GetTestFileFileAbsolutePath( "Indicies/resFileIndex_v0_0_0_INVALID.txt" );
	EXPECT_EQ( resourceGroup.ImportFromFile( importParams ).type, CarbonResources::ResultType::MALFORMED_RESOURCE_INPUT );
}

TEST_F( ResourcesLibraryTest, ResourceGroupLoadEmptyCsv )
{

	CarbonResources::ResourceGroup resourceGroup;
	CarbonResources::ResourceGroupImportFromFileParams importParams;

	// Try and load group, results in success but empty list
	importParams.filename = GetTestFileFileAbsolutePath( "Indicies/resFileIndex_v0_0_0_EMPTY.txt" );
	EXPECT_EQ( resourceGroup.ImportFromFile( importParams ).type, CarbonResources::ResultType::SUCCESS );
}

TEST_F( ResourcesLibraryTest, ResourceGroupLoadNonexistantFileFails )
{

	CarbonResources::ResourceGroup resourceGroup;
	CarbonResources::ResourceGroupImportFromFileParams importParams;

	// Load a file that does not exist
	importParams.filename = GetTestFileFileAbsolutePath( "ResourcesRemote/a9/a9d1721dd5cc6d54_4d7a8d216f4c8c5c6379476c0668fe84.yaml" );
	EXPECT_EQ( resourceGroup.ImportFromFile( importParams ).type, CarbonResources::ResultType::FAILED_TO_OPEN_FILE );
}

TEST_F( ResourcesLibraryTest, ResourceGroupWithInvalidExtensionFails )
{
	CarbonResources::ResourceGroup resourceGroup;
	CarbonResources::ResourceGroupImportFromFileParams importParams;

	// Load a file with a file extension indicating an unsupported format
	importParams.filename = GetTestFileFileAbsolutePath( "Bundle/TestResources/One.png" );
	EXPECT_EQ( resourceGroup.ImportFromFile( importParams ).type, CarbonResources::ResultType::UNSUPPORTED_FILE_FORMAT );
}

// Import a ResourceGroup with version greater than current document minor version specified in enums.h
// This should open ignoring anything extra added in the future version
// The version of the imported ResourceGroup should be set at the max supported version in enums.h
TEST_F( ResourcesLibraryTest, ResourceGroupImportNewerMinorVersion )
{
	CarbonResources::ResourceGroup resourceGroup;
	CarbonResources::ResourceGroupImportFromFileParams importParams;
	importParams.filename = GetTestFileFileAbsolutePath( "ResourceGroups/HigherMinorVersion.yaml" );
	EXPECT_EQ( resourceGroup.ImportFromFile( importParams ).type, CarbonResources::ResultType::SUCCESS );
}

// Import a ResourceGroup with version greater than current document major version specified in enums.h
// This should gracefully fail to open
TEST_F( ResourcesLibraryTest, ResourceGroupImportNewerMajorVersion )
{
	CarbonResources::ResourceGroup resourceGroup;
	CarbonResources::ResourceGroupImportFromFileParams importParams;
	importParams.filename = GetTestFileFileAbsolutePath( "ResourceGroups/HigherMajorVersion.yaml" );
	EXPECT_EQ( resourceGroup.ImportFromFile( importParams ).type, CarbonResources::ResultType::DOCUMENT_VERSION_UNSUPPORTED );
}

// Import a ResourceGroup that doesn't exist
// This should gracefully fail
TEST_F( ResourcesLibraryTest, ResourceGroupImportNonExistantFile )
{
	CarbonResources::ResourceGroup resourceGroup;

	CarbonResources::ResourceGroupImportFromFileParams importParams;

	importParams.filename = GetTestFileFileAbsolutePath( "NonexistentFiles/ResourceGroup_which_does_not_exist.yaml" );

	EXPECT_EQ( resourceGroup.ImportFromFile( importParams ).type, CarbonResources::ResultType::FAILED_TO_OPEN_FILE );
}

// Import a ResourceGroup v0.1.0 and export it again checking input == output
TEST_F( ResourcesLibraryTest, ResourceGroupImportExport_V_0_1_0 )
{

	CarbonResources::ResourceGroup resourceGroup;

	CarbonResources::ResourceGroupImportFromFileParams importParams;

	importParams.filename = GetTestFileFileAbsolutePath( "Indicies/ResourceGroup_v0_1_0.yaml" );

	EXPECT_EQ( resourceGroup.ImportFromFile( importParams ).type, CarbonResources::ResultType::SUCCESS );

	CarbonResources::ResourceGroupExportToFileParams exportParams;

	exportParams.filename = "resPath/ResourceGroup_v0_1_0.yaml";

	EXPECT_EQ( resourceGroup.ExportToFile( exportParams ).type, CarbonResources::ResultType::SUCCESS );

	EXPECT_TRUE( FilesMatch( importParams.filename, exportParams.filename ) );
}

TEST_F( ResourcesLibraryTest, UnpackBundle )
{
	// Load the bundle file
	CarbonResources::BundleResourceGroup bundleResourceGroup;

	CarbonResources::ResourceGroupImportFromFileParams importParamsPrevious;

	importParamsPrevious.filename = GetTestFileFileAbsolutePath( "Bundle/BundleResourceGroup.yaml" );

	EXPECT_EQ( bundleResourceGroup.ImportFromFile( importParamsPrevious ).type, CarbonResources::ResultType::SUCCESS );


	// Unpack the bundle
	CarbonResources::BundleUnpackParams bundleUnpackParams;

	bundleUnpackParams.chunkSourceSettings.sourceType = CarbonResources::ResourceSourceType::LOCAL_CDN;

	bundleUnpackParams.chunkSourceSettings.basePaths = { GetTestFileFileAbsolutePath( "Bundle/LocalRemoteChunks/" ) };

	bundleUnpackParams.resourceDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_RELATIVE;

	bundleUnpackParams.resourceDestinationSettings.basePath = "UnpackBundleOut/";

	EXPECT_EQ( bundleResourceGroup.Unpack( bundleUnpackParams ).type, CarbonResources::ResultType::SUCCESS );

	EXPECT_TRUE( DirectoryIsSubset( GetTestFileFileAbsolutePath( "Bundle/Res" ), "UnpackBundleOut" ) );

	EXPECT_TRUE( std::filesystem::exists( "UnpackBundleOut/ResourceGroup.yaml" ) );
}

TEST_F( ResourcesLibraryTest, UnpackBundleExpectingRemoteCdnButPassedLocalCdn )
{
	// Load the bundle file
	CarbonResources::BundleResourceGroup bundleResourceGroup;

	CarbonResources::ResourceGroupImportFromFileParams importParamsPrevious;

	importParamsPrevious.filename = GetTestFileFileAbsolutePath( "Bundle/BundleResourceGroup.yaml" );

	EXPECT_EQ( bundleResourceGroup.ImportFromFile( importParamsPrevious ).type, CarbonResources::ResultType::SUCCESS );


	// Unpack the bundle
	CarbonResources::BundleUnpackParams bundleUnpackParams;

	bundleUnpackParams.chunkSourceSettings.sourceType = CarbonResources::ResourceSourceType::REMOTE_CDN; // source is LOCAL_CDN

	bundleUnpackParams.chunkSourceSettings.basePaths = { GetTestFileFileAbsolutePath( "Bundle/LocalRemoteChunks/" ) };

	bundleUnpackParams.resourceDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_RELATIVE;

	bundleUnpackParams.resourceDestinationSettings.basePath = "UnpackBundleOut/";

	// Should fail
	EXPECT_EQ( bundleResourceGroup.Unpack( bundleUnpackParams ).type, CarbonResources::ResultType::FAILED_TO_OPEN_FILE );
}

TEST_F( ResourcesLibraryTest, UnpackRemoteBundleAsLocal )
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
	bundleCreateParams.resourceSourceSettings.basePaths = { GetTestFileFileAbsolutePath( "Bundle/Res/" ) };
	bundleCreateParams.chunkDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::REMOTE_CDN;
	bundleCreateParams.chunkDestinationSettings.basePath = "UnpackRemoteBundleAsLocal/Chunks";
	bundleCreateParams.resourceBundleResourceGroupDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_RELATIVE;
	bundleCreateParams.resourceBundleResourceGroupDestinationSettings.basePath = "UnpackRemoteBundleAsLocal";
	bundleCreateParams.chunkSize = 1000000000;
	EXPECT_EQ( resourceGroup.CreateBundle( bundleCreateParams ).type, CarbonResources::ResultType::SUCCESS );

	// Load the bundle file
	CarbonResources::BundleResourceGroup bundleResourceGroup;
	CarbonResources::ResourceGroupImportFromFileParams importParamsPrevious;
	importParamsPrevious.filename = "UnpackRemoteBundleAsLocal/BundleResourceGroup.yaml";
	EXPECT_EQ( bundleResourceGroup.ImportFromFile( importParamsPrevious ).type, CarbonResources::ResultType::SUCCESS );

	// Attempt to unpack the bundle
	CarbonResources::BundleUnpackParams bundleUnpackParams;
	bundleUnpackParams.chunkSourceSettings.sourceType = CarbonResources::ResourceSourceType::LOCAL_CDN;
	bundleUnpackParams.chunkSourceSettings.basePaths = { "UnpackRemoteBundleAsLocal/Chunks/" };
	bundleUnpackParams.resourceDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_RELATIVE;
	bundleUnpackParams.resourceDestinationSettings.basePath = "UnpackRemoteBundleAsLocal/Unpacked";

	// This should fail, since the chunks are unexpectedly gzipped.
	EXPECT_EQ( bundleResourceGroup.Unpack( bundleUnpackParams ).type, CarbonResources::ResultType::FAILED_TO_PARSE_YAML );
}

TEST_F( ResourcesLibraryTest, CreateBundleWithZeroChunkSize )
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

	bundleCreateParams.resourceSourceSettings.basePaths = { GetTestFileFileAbsolutePath( "Bundle/Res/" ) };

	bundleCreateParams.chunkDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_CDN;

	bundleCreateParams.chunkDestinationSettings.basePath = "CreateBundleOut";

	bundleCreateParams.resourceBundleResourceGroupDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_RELATIVE;

	bundleCreateParams.resourceBundleResourceGroupDestinationSettings.basePath = "resPath";

	bundleCreateParams.chunkSize = 0;

	EXPECT_EQ( resourceGroup.CreateBundle( bundleCreateParams ).type, CarbonResources::ResultType::INVALID_CHUNK_SIZE );
}

TEST_F( ResourcesLibraryTest, CreateBundle )
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

	bundleCreateParams.resourceSourceSettings.basePaths = { GetTestFileFileAbsolutePath( "Bundle/Res/" ) };

	bundleCreateParams.chunkDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_CDN;

	bundleCreateParams.chunkDestinationSettings.basePath = "CreateBundleOut";

	bundleCreateParams.resourceBundleResourceGroupDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_RELATIVE;

	bundleCreateParams.resourceBundleResourceGroupDestinationSettings.basePath = "resPath";

	bundleCreateParams.chunkSize = 1000;

	EXPECT_EQ( resourceGroup.CreateBundle( bundleCreateParams ).type, CarbonResources::ResultType::SUCCESS );

	EXPECT_TRUE( FilesMatch( "resPath/BundleResourceGroup.yaml", GetTestFileFileAbsolutePath( "CreateBundle/BundleResourceGroup.yaml" ) ) );
	EXPECT_TRUE( DirectoryIsSubset( "CreateBundleOut", GetTestFileFileAbsolutePath( "CreateBundle/CreateBundleOut" ) ) );
}

TEST_F( ResourcesLibraryTest, CreateAndUnpackBundle )
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

	bundleCreateParams.resourceSourceSettings.basePaths = { GetTestFileFileAbsolutePath( "Bundle/Res/" ) };

	bundleCreateParams.chunkDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_CDN;

	bundleCreateParams.chunkDestinationSettings.basePath = "CreateAndUnpackBundleOut";

	bundleCreateParams.resourceBundleResourceGroupDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_RELATIVE;

	bundleCreateParams.resourceBundleResourceGroupDestinationSettings.basePath = "resPath";

	bundleCreateParams.chunkSize = 1000;

	EXPECT_EQ( resourceGroup.CreateBundle( bundleCreateParams ).type, CarbonResources::ResultType::SUCCESS );

	// Unpack the bundle
	// Load the bundle file
	CarbonResources::BundleResourceGroup bundleResourceGroup;

	CarbonResources::ResourceGroupImportFromFileParams importParamsPrevious;

	importParamsPrevious.filename = bundleCreateParams.resourceBundleResourceGroupDestinationSettings.basePath / bundleCreateParams.resourceGroupBundleRelativePath;

	EXPECT_EQ( bundleResourceGroup.ImportFromFile( importParamsPrevious ).type, CarbonResources::ResultType::SUCCESS );



	// Unpack the bundle
	CarbonResources::BundleUnpackParams bundleUnpackParams;

	bundleUnpackParams.chunkSourceSettings.sourceType = CarbonResources::ResourceSourceType::LOCAL_CDN;

	bundleUnpackParams.chunkSourceSettings.basePaths = { bundleCreateParams.chunkDestinationSettings.basePath };

	bundleUnpackParams.resourceDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_RELATIVE;

	bundleUnpackParams.resourceDestinationSettings.basePath = "CreateAndUnpackBundleOut2/";

	EXPECT_EQ( bundleResourceGroup.Unpack( bundleUnpackParams ).type, CarbonResources::ResultType::SUCCESS );

	EXPECT_TRUE( DirectoryIsSubset( bundleCreateParams.resourceSourceSettings.basePaths.at( 0 ), bundleUnpackParams.resourceDestinationSettings.basePath ) );

	std::filesystem::path unpackedGroupPath = bundleUnpackParams.resourceDestinationSettings.basePath / "ResourceGroup.yaml";

	EXPECT_TRUE( std::filesystem::exists( unpackedGroupPath ) );
}

TEST_F( ResourcesLibraryTest, ApplyPatch )
{
	// Load the patch file
	CarbonResources::PatchResourceGroup patchResourceGroup;

	CarbonResources::ResourceGroupImportFromFileParams importParamsPrevious;

	importParamsPrevious.filename = GetTestFileFileAbsolutePath( "Patch/PatchResourceGroup.yaml" );

	EXPECT_EQ( patchResourceGroup.ImportFromFile( importParamsPrevious ).type, CarbonResources::ResultType::SUCCESS );


	// Apply the patch
	CarbonResources::PatchApplyParams patchApplyParams;

	patchApplyParams.nextBuildResourcesSourceSettings.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;

	patchApplyParams.nextBuildResourcesSourceSettings.basePaths = { GetTestFileFileAbsolutePath( "Patch/NextBuildResources/" ) };

	patchApplyParams.patchBinarySourceSettings.sourceType = CarbonResources::ResourceSourceType::LOCAL_CDN;

	patchApplyParams.patchBinarySourceSettings.basePaths = { GetTestFileFileAbsolutePath( "Patch/LocalCDNPatches/" ) };

	patchApplyParams.resourcesToPatchSourceSettings.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;

	patchApplyParams.resourcesToPatchSourceSettings.basePaths = { GetTestFileFileAbsolutePath( "Patch/PreviousBuildResources/" ) };

	patchApplyParams.resourcesToPatchDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_RELATIVE;

	patchApplyParams.resourcesToPatchDestinationSettings.basePath = "ApplyPatchOut";

	patchApplyParams.temporaryFilePath = "tempFile.resource";

	if( std::filesystem::exists( patchApplyParams.resourcesToPatchDestinationSettings.basePath ) )
	{
		std::filesystem::remove_all( patchApplyParams.resourcesToPatchDestinationSettings.basePath );
	}

	std::filesystem::copy( patchApplyParams.resourcesToPatchSourceSettings.basePaths[0], patchApplyParams.resourcesToPatchDestinationSettings.basePath );

	EXPECT_EQ( patchResourceGroup.Apply( patchApplyParams ).type, CarbonResources::ResultType::SUCCESS );

	// Check Expected Outcome
	std::filesystem::path goldDirectory = GetTestFileFileAbsolutePath( "Patch/NextBuildResources" );
	EXPECT_TRUE( DirectoryIsSubset( patchApplyParams.resourcesToPatchDestinationSettings.basePath, goldDirectory ) );
}
TEST_F( ResourcesLibraryTest, CreatePatchWhereBuildsHaveNoChanges )
{
	// Previous ResourceGroup
	CarbonResources::ResourceGroup resourceGroupPrevious;

	CarbonResources::ResourceGroupImportFromFileParams importParamsPrevious;

	importParamsPrevious.filename = GetTestFileFileAbsolutePath( "Patch/resfileindexShort_build_previous.txt" );

	EXPECT_EQ( resourceGroupPrevious.ImportFromFile( importParamsPrevious ).type, CarbonResources::ResultType::SUCCESS );


	// Latest ResourceGroup
	CarbonResources::ResourceGroup resourceGroupLatest;

	CarbonResources::ResourceGroupImportFromFileParams importParamsLatest;

	importParamsLatest.filename = GetTestFileFileAbsolutePath( "Patch/resfileindexShort_build_previous.txt" );

	EXPECT_EQ( resourceGroupLatest.ImportFromFile( importParamsLatest ).type, CarbonResources::ResultType::SUCCESS );

	// Create a patch from the subtraction index
	CarbonResources::PatchCreateParams patchCreateParams;

	patchCreateParams.resourceGroupRelativePath = "ResourceGroup.yaml";

	patchCreateParams.resourceGroupPatchRelativePath = "PatchResourceGroup.yaml";

	patchCreateParams.resourceSourceSettingsPrevious.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;

	patchCreateParams.resourceSourceSettingsPrevious.basePaths = { GetTestFileFileAbsolutePath( "Patch/PreviousBuildResources" ) };

	patchCreateParams.resourceSourceSettingsNext.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;

	patchCreateParams.resourceSourceSettingsNext.basePaths = { GetTestFileFileAbsolutePath( "Patch/PreviousBuildResources" ) };

	patchCreateParams.resourcePatchBinaryDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_CDN;

	patchCreateParams.resourcePatchBinaryDestinationSettings.basePath = "SharedCache";

	patchCreateParams.resourcePatchResourceGroupDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_RELATIVE;

	patchCreateParams.resourcePatchResourceGroupDestinationSettings.basePath = "resPath";

	patchCreateParams.patchFileRelativePathPrefix = "Patches/Patch";

	patchCreateParams.previousResourceGroup = &resourceGroupPrevious;

	patchCreateParams.maxInputFileChunkSize = 50000000;

	EXPECT_EQ( resourceGroupLatest.CreatePatch( patchCreateParams ).type, CarbonResources::ResultType::SUCCESS );

	// Check expected outcome
}

TEST_F( ResourcesLibraryTest, CreatePatch )
{
	// Previous ResourceGroup
	CarbonResources::ResourceGroup resourceGroupPrevious;

	CarbonResources::ResourceGroupImportFromFileParams importParamsPrevious;

	importParamsPrevious.filename = GetTestFileFileAbsolutePath( "Patch/resfileindexShort_build_previous.txt" );

	EXPECT_EQ( resourceGroupPrevious.ImportFromFile( importParamsPrevious ).type, CarbonResources::ResultType::SUCCESS );


	// Latest ResourceGroup
	CarbonResources::ResourceGroup resourceGroupLatest;

	CarbonResources::ResourceGroupImportFromFileParams importParamsLatest;

	importParamsLatest.filename = GetTestFileFileAbsolutePath( "Patch/resfileindexShort_build_next.txt" );

	EXPECT_EQ( resourceGroupLatest.ImportFromFile( importParamsLatest ).type, CarbonResources::ResultType::SUCCESS );

	// Create a patch from the subtraction index
	CarbonResources::PatchCreateParams patchCreateParams;

	patchCreateParams.resourceGroupRelativePath = "ResourceGroup.yaml";

	patchCreateParams.resourceGroupPatchRelativePath = "PatchResourceGroup.yaml";

	patchCreateParams.resourceSourceSettingsPrevious.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;

	patchCreateParams.resourceSourceSettingsPrevious.basePaths = { GetTestFileFileAbsolutePath( "Patch/PreviousBuildResources" ) };

	patchCreateParams.resourceSourceSettingsNext.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;

	patchCreateParams.resourceSourceSettingsNext.basePaths = { GetTestFileFileAbsolutePath( "Patch/NextBuildResources" ) };

	patchCreateParams.resourcePatchBinaryDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_CDN;

	patchCreateParams.resourcePatchBinaryDestinationSettings.basePath = "SharedCache";

	patchCreateParams.resourcePatchResourceGroupDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_RELATIVE;

	patchCreateParams.resourcePatchResourceGroupDestinationSettings.basePath = "resPath";

	patchCreateParams.patchFileRelativePathPrefix = "Patches/Patch";

	patchCreateParams.previousResourceGroup = &resourceGroupPrevious;

	patchCreateParams.maxInputFileChunkSize = 50000000;

	EXPECT_EQ( resourceGroupLatest.CreatePatch( patchCreateParams ).type, CarbonResources::ResultType::SUCCESS );

	// Check expected outcome
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "Patch/PatchResourceGroup.yaml" );
	EXPECT_TRUE( FilesMatch( goldFile, patchCreateParams.resourcePatchResourceGroupDestinationSettings.basePath / "PatchResourceGroup.yaml" ) );

	std::filesystem::path goldDirectory = GetTestFileFileAbsolutePath( "Patch/LocalCDNPatches" );
	EXPECT_TRUE( DirectoryIsSubset( goldDirectory, patchCreateParams.resourcePatchBinaryDestinationSettings.basePath ) );
}

TEST_F( ResourcesLibraryTest, CreatePatchZeroInputChunkSize )
{
	// Previous ResourceGroup
	CarbonResources::ResourceGroup resourceGroupPrevious;

	CarbonResources::ResourceGroupImportFromFileParams importParamsPrevious;

	importParamsPrevious.filename = GetTestFileFileAbsolutePath( "Patch/resfileindexShort_build_previous.txt" );

	EXPECT_EQ( resourceGroupPrevious.ImportFromFile( importParamsPrevious ).type, CarbonResources::ResultType::SUCCESS );


	// Latest ResourceGroup
	CarbonResources::ResourceGroup resourceGroupLatest;

	CarbonResources::ResourceGroupImportFromFileParams importParamsLatest;

	importParamsLatest.filename = GetTestFileFileAbsolutePath( "Patch/resfileindexShort_build_next.txt" );

	EXPECT_EQ( resourceGroupLatest.ImportFromFile( importParamsLatest ).type, CarbonResources::ResultType::SUCCESS );

	// Create a patch from the subtraction index
	CarbonResources::PatchCreateParams patchCreateParams;

	patchCreateParams.resourceGroupRelativePath = "ResourceGroup.yaml";

	patchCreateParams.resourceGroupPatchRelativePath = "PatchResourceGroup.yaml";

	patchCreateParams.resourceSourceSettingsPrevious.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;

	patchCreateParams.resourceSourceSettingsPrevious.basePaths = { GetTestFileFileAbsolutePath( "Patch/PreviousBuildResources" ) };

	patchCreateParams.resourceSourceSettingsNext.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;

	patchCreateParams.resourceSourceSettingsNext.basePaths = { GetTestFileFileAbsolutePath( "Patch/NextBuildResources" ) };

	patchCreateParams.resourcePatchBinaryDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_CDN;

	patchCreateParams.resourcePatchBinaryDestinationSettings.basePath = "SharedCache";

	patchCreateParams.resourcePatchResourceGroupDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_RELATIVE;

	patchCreateParams.resourcePatchResourceGroupDestinationSettings.basePath = "resPath";

	patchCreateParams.patchFileRelativePathPrefix = "Patches/Patch";

	patchCreateParams.previousResourceGroup = &resourceGroupPrevious;

	patchCreateParams.maxInputFileChunkSize = 0;

	EXPECT_EQ( resourceGroupLatest.CreatePatch( patchCreateParams ).type, CarbonResources::ResultType::INVALID_CHUNK_SIZE );
}

TEST_F( ResourcesLibraryTest, ApplyPatchWithChunking )
{
	// Load the patch file
	CarbonResources::PatchResourceGroup patchResourceGroup;

	CarbonResources::ResourceGroupImportFromFileParams importParamsPrevious;

	importParamsPrevious.filename = GetTestFileFileAbsolutePath( "PatchWithInputChunk/PatchResourceGroup_previousBuild_latestBuild.yaml" );

	EXPECT_EQ( patchResourceGroup.ImportFromFile( importParamsPrevious ).type, CarbonResources::ResultType::SUCCESS );


	// Apply the patch
	CarbonResources::PatchApplyParams patchApplyParams;

	patchApplyParams.nextBuildResourcesSourceSettings.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;

	patchApplyParams.nextBuildResourcesSourceSettings.basePaths = { GetTestFileFileAbsolutePath( "PatchWithInputChunk/NextBuildResources/" ) };

	patchApplyParams.patchBinarySourceSettings.sourceType = CarbonResources::ResourceSourceType::LOCAL_CDN;

	patchApplyParams.patchBinarySourceSettings.basePaths = { GetTestFileFileAbsolutePath( "PatchWithInputChunk/LocalCDNPatches/" ) };

	patchApplyParams.resourcesToPatchSourceSettings.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;

	patchApplyParams.resourcesToPatchSourceSettings.basePaths = { GetTestFileFileAbsolutePath( "PatchWithInputChunk/PreviousBuildResources/" ) };

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

TEST_F( ResourcesLibraryTest, CreatePatchWithChunking )
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

	patchCreateParams.resourceSourceSettingsPrevious.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;

	patchCreateParams.resourceSourceSettingsPrevious.basePaths = { GetTestFileFileAbsolutePath( "PatchWithInputChunk/PreviousBuildResources" ) };

	patchCreateParams.resourceSourceSettingsNext.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;

	patchCreateParams.resourceSourceSettingsNext.basePaths = { GetTestFileFileAbsolutePath( "PatchWithInputChunk/NextBuildResources" ) };

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
	EXPECT_TRUE( DirectoryIsSubset( goldDirectory, patchCreateParams.resourcePatchBinaryDestinationSettings.basePath ) );
}

TEST_F( ResourcesLibraryTest, CreateResourceGroupFromDirectory )
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

TEST_F( ResourcesLibraryTest, CreateResourceGroupFromDirectoryOutputPathIsInvalid )
{
	CarbonResources::ResourceGroup resourceGroup;

	CarbonResources::CreateResourceGroupFromDirectoryParams createResourceGroupParams;

	createResourceGroupParams.directory = GetTestFileFileAbsolutePath( "CreateResourceFiles/ResourceFiles" );

	EXPECT_EQ( resourceGroup.CreateFromDirectory( createResourceGroupParams ).type, CarbonResources::ResultType::SUCCESS );

	CarbonResources::ResourceGroupExportToFileParams exportParams;

	exportParams.filename = "///";

	EXPECT_EQ( resourceGroup.ExportToFile( exportParams ).type, CarbonResources::ResultType::FAILED_TO_SAVE_FILE );
}

TEST_F( ResourcesLibraryTest, CreateResourceGroupFailsWithInvalidInputDirectory )
{
	CarbonResources::ResourceGroup resourceGroup;

	CarbonResources::CreateResourceGroupFromDirectoryParams createResourceGroupParams;

	createResourceGroupParams.directory = "INVALID_PATH";

	EXPECT_EQ( resourceGroup.CreateFromDirectory( createResourceGroupParams ).type, CarbonResources::ResultType::INPUT_DIRECTORY_DOESNT_EXIST );
}

TEST_F( ResourcesLibraryTest, DiffResourceGroupsWithTwoAdditions )
{
	// Load base group
	CarbonResources::ResourceGroup baseResourceGroup;

	CarbonResources::ResourceGroupImportFromFileParams importFromFileParams;

	importFromFileParams.filename = GetTestFileFileAbsolutePath( "DiffGroups/resFileIndex.txt" );

	CarbonResources::Result importFromFileResult = baseResourceGroup.ImportFromFile( importFromFileParams );

	EXPECT_EQ( importFromFileResult.type, CarbonResources::ResultType::SUCCESS );


	// Load group with additions
	CarbonResources::ResourceGroup resourceGroupWithAdditions;

	CarbonResources::ResourceGroupImportFromFileParams importFromFileParamsWithAdditions;

	importFromFileParamsWithAdditions.filename = GetTestFileFileAbsolutePath( "DiffGroups/resFileIndexWithAdditions.txt" );

	CarbonResources::Result importFromFileResultAdditions = resourceGroupWithAdditions.ImportFromFile( importFromFileParamsWithAdditions );

	EXPECT_EQ( importFromFileResultAdditions.type, CarbonResources::ResultType::SUCCESS );


	// Perform diff
	CarbonResources::ResourceGroupDiffAgainstGroupParams diffAgainstGroupParams;

	diffAgainstGroupParams.resourceGroupToDiffAgainst = &baseResourceGroup;

	std::vector<std::filesystem::path> additions;

	std::vector<std::filesystem::path> subtractions;

	diffAgainstGroupParams.additions = &additions;

	diffAgainstGroupParams.subtractions = &subtractions;

	CarbonResources::Result diffResult = resourceGroupWithAdditions.DiffAgainstGroup( diffAgainstGroupParams );

	EXPECT_EQ( diffResult.type, CarbonResources::ResultType::SUCCESS );


	// Test the result
	EXPECT_EQ( additions.size(), 2 );
}

TEST_F( ResourcesLibraryTest, DiffResourceGroupsWithTwoChanges )
{
	// Load base group
	CarbonResources::ResourceGroup baseResourceGroup;

	CarbonResources::ResourceGroupImportFromFileParams importFromFileParams;

	importFromFileParams.filename = GetTestFileFileAbsolutePath( "DiffGroups/resFileIndex.txt" );

	CarbonResources::Result importFromFileResult = baseResourceGroup.ImportFromFile( importFromFileParams );

	EXPECT_EQ( importFromFileResult.type, CarbonResources::ResultType::SUCCESS );


	// Load group with Changes
	CarbonResources::ResourceGroup resourceGroupWithChanges;

	CarbonResources::ResourceGroupImportFromFileParams importFromFileParamsWithChanges;

	importFromFileParamsWithChanges.filename = GetTestFileFileAbsolutePath( "DiffGroups/resFileIndexWithChanges.txt" );

	CarbonResources::Result importFromFileResultChanges = resourceGroupWithChanges.ImportFromFile( importFromFileParamsWithChanges );

	EXPECT_EQ( importFromFileResultChanges.type, CarbonResources::ResultType::SUCCESS );


	// Perform diff
	CarbonResources::ResourceGroupDiffAgainstGroupParams diffAgainstGroupParams;

	diffAgainstGroupParams.resourceGroupToDiffAgainst = &baseResourceGroup;

	std::vector<std::filesystem::path> additions;

	std::vector<std::filesystem::path> subtractions;

	diffAgainstGroupParams.additions = &additions;

	diffAgainstGroupParams.subtractions = &subtractions;

	CarbonResources::Result diffResult = resourceGroupWithChanges.DiffAgainstGroup( diffAgainstGroupParams );

	EXPECT_EQ( diffResult.type, CarbonResources::ResultType::SUCCESS );


	// Test the result
	EXPECT_EQ( additions.size(), 2 );
}

TEST_F( ResourcesLibraryTest, DiffResourceGroupsWithTwoSubtractions )
{
	// Load base group
	CarbonResources::ResourceGroup baseResourceGroup;

	CarbonResources::ResourceGroupImportFromFileParams importFromFileParams;

	importFromFileParams.filename = GetTestFileFileAbsolutePath( "DiffGroups/resFileIndex.txt" );

	CarbonResources::Result importFromFileResult = baseResourceGroup.ImportFromFile( importFromFileParams );

	EXPECT_EQ( importFromFileResult.type, CarbonResources::ResultType::SUCCESS );


	// Load group with Subtractions
	CarbonResources::ResourceGroup resourceGroupWithSubtractions;

	CarbonResources::ResourceGroupImportFromFileParams importFromFileParamsWithSubtractions;

	importFromFileParamsWithSubtractions.filename = GetTestFileFileAbsolutePath( "DiffGroups/resFileIndexWithSubtractions.txt" );

	CarbonResources::Result importFromFileResultSubtractions = resourceGroupWithSubtractions.ImportFromFile( importFromFileParamsWithSubtractions );

	EXPECT_EQ( importFromFileResultSubtractions.type, CarbonResources::ResultType::SUCCESS );


	// Perform diff
	CarbonResources::ResourceGroupDiffAgainstGroupParams diffAgainstGroupParams;

	diffAgainstGroupParams.resourceGroupToDiffAgainst = &baseResourceGroup;

	std::vector<std::filesystem::path> additions;

	std::vector<std::filesystem::path> subtractions;

	diffAgainstGroupParams.additions = &additions;

	diffAgainstGroupParams.subtractions = &subtractions;

	CarbonResources::Result diffResult = resourceGroupWithSubtractions.DiffAgainstGroup( diffAgainstGroupParams );

	EXPECT_EQ( diffResult.type, CarbonResources::ResultType::SUCCESS );


	// Test the result
	EXPECT_EQ( subtractions.size(), 2 );
}

TEST_F( ResourcesLibraryTest, MergeResourceGroupsAdditive )
{
	CarbonResources::ResourceGroup resourceGroup;

	CarbonResources::ResourceGroupImportFromFileParams importFromFileParams;

	importFromFileParams.filename = GetTestFileFileAbsolutePath( "MergeGroups/YamlAdditive/BaseResourceGroup.yaml" );

	CarbonResources::Result importFromFileResult = resourceGroup.ImportFromFile( importFromFileParams );

	EXPECT_EQ( importFromFileResult.type, CarbonResources::ResultType::SUCCESS );

	// Load merge group
	CarbonResources::ResourceGroup resourceGroupToMerge;

	CarbonResources::ResourceGroupImportFromFileParams mergeImportFromFileParams;

	mergeImportFromFileParams.filename = GetTestFileFileAbsolutePath( "MergeGroups/YamlAdditive/MergeResourceGroup.yaml" );

	CarbonResources::Result mergeImportFromFileResult = resourceGroupToMerge.ImportFromFile( mergeImportFromFileParams );

	EXPECT_EQ( mergeImportFromFileResult.type, CarbonResources::ResultType::SUCCESS );

	// Merge resource groups
	CarbonResources::ResourceGroup mergedResourceGroup;

	CarbonResources::ResourceGroupMergeParams mergeParams;

	mergeParams.resourceGroupToMerge = &resourceGroupToMerge;

	mergeParams.mergedResourceGroup = &mergedResourceGroup;

	CarbonResources::Result mergeResult = resourceGroup.Merge( mergeParams );

	EXPECT_EQ( mergeResult.type, CarbonResources::ResultType::SUCCESS );

	// Export the merged result
	CarbonResources::ResourceGroupExportToFileParams exportParams;

	exportParams.filename = "Merge/mergedResourceGroup.yaml";

	CarbonResources::Result exportResult = mergedResourceGroup.ExportToFile( exportParams );

	EXPECT_EQ( exportResult.type, CarbonResources::ResultType::SUCCESS );

	// Check output matches expected
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "MergeGroups/YamlAdditive/ExpectedMergedResourceGroup.yaml" );

	EXPECT_TRUE( FilesMatch( goldFile, exportParams.filename ) );
}

TEST_F( ResourcesLibraryTest, MergeResourceGroupsIdentical )
{
	CarbonResources::ResourceGroup resourceGroup;

	CarbonResources::ResourceGroupImportFromFileParams importFromFileParams;

	importFromFileParams.filename = GetTestFileFileAbsolutePath( "MergeGroups/YamlIdentical/BaseResourceGroup.yaml" );

	CarbonResources::Result importFromFileResult = resourceGroup.ImportFromFile( importFromFileParams );

	EXPECT_EQ( importFromFileResult.type, CarbonResources::ResultType::SUCCESS );

	// Load merge group
	CarbonResources::ResourceGroup resourceGroupToMerge;

	CarbonResources::ResourceGroupImportFromFileParams mergeImportFromFileParams;

	mergeImportFromFileParams.filename = GetTestFileFileAbsolutePath( "MergeGroups/YamlIdentical/MergeResourceGroup.yaml" );

	CarbonResources::Result mergeImportFromFileResult = resourceGroupToMerge.ImportFromFile( mergeImportFromFileParams );

	EXPECT_EQ( mergeImportFromFileResult.type, CarbonResources::ResultType::SUCCESS );

	// Merge resource groups
	CarbonResources::ResourceGroup mergedResourceGroup;

	CarbonResources::ResourceGroupMergeParams mergeParams;

	mergeParams.resourceGroupToMerge = &resourceGroupToMerge;

	mergeParams.mergedResourceGroup = &mergedResourceGroup;

	CarbonResources::Result mergeResult = resourceGroup.Merge( mergeParams );

	EXPECT_EQ( mergeResult.type, CarbonResources::ResultType::SUCCESS );

	// Export the merged result
	CarbonResources::ResourceGroupExportToFileParams exportParams;

	exportParams.filename = "Merge/mergedResourceGroup.yaml";

	CarbonResources::Result exportResult = mergedResourceGroup.ExportToFile( exportParams );

	EXPECT_EQ( exportResult.type, CarbonResources::ResultType::SUCCESS );

	// Check output matches expected
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "MergeGroups/YamlIdentical/ExpectedMergedResourceGroup.yaml" );

	EXPECT_TRUE( FilesMatch( goldFile, exportParams.filename ) );
}

TEST_F( ResourcesLibraryTest, MergeResourceGroupsWithIntersect_V_0_0_0 )
{
	CarbonResources::ResourceGroup resourceGroup;

	CarbonResources::ResourceGroupImportFromFileParams importFromFileParams;

	importFromFileParams.filename = GetTestFileFileAbsolutePath( "MergeGroups/CSVWithIntersect/BaseResourceGroup.txt" );

	CarbonResources::Result importFromFileResult = resourceGroup.ImportFromFile( importFromFileParams );

	EXPECT_EQ( importFromFileResult.type, CarbonResources::ResultType::SUCCESS );

	// Load merge group
	CarbonResources::ResourceGroup resourceGroupToMerge;

	CarbonResources::ResourceGroupImportFromFileParams mergeImportFromFileParams;

	mergeImportFromFileParams.filename = GetTestFileFileAbsolutePath( "MergeGroups/CSVWithIntersect/MergeResourceGroup.txt" );

	CarbonResources::Result mergeImportFromFileResult = resourceGroupToMerge.ImportFromFile( mergeImportFromFileParams );

	EXPECT_EQ( mergeImportFromFileResult.type, CarbonResources::ResultType::SUCCESS );

	// Merge resource groups
	CarbonResources::ResourceGroup mergedResourceGroup;

	CarbonResources::ResourceGroupMergeParams mergeParams;

	mergeParams.resourceGroupToMerge = &resourceGroupToMerge;

	mergeParams.mergedResourceGroup = &mergedResourceGroup;

	CarbonResources::Result mergeResult = resourceGroup.Merge( mergeParams );

	EXPECT_EQ( mergeResult.type, CarbonResources::ResultType::SUCCESS );

	// Export the merged result
	CarbonResources::ResourceGroupExportToFileParams exportParams;

	exportParams.filename = "Merge/mergedResourceGroup.txt";

	exportParams.outputDocumentVersion = CarbonResources::Version{ 0, 0, 0 };

	CarbonResources::Result exportResult = mergedResourceGroup.ExportToFile( exportParams );

	EXPECT_EQ( exportResult.type, CarbonResources::ResultType::SUCCESS );

	// Check output matches expected
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "MergeGroups/CSVWithIntersect/ExpectedMergedResourceGroup.txt" );

	EXPECT_TRUE( FilesMatch( goldFile, exportParams.filename ) );
}

TEST_F( ResourcesLibraryTest, MergeResourceGroupsAdditive_V_0_0_0 )
{
	CarbonResources::ResourceGroup resourceGroup;

	CarbonResources::ResourceGroupImportFromFileParams importFromFileParams;

	importFromFileParams.filename = GetTestFileFileAbsolutePath( "MergeGroups/CSVAdditive/BaseResourceGroup.txt" );

	CarbonResources::Result importFromFileResult = resourceGroup.ImportFromFile( importFromFileParams );

	EXPECT_EQ( importFromFileResult.type, CarbonResources::ResultType::SUCCESS );

	// Load merge group
	CarbonResources::ResourceGroup resourceGroupToMerge;

	CarbonResources::ResourceGroupImportFromFileParams mergeImportFromFileParams;

	mergeImportFromFileParams.filename = GetTestFileFileAbsolutePath( "MergeGroups/CSVAdditive/MergeResourceGroup.txt" );

	CarbonResources::Result mergeImportFromFileResult = resourceGroupToMerge.ImportFromFile( mergeImportFromFileParams );

	EXPECT_EQ( mergeImportFromFileResult.type, CarbonResources::ResultType::SUCCESS );

	// Merge resource groups
	CarbonResources::ResourceGroup mergedResourceGroup;

	CarbonResources::ResourceGroupMergeParams mergeParams;

	mergeParams.resourceGroupToMerge = &resourceGroupToMerge;

	mergeParams.mergedResourceGroup = &mergedResourceGroup;

	CarbonResources::Result mergeResult = resourceGroup.Merge( mergeParams );

	EXPECT_EQ( mergeResult.type, CarbonResources::ResultType::SUCCESS );

	// Export the merged result
	CarbonResources::ResourceGroupExportToFileParams exportParams;

	exportParams.filename = "Merge/mergedResourceGroup.txt";

	exportParams.outputDocumentVersion = CarbonResources::Version{ 0, 0, 0 };

	CarbonResources::Result exportResult = mergedResourceGroup.ExportToFile( exportParams );

	EXPECT_EQ( exportResult.type, CarbonResources::ResultType::SUCCESS );

	// Check output matches expected
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "MergeGroups/CSVAdditive/ExpectedMergedResourceGroup.txt" );

	EXPECT_TRUE( FilesMatch( goldFile, exportParams.filename ) );
}

TEST_F( ResourcesLibraryTest, RemoveResource )
{
	CarbonResources::ResourceGroup resourceGroup;

	// Import ResourceGroup
	CarbonResources::ResourceGroupImportFromFileParams importParams;

	importParams.filename = GetTestFileFileAbsolutePath( "RemoveResource/BaseResourceGroup.yaml" );

	CarbonResources::Result importResult = resourceGroup.ImportFromFile( importParams );

	EXPECT_EQ( importResult.type, CarbonResources::ResultType::SUCCESS );

	// Remove resource B.txt
	CarbonResources::ResourceGroupRemoveResourcesParams removeParams;

	std::vector<std::filesystem::path> resourcesToRemove;

	resourcesToRemove.push_back( "B.txt" );

	removeParams.resourcesToRemove = &resourcesToRemove;

	removeParams.errorIfResourceNotFound = true;

	CarbonResources::Result removeResult = resourceGroup.RemoveResources( removeParams );

	EXPECT_EQ( removeResult.type, CarbonResources::ResultType::SUCCESS );

	// Export the ResourceGroup
	CarbonResources::ResourceGroupExportToFileParams exportParams;

	exportParams.filename = "RemoveResource/ResourceGroup.yaml";

	CarbonResources::Result exportResult = resourceGroup.ExportToFile( exportParams );

	EXPECT_EQ( exportResult.type, CarbonResources::ResultType::SUCCESS );


	// Check output matches expected
	std::filesystem::path goldFile = GetTestFileFileAbsolutePath( "RemoveResource/ResourceGroupAfterRemove.yaml" );

	EXPECT_TRUE( FilesMatch( goldFile, exportParams.filename ) );
}