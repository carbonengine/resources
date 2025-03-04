#include <ResourceGroup.h>
#include <BundleResourceGroup.h>
#include <PatchResourceGroup.h>

#include "CarbonResourcesTestFixture.h"

#include <gtest/gtest.h>

#include <BinaryResourceGroup.h>

#include <BinaryResource.h>

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

	exportParams.outputFilename = "binaryFileIndex.yaml";

	binaryResourceGroup.ExportToFile( exportParams );

    std::string goldStandardFilename = GetTestFileFileAbsolutePath( "/Indicies/binaryFileIndex_v0_1_0.yaml" );

	EXPECT_TRUE( FilesMatch( exportParams.outputFilename, exportParams.outputFilename ) );
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

	exportParams.outputFilename = "binaryFileIndex.yaml";

	binaryResourceGroup.ExportToFile( exportParams );

    std::string inputFilename = importParams.dataParams.resourceSourceSettings.developmentLocalBasePath + inputResName.substr( prefix.size() );

	EXPECT_TRUE( FilesMatch( inputFilename, exportParams.outputFilename ) );
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

	exportParams.outputFilename = "resFileIndex.yaml";

	resourceGroup.ExportToFile( exportParams );

    std::string goldStandardFilename = GetTestFileFileAbsolutePath( "/Indicies/resFileIndex_v0_1_0.yaml" );

	EXPECT_TRUE( FilesMatch( exportParams.outputFilename, goldStandardFilename ) );
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

	exportParams.outputFilename = "resFileIndex.yaml";

	resourceGroup.ExportToFile( exportParams );

    std::string inputFilename = importParams.dataParams.resourceSourceSettings.developmentLocalBasePath + inputResName.substr( prefix.size() );

    EXPECT_TRUE( FilesMatch( inputFilename, exportParams.outputFilename ) );
}

TEST_F( CarbonResourcesLibraryTest, CreateBundle )
{
    // Not yet implemented
	EXPECT_TRUE( false );
}
TEST_F( CarbonResourcesLibraryTest, CreatePatch )
{
	/*
    // Import a resource group to create patch for
    // This whole process is WIP

    // Previous
	CarbonResources::ResourceGroup resourceGroupPrevious("res:/resfileindexShort.txt");

	CarbonResources::ResourceGroupImportFromFileParams importParamsPrevious;

    importParamsPrevious.dataParams.resourceSourceSettings.developmentLocalBasePath = GetTestFileFileAbsolutePath( "/Indicies/" );

	resourceGroupPrevious.ImportFromFile( importParamsPrevious );



    // Latest
	CarbonResources::ResourceGroup resourceGroupLatest("res:/resfileindexShort.txt");

	CarbonResources::ResourceGroupImportFromFileParams importParamsLatest;

    importParamsLatest.dataParams.resourceSourceSettings.developmentLocalBasePath = GetTestFileFileAbsolutePath( "/Indicies/" );

	resourceGroupLatest.ImportFromFile( importParamsLatest );


    // Create Patches and Patch Resource Group
    CarbonResources::PatchCreateParams patchCreateParams;

    patchCreateParams.latestResourceGroup = &resourceGroupLatest;

    patchCreateParams.outputDirectoryPath = "/CDN_OUT/"; //TODO placeholder, not worked out file structures for this kind of thing yet

    patchCreateParams.previousResourceGroup = &resourceGroupPrevious;

    CarbonResources::PatchResourceGroup patchResourceGroup("res:/patch.yaml");

    patchCreateParams.patchResourceGroup = &patchResourceGroup;

    

    //patchCreateParams.basePath = "/CDN_IN/"; //TODO placeholder, not worked out file structures for this kind of thing yet

    resourceGroupLatest.CreatePatch( patchCreateParams );

    // Run checks on the output TODO
	EXPECT_TRUE( false );
    */

    
}