#include "CreatePatchCliOperation.h"

#include <string>
#include <argparse/argparse.hpp>
#include <ResourceGroup.h>

CreatePatchCliOperation::CreatePatchCliOperation() :
	CliOperation( "create-patch", "Creates a patch binaries and a Patch Resource Group from two supplied ResourceGroups and two resource source directories, one for previous build and one for next." ),
	m_previousResourceGroupPathArgumentId( "previous-resourcegroup-path" ),
	m_nextResourceGroupPathArgumentId( "next-resourcegroup-path" ),
	m_resourceGroupRelativePathArgumentId( "--resourcegroup-relative-path" ),
	m_patchResourceGroupRelativePathArgumentId( "--patchResourcegroup-relative-path" ),
	m_resourceSourceTypePreviousArgumentId( "--resource-source-type-previous" ),
	m_resourceSourceBasePathPreviousArgumentId( "--resource-source-base-path-previous" ),
	m_resourceSourceTypeNextArgumentId( "--resource-source-type-next" ),
	m_resourceSourceBasePathNextArgumentId( "--resource-source-base-path-next" ),
	m_patchBinaryDestinationTypeArgumentId( "--patch-destination-type" ),
	m_patchBinaryDestinationBasePathArgumentId( "--patch-destination-base-path" ),
	m_patchResourceGroupDestinationTypeArgumentId( "--patch-resourcegroup-destination-type" ),
	m_patchResourceGroupDestinationBasePathArgumentId( "--patch-resourcegroup-destination-path" ),
	m_patchFileRelativePathPrefixArgumentId( "--patch-prefix" ),
	m_maxInputChunkSizeArgumentId( "--chunk-size" ),
	m_downloadRetrySecondsArgumentId( "--download-retry" ),
	m_indexFolderArgumentId( "--index-folder" )
{

	AddRequiredPositionalArgument( m_previousResourceGroupPathArgumentId, "Filename to previous resourceGroup." );

    AddRequiredPositionalArgument( m_nextResourceGroupPathArgumentId, "Filename to next resourceGroup." );

    // Struct is inspected to ascertain default values
	// This keeps default value settings in one place
	// Lib defaults matches CLI
    CarbonResources::PatchCreateParams defaultParams;

    AddArgument( m_resourceSourceBasePathPreviousArgumentId, "Represents the base path to source resources for previous.", true, true, PathListToString( defaultParams.resourceSourceSettingsPrevious.basePaths ) );

    AddArgument( m_resourceSourceBasePathNextArgumentId, "Represents the base path to source resources for next.", true, true, PathListToString( defaultParams.resourceSourceSettingsNext.basePaths ) );

    AddArgument( m_resourceSourceTypeNextArgumentId, "Represents the type of repository to source resources for next.", false, false, SourceTypeToString( defaultParams.resourceSourceSettingsNext.sourceType ), ResourceSourceTypeChoicesAsString() );

	AddArgument( m_patchBinaryDestinationTypeArgumentId, "Represents the type of repository where binary patches will be saved.", false, false, DestinationTypeToString( defaultParams.resourcePatchBinaryDestinationSettings.destinationType ), ResourceDestinationTypeChoicesAsString() );

    AddArgument( m_resourceGroupRelativePathArgumentId, "Relative path for output resourceGroup which will contain the diff between the supplied previous ResourceGroup and next ResourceGroup.", false, false, defaultParams.resourceGroupRelativePath.string() );

    AddArgument( m_patchResourceGroupRelativePathArgumentId, "Relative path for output PatchResourceGroup which will contain all the patches produced.", false, false, defaultParams.resourceGroupPatchRelativePath.string() );

    AddArgument( m_resourceSourceTypePreviousArgumentId, "Represents the type of repository to source resources for previous.", false, false, SourceTypeToString( defaultParams.resourceSourceSettingsPrevious.sourceType ), ResourceSourceTypeChoicesAsString() );

	AddArgument( m_patchBinaryDestinationBasePathArgumentId, "Represents the base path where binary patches will be saved.", false, false, defaultParams.resourcePatchBinaryDestinationSettings.basePath.string() );

    AddArgument( m_patchResourceGroupDestinationTypeArgumentId, "Represents the type of repository where the patch ResourceGroup will be saved.", false, false, DestinationTypeToString( defaultParams.resourcePatchResourceGroupDestinationSettings.destinationType ), ResourceDestinationTypeChoicesAsString() );

	AddArgument( m_patchResourceGroupDestinationBasePathArgumentId, "Represents the base path where the patch ResourceGroup will be saved.", false, false, defaultParams.resourcePatchResourceGroupDestinationSettings.basePath.string() );

    AddArgument( m_patchFileRelativePathPrefixArgumentId, "Relative path prefix for produced patch binaries. Default is “Patches/Patch” which will produce patches such as Patches/Patch.1 …", false, false, defaultParams.patchFileRelativePathPrefix.string() );

    AddArgument( m_maxInputChunkSizeArgumentId, "Files are processed in chunks, maxInputFileChunkSize indicate the size of this chunk. Files smaller than chunk will be processed in one pass.", false, false, SizeToString( defaultParams.maxInputFileChunkSize ) );

	AddArgument( m_downloadRetrySecondsArgumentId, "The number of seconds before attempt to download a resource fails with a network related error", false, false, SecondsToString( defaultParams.downloadRetrySeconds ) );

	AddArgument( m_indexFolderArgumentId, "The folder in which to place indexes generated for patch files.", false, false, defaultParams.indexFolder.string() );
}

bool CreatePatchCliOperation::Execute( std::string& returnErrorMessage ) const
{
	CarbonResources::ResourceGroupImportFromFileParams previousResourceGroupParams;

    previousResourceGroupParams.filename = m_argumentParser->get<std::string>( m_previousResourceGroupPathArgumentId );

    CarbonResources::ResourceGroupImportFromFileParams nextResourceGroupParams;

    nextResourceGroupParams.filename = m_argumentParser->get<std::string>( m_nextResourceGroupPathArgumentId );

    CarbonResources::PatchCreateParams createPatchParams;

	createPatchParams.resourceGroupRelativePath = m_argumentParser->get<std::string>( m_resourceGroupRelativePathArgumentId );

	createPatchParams.resourceGroupPatchRelativePath = m_argumentParser->get<std::string>( m_patchResourceGroupRelativePathArgumentId );

	std::string resourceSourceTypePrevious = m_argumentParser->get<std::string>( m_resourceSourceTypePreviousArgumentId );

    if (!StringToResourceSourceType(resourceSourceTypePrevious, createPatchParams.resourceSourceSettingsPrevious.sourceType))
    {
		returnErrorMessage = "Invalid resource source previous type";

		return false;
    }

    for (std::string basePath : m_argumentParser->get<std::vector<std::string>>(m_resourceSourceBasePathPreviousArgumentId))
    {
		createPatchParams.resourceSourceSettingsPrevious.basePaths.push_back( basePath );
    }

	std::string resourceSourceTypeNext = m_argumentParser->get<std::string>( m_resourceSourceTypeNextArgumentId );

    if (!StringToResourceSourceType(resourceSourceTypeNext, createPatchParams.resourceSourceSettingsNext.sourceType))
    {
		returnErrorMessage = "Invalid resource source next type";

		return false;
    }

    for (std::string basePath : m_argumentParser->get<std::vector<std::string>>(m_resourceSourceBasePathNextArgumentId))
    {
		createPatchParams.resourceSourceSettingsNext.basePaths.push_back( basePath );
    }

	std::string patchBinaryDestinationType = m_argumentParser->get<std::string>( m_patchBinaryDestinationTypeArgumentId );

    if (!StringToResourceDestinationType(patchBinaryDestinationType, createPatchParams.resourcePatchBinaryDestinationSettings.destinationType))
    {
		returnErrorMessage = "Invalid patch binary destination type";

		return false;
    }

	createPatchParams.resourcePatchBinaryDestinationSettings.basePath = m_argumentParser->get<std::string>( m_patchBinaryDestinationBasePathArgumentId );

	std::string patchResourceGroupDestinationType = m_argumentParser->get<std::string>( m_patchResourceGroupDestinationTypeArgumentId );

    if (!StringToResourceDestinationType(patchResourceGroupDestinationType, createPatchParams.resourcePatchResourceGroupDestinationSettings.destinationType))
    {
		returnErrorMessage = "Invalid resource group destination type";

		return false;
    }

	createPatchParams.resourcePatchResourceGroupDestinationSettings.basePath = m_argumentParser->get<std::string>( m_patchResourceGroupDestinationBasePathArgumentId );

	createPatchParams.patchFileRelativePathPrefix = m_argumentParser->get<std::string>( m_patchFileRelativePathPrefixArgumentId );

	
	try
	{
		createPatchParams.maxInputFileChunkSize = std::stoull( m_argumentParser->get( m_maxInputChunkSizeArgumentId ) );
	}
	catch( std::invalid_argument& e )
	{
		returnErrorMessage = "Invalid chunk size";

		return false;
	}
	catch( std::out_of_range& e )
	{
		returnErrorMessage = "Invalid chunk size";
		return false;
	}

    long long retrySeconds{ 120 };

    try
	{
		retrySeconds = std::stoll( m_argumentParser->get( m_downloadRetrySecondsArgumentId ) );
	}
	catch( std::invalid_argument& e )
	{
		returnErrorMessage = "Invalid retry seconds";

		return false;
	}
	catch( std::out_of_range& e )
	{
		returnErrorMessage = "Invalid retry seconds";

		return false;
	}

	createPatchParams.downloadRetrySeconds = std::chrono::seconds( retrySeconds );

    if( s_verbosityLevel == CarbonResources::STATUS_LEVEL::OFF )
    {
		PrintStartBanner( previousResourceGroupParams, nextResourceGroupParams, createPatchParams );
    }

	createPatchParams.indexFolder = m_argumentParser->get( m_indexFolderArgumentId );

	return CreatePatch( previousResourceGroupParams, nextResourceGroupParams, createPatchParams );
}

void CreatePatchCliOperation::PrintStartBanner( const CarbonResources::ResourceGroupImportFromFileParams& previousResourceGroupParams, const CarbonResources::ResourceGroupImportFromFileParams& nextResourceGroupParams, CarbonResources::PatchCreateParams& createPatchParams ) const
{
	std::cout << "---Running Patch Creation---" << std::endl;

    PrintCommonOperationHeaderInformation();

	std::cout << "Previous Resource Group: " << previousResourceGroupParams.filename << std::endl;

	std::cout << "Next Resource Group: " << nextResourceGroupParams.filename << std::endl;

	std::cout << "Max Input File Chunk Size: " << createPatchParams.maxInputFileChunkSize << std::endl;

	std::cout << "Resource Group Relative Path: " << createPatchParams.resourceGroupRelativePath << std::endl;

	std::cout << "Resource Group Patch Relative Path: " << createPatchParams.resourceGroupPatchRelativePath << std::endl;

	std::cout << "Patch File Relative Path Prefix: " << createPatchParams.patchFileRelativePathPrefix << std::endl;

    for( std::filesystem::path basePath : createPatchParams.resourceSourceSettingsPrevious.basePaths )
	{
		std::cout << "Resource Source Settings From Base Path: " << basePath << std::endl;
	}

	std::cout << "Resource Source Settings From Source Type: " << SourceTypeToString( createPatchParams.resourceSourceSettingsPrevious.sourceType ) << std::endl;

    for( std::filesystem::path basePath : createPatchParams.resourceSourceSettingsNext.basePaths )
	{
		std::cout << "Resource Source Settings To Base Path: " << basePath << std::endl;
	}

	std::cout << "Resource Source Settings To Source Type: " << SourceTypeToString( createPatchParams.resourceSourceSettingsNext.sourceType ) << std::endl;

	std::cout << "Resource Patch Binary Destination Settings Base Path: " << createPatchParams.resourcePatchBinaryDestinationSettings.basePath << std::endl;

	std::cout << "Resource Patch Binary Destination Settings Destination Type: " << DestinationTypeToString( createPatchParams.resourcePatchBinaryDestinationSettings.destinationType ) << std::endl;

	std::cout << "Resource Patch Resource Group Destination Settings Base Path: " << createPatchParams.resourcePatchResourceGroupDestinationSettings.basePath << std::endl;

	std::cout << "Resource Patch Resource Group Destination Settings Destination Type: " << DestinationTypeToString( createPatchParams.resourcePatchResourceGroupDestinationSettings.destinationType ) << std::endl;

	std::cout << "Download Retry Seconds: " <<  createPatchParams.downloadRetrySeconds.count() << std::endl;

	std::cout << "Index File Folder: " << createPatchParams.indexFolder;

	std::cout << "----------------------------\n" << std::endl;
}

bool CreatePatchCliOperation::CreatePatch( CarbonResources::ResourceGroupImportFromFileParams& previousResourceGroupParams, CarbonResources::ResourceGroupImportFromFileParams& nextResourceGroupParams, CarbonResources::PatchCreateParams& createPatchParams ) const
{
	CarbonResources::StatusCallback statusCallback = GetStatusCallback();

    // Get status callback relevant to verbosity level
	createPatchParams.statusCallback = statusCallback;

	// Previous ResourceGroup
	if( createPatchParams.statusCallback )
	{
		createPatchParams.statusCallback( CarbonResources::STATUS_LEVEL::OVERVIEW, CarbonResources::STATUS_PROGRESS_TYPE::PERCENTAGE, 0, "Loading previous resource group." );
	}

	CarbonResources::ResourceGroup resourceGroupPrevious;

    previousResourceGroupParams.statusCallback = statusCallback;

	CarbonResources::Result importPreviousFromFileResult = resourceGroupPrevious.ImportFromFile( previousResourceGroupParams );

	if( importPreviousFromFileResult.type != CarbonResources::ResultType::SUCCESS )
	{
		PrintCarbonResourcesError( importPreviousFromFileResult );

		return false;
	}

	// Latest ResourceGroup
	if( createPatchParams.statusCallback )
	{
		createPatchParams.statusCallback( CarbonResources::STATUS_LEVEL::OVERVIEW, CarbonResources::STATUS_PROGRESS_TYPE::PERCENTAGE, 50, "Loading latest resource group." );
	}

	CarbonResources::ResourceGroup resourceGroupLatest;

    nextResourceGroupParams.statusCallback = statusCallback;

	CarbonResources::Result importNextFromFileResult = resourceGroupLatest.ImportFromFile( nextResourceGroupParams );

	if( importNextFromFileResult.type != CarbonResources::ResultType::SUCCESS )
	{
		PrintCarbonResourcesError( importPreviousFromFileResult );

		return false;
	}

    createPatchParams.previousResourceGroup = &resourceGroupPrevious;

    // Create Patch
	if( createPatchParams.statusCallback )
	{
		createPatchParams.statusCallback( CarbonResources::STATUS_LEVEL::OVERVIEW, CarbonResources::STATUS_PROGRESS_TYPE::PERCENTAGE, 75, "Creating Patch." );
	}

	CarbonResources::Result createPatchResult = resourceGroupLatest.CreatePatch( createPatchParams );

	if( createPatchResult.type != CarbonResources::ResultType::SUCCESS )
	{
		PrintCarbonResourcesError( createPatchResult );

		return false;
	}

    if( createPatchParams.statusCallback )
	{
		createPatchParams.statusCallback( CarbonResources::STATUS_LEVEL::OVERVIEW, CarbonResources::STATUS_PROGRESS_TYPE::PERCENTAGE, 100, "Patch created succesfully." );
	}

    return true;
}