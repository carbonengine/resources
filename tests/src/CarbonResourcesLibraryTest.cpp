#include <ResourceGroup.h>
#include <BundleResourceGroup.h>
#include <PatchResourceGroup.h>

#include "CarbonResourcesTestFixture.h"

#include <gtest/gtest.h>

#include <BinaryResourceGroup.h>


// TODO I think it would be good if the output files of the tests were put in folders which match the test name
// The DLL handles the case where the dll is a newer version that what was used to compile against, find a way to test this

struct CarbonResourcesLibraryTest : public CarbonResourcesTestFixture{};

// Import ResourceGroup V0.0.0
// Export should output as V0.1.0
// This is the only instance that exporting a file of a lower version results in a version bump
TEST_F( CarbonResourcesLibraryTest, BinaryGroupImportExport_V_0_0_0_To_V_0_1_0 )
{
	CarbonResources::BinaryResourceGroup binaryResourceGroup("res:/binaryFileIndex_v0_0_0.txt");

	CarbonResources::ResourceGroupImportFromFileParams importParams;

    importParams.dataParams.resourceSourceSettings.developmentLocalBasePath = GetTestFileFileAbsolutePath( "/Indicies/" );

	binaryResourceGroup.ImportFromFile( importParams );

	CarbonResources::ResourceGroupExportToFileParams exportParams;

    exportParams.resourceDetinationSettings.productionLocalBasePath = "SharedCache";

	binaryResourceGroup.ExportToFile( exportParams );

    std::string goldStandardFilename = GetTestFileFileAbsolutePath( "/Indicies/binaryFileIndex_v0_1_0.yaml" );

    //TODO reinstate
	//EXPECT_TRUE( FilesMatch( exportParams.outputFilename, exportParams.outputFilename ) );
}

// Import a BinaryResourceGroup v0.1.0 and export it again checking input == output
TEST_F( CarbonResourcesLibraryTest, BinaryGroupImportExport_V_0_1_0 )
{
	std::string prefix = "res:/";

    std::string inputResName = prefix + "binaryFileIndex_v0_1_0.yaml";

	CarbonResources::BinaryResourceGroup binaryResourceGroup( inputResName );

	CarbonResources::ResourceGroupImportFromFileParams importParams;

    importParams.dataParams.resourceSourceSettings.developmentLocalBasePath = GetTestFileFileAbsolutePath( "/Indicies/" );

	binaryResourceGroup.ImportFromFile( importParams );

	CarbonResources::ResourceGroupExportToFileParams exportParams;

    exportParams.resourceDetinationSettings.productionLocalBasePath = "SharedCache";

	binaryResourceGroup.ExportToFile( exportParams );

    std::string inputFilename = importParams.dataParams.resourceSourceSettings.developmentLocalBasePath + inputResName.substr( prefix.size() );

    //TODO reinstate
	//EXPECT_TRUE( FilesMatch( inputFilename, exportParams.outputFilename ) );
}

// Import ResourceGroup V0.0.0 
// Export should output as V0.1.0
// This is the only instance that exporting a file of a lower version results in a version bump
TEST_F( CarbonResourcesLibraryTest, ResourceGroupImportExport_V_0_0_0_To_V_0_1_0 )
{
	CarbonResources::ResourceGroup resourceGroup("res:/resFileIndex_v0_0_0.txt");

	CarbonResources::ResourceGroupImportFromFileParams importParams;

    importParams.dataParams.resourceSourceSettings.developmentLocalBasePath = GetTestFileFileAbsolutePath( "/Indicies/" );

	resourceGroup.ImportFromFile( importParams );

	CarbonResources::ResourceGroupExportToFileParams exportParams;

    exportParams.resourceDetinationSettings.productionLocalBasePath = "SharedCache";

	resourceGroup.ExportToFile( exportParams );

    std::string goldStandardFilename = GetTestFileFileAbsolutePath( "/Indicies/resFileIndex_v0_1_0.yaml" );

    //TODO reinstate
	//EXPECT_TRUE( FilesMatch( exportParams.outputFilename, goldStandardFilename ) );
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
	std::string prefix = "res:/";

	std::string inputResName = prefix + "resFileIndex_v0_1_0.yaml";

	CarbonResources::ResourceGroup resourceGroup( inputResName );

	CarbonResources::ResourceGroupImportFromFileParams importParams;

    importParams.dataParams.resourceSourceSettings.developmentLocalBasePath = GetTestFileFileAbsolutePath( "/Indicies/" );

	resourceGroup.ImportFromFile( importParams );

	CarbonResources::ResourceGroupExportToFileParams exportParams;

    exportParams.resourceDetinationSettings.productionLocalBasePath = "SharedCache";

	resourceGroup.ExportToFile( exportParams ); //TODO test the return values from these calls

    std::string inputFilename = importParams.dataParams.resourceSourceSettings.developmentLocalBasePath + inputResName.substr( prefix.size() );

    //TODO reinstate
    //EXPECT_TRUE( FilesMatch( inputFilename, exportParams.outputFilename ) );
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
	// Not yet implemented
	EXPECT_TRUE( false );
}
TEST_F( CarbonResourcesLibraryTest, CreatePatch )
{
	GTEST_SKIP() << "Patch creation still being implemented";
    // This whole process is WIP

    // Previous ResourceGroup
	CarbonResources::ResourceGroup resourceGroupPrevious("res:/resfileindexShort_build1.txt");

	CarbonResources::ResourceGroupImportFromFileParams importParamsPrevious;

    importParamsPrevious.dataParams.resourceSourceSettings.developmentLocalBasePath = GetTestFileFileAbsolutePath( "/Indicies/" );

	resourceGroupPrevious.ImportFromFile( importParamsPrevious );


    // Latest ResourceGroup
	CarbonResources::ResourceGroup resourceGroupLatest("res:/resfileindexShort_build2.txt");

	CarbonResources::ResourceGroupImportFromFileParams importParamsLatest;

    importParamsLatest.dataParams.resourceSourceSettings.developmentLocalBasePath = GetTestFileFileAbsolutePath( "/Indicies/" );    //TODO deal with file paths better using filesystem things

	resourceGroupLatest.ImportFromFile( importParamsLatest );


    // Create a subtraction between previous and latest ResourceGroups
	CarbonResources::ResourceGroup resourceGroupSubtraction( "res:/resfileindex_previousBuild_latestBuild.txt" );

    CarbonResources::ResourceGroupSubtractionParams resourceGroupSubtractionParams;

    resourceGroupSubtractionParams.subtractResourceGroup = &resourceGroupPrevious;

    resourceGroupSubtractionParams.result = &resourceGroupSubtraction;

    resourceGroupLatest.Subtraction( resourceGroupSubtractionParams );


    // Save the subtraction index
	CarbonResources::ResourceGroupExportToFileParams exportParams;

    exportParams.resourceDetinationSettings.productionLocalBasePath = "SharedCache";

	resourceGroupSubtraction.ExportToFile( exportParams );


    // Create a patch from the subtraction index
	CarbonResources::PatchCreateParams patchCreateParams;

    patchCreateParams.resourceSourceSettingsFrom.productionLocalBasePath = GetTestFileFileAbsolutePath( "/resourcesLocal" );

    patchCreateParams.resourceSourceSettingsTo.productionLocalBasePath = GetTestFileFileAbsolutePath( "/resourcesRemote" );

    patchCreateParams.resourceDestinationSettings.productionLocalBasePath = "SharedCache";

	resourceGroupSubtraction.CreatePatch( patchCreateParams );


    // Save patch resource to respath
	//CarbonResources::PatchResourceGroup patchResourceGroup( patchPath.ToString(), this );

	//CarbonResources::ResourceGroupExportToFileParams patchResourceGroupExportToFileParams;

	//patchResourceGroupExportToFileParams.resourceDetinationSettings.developmentLocalBasePath = "resPath";

	//patchCreateParams.patchResourceGroup->ExportToFile( patchResourceGroupExportToFileParams );

    // Who owns that memory then? This is not right TODO


    // TODO run tests on patch create
    
}