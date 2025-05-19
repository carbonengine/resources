#include "CreatePatchCliOperation.h"

#include <string>
#include <argparse/argparse.hpp>
#include <ResourceGroup.h>

CreatePatchCliOperation::CreatePatchCliOperation() :
	CliOperation( "create-patch", "Create a patch from two supplied ResourceGroups" ),
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
	m_downloadRetrySecondsArgumentId( "--download-retry" )
{

    // TODO: The interface here is totally WIP, it needs actually designing to be easy to use.
	// This is linked to having sensible default values so that not all options are always required and large complex commands are opt in.
	AddRequiredPositionalArgument( m_previousResourceGroupPathArgumentId, "Filename to previous resourceGroup." );

    AddRequiredPositionalArgument( m_nextResourceGroupPathArgumentId, "Filename to next resourceGroup." );

    AddArgument( m_resourceGroupRelativePathArgumentId, "Relative path for output resourceGroup which will contain the diff between the supplied previous ResourceGroup and next ResourceGroup.", false, false, "ResourceGroup.yaml" );

    AddArgument( m_patchResourceGroupRelativePathArgumentId, "Relative path for output PatchResourceGroup which will contain all the patches produced.", false, false, "PatchResourceGroup.yaml" );

    AddArgument( m_resourceSourceTypePreviousArgumentId, "Represents the type of repository to source resources for previous.", false, false, "LOCAL_RELATIVE" );

    AddArgument( m_resourceSourceBasePathPreviousArgumentId, "Represents the base path to source resources for previous.", false, true, "" );

    AddArgument( m_resourceSourceTypeNextArgumentId, "Represents the type of repository to source resources for next.", false, false, "LOCAL_RELATIVE" );

	AddArgument( m_resourceSourceBasePathNextArgumentId, "Represents the base path to source resources for next.", false, true, "" );

    AddArgument( m_patchBinaryDestinationTypeArgumentId, "Represents the type of repository where binary patches will be saved.", false, false, "LOCAL_RELATIVE" );

	AddArgument( m_patchBinaryDestinationBasePathArgumentId, "Represents the base path where binary patches will be saved.", false, false, "." );

    AddArgument( m_patchResourceGroupDestinationTypeArgumentId, "Represents the type of repository where the patch ResourceGroup will be saved.", false, false, "LOCAL_RELATIVE" );

	AddArgument( m_patchResourceGroupDestinationBasePathArgumentId, "Represents the base path where the patch ResourceGroup will be saved.", false, false, "." );

    AddArgument( m_patchFileRelativePathPrefixArgumentId, "Relative path prefix for produced patch binaries. Default is “Patches/Patch” which will produce patches such as Patches/Patch.1 …", false, false, "Patches/Patch" );

    AddArgument( m_maxInputChunkSizeArgumentId, "Files are processed in chunks, maxInputFileChunkSize indicate the size of this chunk. Files smaller than chunk will be processed in one pass.", false, false, "100000000" );

	AddArgument( m_downloadRetrySecondsArgumentId, "The number of seconds before attempt to download a resource fails with a network related error", false, false, "120");
}

bool CreatePatchCliOperation::Execute() const
{
	CarbonResources::ResourceGroupImportFromFileParams previousResourceGroupParams;

    previousResourceGroupParams.filename = m_argumentParser->get<std::string>( m_previousResourceGroupPathArgumentId );

    CarbonResources::ResourceGroupImportFromFileParams nextResourceGroupParams;

    nextResourceGroupParams.filename = m_argumentParser->get<std::string>( m_nextResourceGroupPathArgumentId );

    CarbonResources::PatchCreateParams createPatchParams;

	createPatchParams.resourceGroupRelativePath = m_argumentParser->get<std::string>( m_resourceGroupRelativePathArgumentId );

	createPatchParams.resourceGroupPatchRelativePath = m_argumentParser->get<std::string>( m_patchResourceGroupRelativePathArgumentId );

	std::string resourceSourceTypePrevious = m_argumentParser->get<std::string>( m_resourceSourceTypePreviousArgumentId );

	if( resourceSourceTypePrevious == "LOCAL_CDN" )
	{
		createPatchParams.resourceSourceSettingsFrom.sourceType = CarbonResources::ResourceSourceType::LOCAL_CDN;
	}
	else if( resourceSourceTypePrevious == "REMOTE_CDN" )
	{
		createPatchParams.resourceSourceSettingsFrom.sourceType = CarbonResources::ResourceSourceType::REMOTE_CDN;
	}
	else if( resourceSourceTypePrevious == "LOCAL_RELATIVE" )
	{
		createPatchParams.resourceSourceSettingsFrom.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;
	}
	else
	{
		return false;
	}

    for (std::string basePath : m_argumentParser->get<std::vector<std::string>>(m_resourceSourceBasePathPreviousArgumentId))
    {
		createPatchParams.resourceSourceSettingsFrom.basePaths.push_back( basePath );
    }

	std::string resourceSourceTypeNext = m_argumentParser->get<std::string>( m_resourceSourceTypeNextArgumentId );

	if( resourceSourceTypeNext == "LOCAL_CDN" )
	{
		createPatchParams.resourceSourceSettingsTo.sourceType = CarbonResources::ResourceSourceType::LOCAL_CDN;
	}
	else if( resourceSourceTypeNext == "REMOTE_CDN" )
	{
		createPatchParams.resourceSourceSettingsTo.sourceType = CarbonResources::ResourceSourceType::REMOTE_CDN;
	}
	else if( resourceSourceTypeNext == "LOCAL_RELATIVE" )
	{
		createPatchParams.resourceSourceSettingsTo.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;
	}
	else
	{
		return false;
	}

    for (std::string basePath : m_argumentParser->get<std::vector<std::string>>(m_resourceSourceBasePathNextArgumentId))
    {
		createPatchParams.resourceSourceSettingsTo.basePaths.push_back( basePath );
    }

	std::string patchBinaryDestinationType = m_argumentParser->get<std::string>( m_patchBinaryDestinationTypeArgumentId );

	if( patchBinaryDestinationType == "LOCAL_CDN" )
	{
		createPatchParams.resourcePatchBinaryDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_CDN;
	}
	else if( patchBinaryDestinationType == "REMOTE_CDN" )
	{
		createPatchParams.resourcePatchBinaryDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::REMOTE_CDN;
	}
	else if( patchBinaryDestinationType == "LOCAL_RELATIVE" )
	{
		createPatchParams.resourcePatchBinaryDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_RELATIVE;
	}
	else
	{
		return false;
	}

	createPatchParams.resourcePatchBinaryDestinationSettings.basePath = m_argumentParser->get<std::string>( m_patchBinaryDestinationBasePathArgumentId );

	std::string patchResourceGroupDestinationType = m_argumentParser->get<std::string>( m_patchResourceGroupDestinationTypeArgumentId );

	if( patchResourceGroupDestinationType == "LOCAL_CDN" )
	{
		createPatchParams.resourcePatchResourceGroupDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_CDN;
	}
	else if( patchResourceGroupDestinationType == "REMOTE_CDN" )
	{
		createPatchParams.resourcePatchResourceGroupDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::REMOTE_CDN;
	}
	else if( patchResourceGroupDestinationType == "LOCAL_RELATIVE" )
	{
		createPatchParams.resourcePatchResourceGroupDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_RELATIVE;
	}
	else
	{
		return false;
	}

	createPatchParams.resourcePatchResourceGroupDestinationSettings.basePath = m_argumentParser->get<std::string>( m_patchResourceGroupDestinationBasePathArgumentId );

	createPatchParams.patchFileRelativePathPrefix = m_argumentParser->get<std::string>( m_patchFileRelativePathPrefixArgumentId );

	long long retrySeconds{120};
	try
	{
		createPatchParams.maxInputFileChunkSize = std::stoull( m_argumentParser->get( m_maxInputChunkSizeArgumentId ) );
		retrySeconds = std::stoll( m_argumentParser->get( m_downloadRetrySecondsArgumentId ) );
	}
	catch( std::invalid_argument& e )
	{
		return false;
	}
	catch( std::out_of_range& e )
	{
		return false;
	}
	createPatchParams.downloadRetrySeconds = std::chrono::seconds( retrySeconds );

    if (s_verbosity > 0)
    {
		PrintStartBanner( previousResourceGroupParams, nextResourceGroupParams, createPatchParams );
    }

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

    for( std::filesystem::path basePath : createPatchParams.resourceSourceSettingsFrom.basePaths )
	{
		std::cout << "Resource Source Settings From Base Path: " << basePath << std::endl;
	}

	std::cout << "Resource Source Settings From Source Type: " << SourceTypeToString( createPatchParams.resourceSourceSettingsFrom.sourceType ) << std::endl;

    for( std::filesystem::path basePath : createPatchParams.resourceSourceSettingsTo.basePaths )
	{
		std::cout << "Resource Source Settings To Base Path: " << basePath << std::endl;
	}

	std::cout << "Resource Source Settings To Source Type: " << SourceTypeToString( createPatchParams.resourceSourceSettingsTo.sourceType ) << std::endl;

	std::cout << "Resource Patch Binary Destination Settings Base Path: " << createPatchParams.resourcePatchBinaryDestinationSettings.basePath << std::endl;

	std::cout << "Resource Patch Binary Destination Settings Destination Type: " << DestinationTypeToString( createPatchParams.resourcePatchBinaryDestinationSettings.destinationType ) << std::endl;

	std::cout << "Resource Patch Resource Group Destination Settings Base Path: " << createPatchParams.resourcePatchResourceGroupDestinationSettings.basePath << std::endl;

	std::cout << "Resource Patch Resource Group Destination Settings Destination Type: " << DestinationTypeToString( createPatchParams.resourcePatchResourceGroupDestinationSettings.destinationType ) << std::endl;

	std::cout << "Download Retry Seconds: " <<  createPatchParams.downloadRetrySeconds.count() << std::endl;

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
		createPatchParams.statusCallback( 0, 0, "Loading previous resource group." );
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
		createPatchParams.statusCallback( 0, 50, "Loading latest resource group." );
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
		createPatchParams.statusCallback( 0, 75, "Creating Patch." );
	}

	CarbonResources::Result createPatchResult = resourceGroupLatest.CreatePatch( createPatchParams );

	if( createPatchResult.type != CarbonResources::ResultType::SUCCESS )
	{
		PrintCarbonResourcesError( createPatchResult );

		return false;
	}

    if( createPatchParams.statusCallback )
	{
		createPatchParams.statusCallback( 0, 100, "Patch created succesfully." );
	}

    return true;
}