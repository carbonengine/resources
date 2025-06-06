#include "ApplyPatchCliOperation.h"

#include <PatchResourceGroup.h>

#include <iostream>
#include <argparse/argparse.hpp>

ApplyPatchCliOperation::ApplyPatchCliOperation() :
	CliOperation("apply-patch", "Applies a patch supplied via a Patch Resource Group to a directory [Only available in extended feature development build]"),
	m_patchResourceGroupPathArgumentId("patch-resource-group-path"),
	m_patchBinariesSourceBasePathsArgumentId("--patch-binaries-base-path"),
	m_patchBinariesSourceTypeArgumentId("--patch-binaries-source-type"),
	m_resourcesToPatchSourceBasePathsArgumentId("--resources-to-patch-base-path"),
	m_resourcesToPatchSourceTypeArgumentId("--resources-to-patch-source-type"),
	m_nextResourcesBasePathsArgumentId("--next-resources-base-path"),
	m_nextResourcesSourceTypeArgumentId("--next-resources-source-type"),
	m_resourcesToPatchDestinationPathArgumentId("--output-base-path"),
	m_resourcesToPatchDestinationTypeArgumentId("--output-destination-type")
{
	AddRequiredPositionalArgument( m_patchResourceGroupPathArgumentId, "The path to the PatchResourceGroup.yaml file." );

	CarbonResources::PatchApplyParams defaultParams;

	AddArgument( m_patchBinariesSourceBasePathsArgumentId, "The paths to the folders containing the patch binaries.", true, true, PathsToString( defaultParams.patchBinarySourceSettings.basePaths ) );

    AddArgument( m_resourcesToPatchSourceBasePathsArgumentId, "The paths to the folders containing resources to patch.", true, true, PathsToString( defaultParams.resourcesToPatchSourceSettings.basePaths ) );

    AddArgument( m_nextResourcesBasePathsArgumentId, "The path to resources after the patch. This is used to get fully added files which are not included in the generated patch files.", true, true, PathListToString( defaultParams.nextBuildResourcesSourceSettings.basePaths ) );

	AddArgument( m_patchBinariesSourceTypeArgumentId, "The type of repository the patch binaries are sourced from.", false, false, SourceTypeToString( defaultParams.patchBinarySourceSettings.sourceType ), ResourceSourceTypeChoicesAsString() );

	AddArgument( m_resourcesToPatchSourceTypeArgumentId, "The type of repository the resources to patch are sourced from.", false, false, SourceTypeToString( defaultParams.resourcesToPatchSourceSettings.sourceType ), ResourceSourceTypeChoicesAsString() );

	AddArgument( m_nextResourcesSourceTypeArgumentId, "The type of repository the resources after the patch are sourced from.", false, false, SourceTypeToString( defaultParams.nextBuildResourcesSourceSettings.sourceType ), ResourceSourceTypeChoicesAsString() );

	AddArgument( m_resourcesToPatchDestinationPathArgumentId, "The path in which to place the patched version of the files.", false, false, "ApplyPatchOut" );

	AddArgument( m_resourcesToPatchDestinationTypeArgumentId, "The type of repository in which to place the patched version of the files.", false, false, DestinationTypeToString( defaultParams.resourcesToPatchDestinationSettings.destinationType ), ResourceDestinationTypeChoicesAsString() );
}

bool ApplyPatchCliOperation::Execute( std::string& returnErrorMessage ) const
{
	CarbonResources::ResourceGroupImportFromFileParams importParamsPrevious;

	std::optional<std::string> filename = m_argumentParser->present<std::string>( m_patchResourceGroupPathArgumentId );
	if( !filename.has_value() )
	{
		returnErrorMessage = "Failed to parse patch resource group path";

		return false;
	}
    importParamsPrevious.filename = filename.value();

	CarbonResources::PatchApplyParams patchApplyParams;

	std::vector<std::filesystem::path> newBuildResourceSettingBasePaths;
	std::optional<std::string> nextResources = m_argumentParser->present<std::string>( m_nextResourcesBasePathsArgumentId );
	if( !nextResources.has_value() )
	{
		returnErrorMessage = "Failed to parse next resource base path";

		return false;
	}
	newBuildResourceSettingBasePaths.push_back( nextResources.value() );
	std::string nextResourcesType = m_argumentParser->get( m_nextResourcesSourceTypeArgumentId );
	if( !StringToResourceSourceType( nextResourcesType, patchApplyParams.nextBuildResourcesSourceSettings.sourceType ) )
	{
		returnErrorMessage = "Invalid next build source type";

		return false;
	}

	std::vector<std::filesystem::path> patchBinarySourceSettingsBasePaths;
	auto patchBinaries = m_argumentParser->present<std::vector<std::string>>( m_patchBinariesSourceBasePathsArgumentId );
	if( !patchBinaries.has_value() )
	{
		returnErrorMessage = "Failed to parse patch binaries base path";

		return false;
	}
	for( auto path : patchBinaries.value() )
	{
		patchBinarySourceSettingsBasePaths.push_back( path );
	}
	patchApplyParams.patchBinarySourceSettings.basePaths = patchBinarySourceSettingsBasePaths;
	patchApplyParams.nextBuildResourcesSourceSettings.basePaths = newBuildResourceSettingBasePaths;
	std::string patchBinariesType = m_argumentParser->get( m_patchBinariesSourceTypeArgumentId );
	if( !StringToResourceSourceType( patchBinariesType, patchApplyParams.patchBinarySourceSettings.sourceType ) )
	{
		returnErrorMessage = "Invalid patch binary source type";

		return false;
	}

	std::vector<std::filesystem::path> resourcesToPatchSourceSettingsBasePaths;
	auto resourcesToPatch = m_argumentParser->present<std::vector<std::string>>( m_resourcesToPatchSourceBasePathsArgumentId );
	if( !resourcesToPatch.has_value() )
	{
		returnErrorMessage = "Failed to parse patch source base path";

		return false;
	}
	for( auto path : resourcesToPatch.value() )
	{
		resourcesToPatchSourceSettingsBasePaths.push_back( path );
	}
    patchApplyParams.resourcesToPatchSourceSettings.basePaths = resourcesToPatchSourceSettingsBasePaths;
	std::string resourcesToPatchType = m_argumentParser->get( m_resourcesToPatchSourceTypeArgumentId );
	if( !StringToResourceSourceType( resourcesToPatchType, patchApplyParams.resourcesToPatchSourceSettings.sourceType ) )
	{
		returnErrorMessage = "Invalid resources to patch source type";

		return false;
	}

    patchApplyParams.resourcesToPatchDestinationSettings.basePath = m_argumentParser->get( m_resourcesToPatchDestinationPathArgumentId );

	std::string outputType = m_argumentParser->get( m_resourcesToPatchDestinationTypeArgumentId );
	if( !StringToResourceDestinationType( outputType , patchApplyParams.resourcesToPatchDestinationSettings.destinationType ) )
	{
		returnErrorMessage = "Invalid resources to patch destination type";

		return false;
	}

    patchApplyParams.temporaryFilePath = "tempFile.resource";

	PrintStartBanner( importParamsPrevious, patchApplyParams );

	return ApplyPatch( importParamsPrevious, patchApplyParams );
}



void ApplyPatchCliOperation::PrintStartBanner(const CarbonResources::ResourceGroupImportFromFileParams& importParamsPrevious, const CarbonResources::PatchApplyParams patchApplyParams) const
{
	if( s_verbosityLevel == CarbonResources::StatusLevel::OFF )
	{
		return;
	}

	std::cout << "---Applying Patch---" << std::endl;

    PrintCommonOperationHeaderInformation();

	std::cout << "Patch Resource Group: " << importParamsPrevious.filename << std::endl;
	std::cout << "Patch Binaries Base Paths: " << PathsToString( patchApplyParams.patchBinarySourceSettings.basePaths ) << std::endl;
	std::cout << "Patch Binaries Source Type: " << SourceTypeToString( patchApplyParams.patchBinarySourceSettings.sourceType ) << std::endl;
	std::cout << "Resources To Patch Base Paths: " << PathsToString( patchApplyParams.resourcesToPatchSourceSettings.basePaths ) << std::endl;
	std::cout << "Resources To Patch Source Type: " << SourceTypeToString( patchApplyParams.resourcesToPatchSourceSettings.sourceType ) << std::endl;
	std::cout << "Next Resources Base Path: " << PathsToString( patchApplyParams.nextBuildResourcesSourceSettings.basePaths ) << std::endl;
	std::cout << "Next Resources Source Type: " << SourceTypeToString( patchApplyParams.nextBuildResourcesSourceSettings.sourceType ) << std::endl;
	std::cout << "Output Path Base Path: " << patchApplyParams.resourcesToPatchDestinationSettings.basePath << std::endl;
	std::cout << "Output Path Destination Type: " << DestinationTypeToString( patchApplyParams.resourcesToPatchDestinationSettings.destinationType ) << std::endl;

	std::cout << "----------------------------\n" << std::endl;
}

bool ApplyPatchCliOperation::ApplyPatch(CarbonResources::ResourceGroupImportFromFileParams& importParamsPrevious, CarbonResources::PatchApplyParams patchApplyParams) const
{
	CarbonResources::StatusCallback statusCallback = GetStatusCallback();

	if( statusCallback )
	{
		statusCallback( CarbonResources::StatusLevel::OVERVIEW, CarbonResources::StatusProgressType::PERCENTAGE, 0, "Applying Patch." );
	}

	// Load the patch file
	CarbonResources::PatchResourceGroup patchResourceGroup;

    importParamsPrevious.statusCallback = statusCallback;

	if( patchResourceGroup.ImportFromFile( importParamsPrevious ).type != CarbonResources::ResultType::SUCCESS )
	{
		return false;
	}

    if( statusCallback )
	{
		statusCallback( CarbonResources::StatusLevel::OVERVIEW, CarbonResources::StatusProgressType::PERCENTAGE, 20, "Applying patches from resource group" );
	}

    patchApplyParams.statusCallback = statusCallback;

    // Apply the patch
    CarbonResources::Result applyPatchResult = patchResourceGroup.Apply( patchApplyParams );

    if (applyPatchResult.type != CarbonResources::ResultType::SUCCESS)
    {
    	std::string out;
    	CarbonResources::ResultTypeToString( applyPatchResult.type, out );
    	std::cerr << "Failed to apply patch: " << out << std::endl;
    	exit(1);
    }

    if( statusCallback )
	{
		statusCallback( CarbonResources::StatusLevel::OVERVIEW, CarbonResources::StatusProgressType::PERCENTAGE, 100, "Successfully applied patch" );
	}
    
	return true;
}
