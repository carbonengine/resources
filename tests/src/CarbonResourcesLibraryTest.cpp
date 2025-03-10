#include <ResourceGroup.h>
#include <BundleResourceGroup.h>
#include <PatchResourceGroup.h>

#include "CarbonResourcesTestFixture.h"

#include <gtest/gtest.h>

#include <BinaryResourceGroup.h>


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

	CarbonResources::BinaryResourceGroup binaryResourceGroup;

	CarbonResources::ResourceGroupImportFromFileParams importParams;

    importParams.filename = GetTestFileFileAbsolutePath( "Indicies/binaryFileIndex_v0_0_0.txt" );

	EXPECT_EQ(binaryResourceGroup.ImportFromFile( importParams ),CarbonResources::Result::SUCCESS);

	CarbonResources::ResourceGroupExportToFileParams exportParams;

    exportParams.filename = "resPath/BinaryResourceGroup_v0_1_0.yaml";

	EXPECT_EQ(binaryResourceGroup.ExportToFile( exportParams ),CarbonResources::Result::SUCCESS);

    std::filesystem::path goldStandardFilename = GetTestFileFileAbsolutePath( "Indicies/BinaryResourceGroup_v0_1_0.yaml" );

    //TODO reinstate
	EXPECT_TRUE( FilesMatch( exportParams.filename, goldStandardFilename ) );
}

// Import a BinaryResourceGroup v0.1.0 and export it again checking input == output
TEST_F( CarbonResourcesLibraryTest, BinaryGroupImportExport_V_0_1_0 )
{
	CarbonResources::BinaryResourceGroup binaryResourceGroup;

	CarbonResources::ResourceGroupImportFromFileParams importParams;

    importParams.filename = GetTestFileFileAbsolutePath( "Indicies/BinaryResourceGroup_v0_1_0.yaml" );

	EXPECT_EQ(binaryResourceGroup.ImportFromFile( importParams ),CarbonResources::Result::SUCCESS);

	CarbonResources::ResourceGroupExportToFileParams exportParams;

    exportParams.filename = "resPath/BinaryResourceGroup_v0_1_0.yaml";

	EXPECT_EQ(binaryResourceGroup.ExportToFile( exportParams ),CarbonResources::Result::SUCCESS);

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

	EXPECT_EQ(resourceGroup.ImportFromFile( importParams ),CarbonResources::Result::SUCCESS);

	CarbonResources::ResourceGroupExportToFileParams exportParams;

    exportParams.filename = "resPath/ResourceGroup_v0_1_0.yaml";

	EXPECT_EQ(resourceGroup.ExportToFile( exportParams ),CarbonResources::Result::SUCCESS);

    std::filesystem::path goldStandardFilename = GetTestFileFileAbsolutePath( "Indicies/ResourceGroup_v0_1_0.yaml" );

	EXPECT_TRUE( FilesMatch( exportParams.filename, goldStandardFilename ) );
}

// Import a ResourceGroup with missing parameters that are expected for the version provided
// This should fail gracefully and give appropriate feedback to user
TEST_F( CarbonResourcesLibraryTest, ResourceGroupHandleImportMissingParametersForVersion )
{
	// Not yet implemented
	EXPECT_TRUE( false );
}

// Import a mal formed ResourceGroup
// This should fail gracefully and give appropriate feedback to user
TEST_F( CarbonResourcesLibraryTest, ResourceGroupHandleImportIncorrectlyFormattedInput )
{
	// Not yet implemented
	EXPECT_TRUE( false );
}

// Import a ResourceGroup with version greater than current document minor version specified in enums.h
// This should open ignoring anything extra added in the future version
// The version of the imported ResourceGroup should be set at the max supported version in enums.h
TEST_F( CarbonResourcesLibraryTest, ResourceGroupImportNewerMinorVersion )
{
	// Not yet implemented
	EXPECT_TRUE( false );
}

// Import a ResourceGroup with version greater than current document major version specified in enums.h
// This should gracefully fail to open
TEST_F( CarbonResourcesLibraryTest, ResourceGroupImportNewerMajorVersion )
{
	// Not yet implemented
	EXPECT_TRUE( false );
}

// Import a ResourceGroup that doesn't exist
// This should gracefully fail
TEST_F( CarbonResourcesLibraryTest, ResourceGroupImportNonExistantFile )
{
	// Not yet implemented
	EXPECT_TRUE( false );
}

// Import a ResourceGroup v0.1.0 and export it again checking input == output
TEST_F( CarbonResourcesLibraryTest, ResourceGroupImportExport_V_0_1_0 )
{

	CarbonResources::ResourceGroup resourceGroup;

	CarbonResources::ResourceGroupImportFromFileParams importParams;

    importParams.filename = GetTestFileFileAbsolutePath( "Indicies/ResourceGroup_v0_1_0.yaml" );

	EXPECT_EQ(resourceGroup.ImportFromFile( importParams ),CarbonResources::Result::SUCCESS);

	CarbonResources::ResourceGroupExportToFileParams exportParams;

    exportParams.filename = "resPath/ResourceGroup_v0_1_0.yaml";

	EXPECT_EQ(resourceGroup.ExportToFile( exportParams ),CarbonResources::Result::SUCCESS);

	EXPECT_TRUE( FilesMatch( importParams.filename, exportParams.filename ) );
}

TEST_F( CarbonResourcesLibraryTest, UnpackBundle )
{
	// Not yet implemented
	EXPECT_TRUE( false );
}
TEST_F( CarbonResourcesLibraryTest, CreateBundle )
{
    // Not yet implemented
	EXPECT_TRUE( false );
}

TEST_F( CarbonResourcesLibraryTest, ApplyPatch )
{
	// Load the patch file
	CarbonResources::PatchResourceGroup patchResourceGroup;

    CarbonResources::ResourceGroupImportFromFileParams importParamsPrevious;

    importParamsPrevious.filename = GetTestFileFileAbsolutePath( "Patch/PatchResourceGroup_previousBuild_latestBuild.yaml" );

	EXPECT_EQ( patchResourceGroup.ImportFromFile( importParamsPrevious ), CarbonResources::Result::SUCCESS );


    // Apply the patch
	CarbonResources::PatchApplyParams patchApplyParams;

    patchApplyParams.patchBinarySourceSettings.productionLocalBasePath = GetTestFileFileAbsolutePath( "Patch/LocalRemote/" );

    patchApplyParams.resourcesToPatchSourceSettings.productionLocalBasePath = GetTestFileFileAbsolutePath( "Patch/Local/" );

    patchApplyParams.resourcesToPatchDestinationSettings.productionLocalBasePath = "ApplyPatchOut";

    EXPECT_EQ(patchResourceGroup.Apply( patchApplyParams ),CarbonResources::Result::SUCCESS);

    // TODO test the output of the applied patches

}

TEST_F( CarbonResourcesLibraryTest, CreatePatch )
{
    // Previous ResourceGroup
	CarbonResources::ResourceGroup resourceGroupPrevious;

	CarbonResources::ResourceGroupImportFromFileParams importParamsPrevious;

    importParamsPrevious.filename = GetTestFileFileAbsolutePath( "Patch/resfileindexShort_build_previous.txt" );

	EXPECT_EQ(resourceGroupPrevious.ImportFromFile( importParamsPrevious ),CarbonResources::Result::SUCCESS);


    // Latest ResourceGroup
	CarbonResources::ResourceGroup resourceGroupLatest;

	CarbonResources::ResourceGroupImportFromFileParams importParamsLatest;

    importParamsLatest.filename = GetTestFileFileAbsolutePath( "Patch/resfileindexShort_build_next.txt" );

	EXPECT_EQ(resourceGroupLatest.ImportFromFile( importParamsLatest ),CarbonResources::Result::SUCCESS);



    // Create a patch from the subtraction index
	CarbonResources::PatchCreateParams patchCreateParams;

    patchCreateParams.resourceGroupRelativePath = "ResourceGroup_previousBuild_latestBuild.yaml";

    patchCreateParams.resourceGroupPatchRelativePath = "PatchResourceGroup_previousBuild_latestBuild.yaml";

    patchCreateParams.resourceSourceSettingsFrom.productionLocalBasePath = GetTestFileFileAbsolutePath( "resourcesLocal" );

    patchCreateParams.resourceSourceSettingsTo.productionLocalBasePath = GetTestFileFileAbsolutePath( "resourcesRemote" );

    patchCreateParams.resourcePatchBinaryDestinationSettings.productionLocalBasePath = "SharedCache";

    patchCreateParams.resourcePatchResourceGroupDestinationSettings.developmentLocalBasePath = "resPath";

    patchCreateParams.previousResourceGroup = &resourceGroupPrevious;
    
	EXPECT_EQ(resourceGroupLatest.CreatePatch( patchCreateParams ),CarbonResources::Result::SUCCESS);


    // TODO run tests on patch create
    
}