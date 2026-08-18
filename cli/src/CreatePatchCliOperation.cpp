// Copyright © 2025 CCP ehf.

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
	m_newFilesResourceGroupRelativePathArgumentId( "--new-files-resourcegroup-relative-path" ),
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
	m_networkRetryBackoffMultiplierId( "--download-retry" ),
	m_networkRetryCountId( "--network-retry-count" ),
	m_indexFolderArgumentId( "--index-folder" ),
	m_skipCompressionCalculation( "--skip-compression" ),
	m_newFilesResourceGroupDestinationTypeArgumentId( "--new-files-resourcegroup-destination-type" ),
	m_newFilesResourceGroupDestinationPathArgumentId( "--new-files-resourcegroup-destination-base-path" ),
	m_maxOverallPatchArgumentId( "--max-overall-patch-size" )
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

    AddArgument( m_newFilesResourceGroupRelativePathArgumentId, "Relative path for output new files ResourceGroup which will contain all the new files added between supplied builds.", false, false, defaultParams.resourceGroupNewFilesRelativePath.string() );

	AddArgument( m_resourceSourceTypePreviousArgumentId, "Represents the type of repository to source resources for previous.", false, false, SourceTypeToString( defaultParams.resourceSourceSettingsPrevious.sourceType ), ResourceSourceTypeChoicesAsString() );

	AddArgument( m_patchBinaryDestinationBasePathArgumentId, "Represents the base path where binary patches will be saved.", false, false, defaultParams.resourcePatchBinaryDestinationSettings.basePath.string() );

	AddArgument( m_patchResourceGroupDestinationTypeArgumentId, "Represents the type of repository where the patch ResourceGroup will be saved.", false, false, DestinationTypeToString( defaultParams.resourcePatchResourceGroupDestinationSettings.destinationType ), ResourceDestinationTypeChoicesAsString() );

	AddArgument( m_patchResourceGroupDestinationBasePathArgumentId, "Represents the base path where the patch ResourceGroup will be saved.", false, false, defaultParams.resourcePatchResourceGroupDestinationSettings.basePath.string() );

	AddArgument( m_patchFileRelativePathPrefixArgumentId, "Relative path prefix for produced patch binaries. Default is 'Patches/Patch' which will produce patches such as Patches/Patch.1 ...", false, false, defaultParams.patchFileRelativePathPrefix.string() );

	AddArgument( m_maxInputChunkSizeArgumentId, "Files are processed in chunks, maxInputFileChunkSize indicate the size of this chunk. Files smaller than chunk will be processed in one pass.", false, false, SizeToString( defaultParams.maxInputFileChunkSize ) );

    AddArgument( m_networkRetryCountId, "Number of retries to attempt when encountering a failed download.", false, false, SizeToString( defaultParams.downloadSettings.retryCount ) );

	AddArgument( m_networkRetryBackoffMultiplierId, "Multiplier in seconds to wait for when retrying, value will multiply on each retry to backoff.", false, false, SecondsToString( defaultParams.downloadSettings.retrySeconds ) );

	AddArgument( m_indexFolderArgumentId, "The folder in which to place indexes generated for patch files.", false, false, defaultParams.indexFolder.string() );

    AddArgumentFlag( m_skipCompressionCalculation, "Set skip compression calculations on patches." );

    AddArgument( m_newFilesResourceGroupDestinationTypeArgumentId, "Represents the type of repository where the new files ResourceGroup will be saved.", false, false, DestinationTypeToString( defaultParams.resourceNewFilesResourceGroupDestinationSettings.destinationType ), ResourceDestinationTypeChoicesAsString() );

	AddArgument( m_newFilesResourceGroupDestinationPathArgumentId, "Represents the base path where the new files ResourceGroup will be saved.", false, false, defaultParams.resourceNewFilesResourceGroupDestinationSettings.basePath.string() );

    AddArgument( m_maxOverallPatchArgumentId, "The maximum overall patch size. If patch generation goes over this value the process will stop with failure. This value represents the uncompressed size. A value of 0 will be treated as unlimited", false, false, SizeToString( defaultParams.maxTotalPatchSize ) );
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

    createPatchParams.resourceGroupNewFilesRelativePath = m_argumentParser->get<std::string>( m_newFilesResourceGroupRelativePathArgumentId );

	std::string resourceSourceTypePrevious = m_argumentParser->get<std::string>( m_resourceSourceTypePreviousArgumentId );

	if( !StringToResourceSourceType( resourceSourceTypePrevious, createPatchParams.resourceSourceSettingsPrevious.sourceType ) )
	{
		returnErrorMessage = "Invalid resource source previous type";

		return false;
	}

	for( std::string basePath : m_argumentParser->get<std::vector<std::string>>( m_resourceSourceBasePathPreviousArgumentId ) )
	{
		createPatchParams.resourceSourceSettingsPrevious.basePaths.push_back( basePath );
	}

	std::string resourceSourceTypeNext = m_argumentParser->get<std::string>( m_resourceSourceTypeNextArgumentId );

	if( !StringToResourceSourceType( resourceSourceTypeNext, createPatchParams.resourceSourceSettingsNext.sourceType ) )
	{
		returnErrorMessage = "Invalid resource source next type";

		return false;
	}

	for( std::string basePath : m_argumentParser->get<std::vector<std::string>>( m_resourceSourceBasePathNextArgumentId ) )
	{
		createPatchParams.resourceSourceSettingsNext.basePaths.push_back( basePath );
	}

	std::string patchBinaryDestinationType = m_argumentParser->get<std::string>( m_patchBinaryDestinationTypeArgumentId );

	if( !StringToResourceDestinationType( patchBinaryDestinationType, createPatchParams.resourcePatchBinaryDestinationSettings.destinationType ) )
	{
		returnErrorMessage = "Invalid patch binary destination type";

		return false;
	}

	createPatchParams.resourcePatchBinaryDestinationSettings.basePath = m_argumentParser->get<std::string>( m_patchBinaryDestinationBasePathArgumentId );

	std::string patchResourceGroupDestinationType = m_argumentParser->get<std::string>( m_patchResourceGroupDestinationTypeArgumentId );

	if( !StringToResourceDestinationType( patchResourceGroupDestinationType, createPatchParams.resourcePatchResourceGroupDestinationSettings.destinationType ) )
	{
		returnErrorMessage = "Invalid resource group destination type";

		return false;
	}

	createPatchParams.resourcePatchResourceGroupDestinationSettings.basePath = m_argumentParser->get<std::string>( m_patchResourceGroupDestinationBasePathArgumentId );

	createPatchParams.patchFileRelativePathPrefix = m_argumentParser->get<std::string>( m_patchFileRelativePathPrefixArgumentId );


	try
	{
		unsigned long in = std::stoul( m_argumentParser->get( m_maxInputChunkSizeArgumentId ) );
		if( in > std::numeric_limits<uint32_t>::max() )
		{
			returnErrorMessage = "Invalid chunk size";
			return false;
		}
		createPatchParams.maxInputFileChunkSize = static_cast<uint32_t>( in );
	}
	catch( std::invalid_argument& )
	{
		returnErrorMessage = "Invalid chunk size";
		return false;
	}
	catch( std::out_of_range& )
	{
		returnErrorMessage = "Invalid chunk size";
		return false;
	}

	long long retrySeconds{ 120 };

    try
	{
		unsigned long long networkRetryCountUnsignedLongLong = std::stoull( m_argumentParser->get( m_networkRetryCountId ) );

		if( networkRetryCountUnsignedLongLong > std::numeric_limits<uint32_t>::max() )
		{
			return false;
		}

		createPatchParams.downloadSettings.retryCount = static_cast<uint32_t>( networkRetryCountUnsignedLongLong );
	}
	catch( std::invalid_argument& )
	{
		return false;
	}
	catch( std::out_of_range& )
	{
		return false;
	}

	try
	{
		unsigned long long networkRetryBackoffMultiplierLongLong = std::stoull( m_argumentParser->get( m_networkRetryBackoffMultiplierId ) );

		createPatchParams.downloadSettings.retrySeconds = std::chrono::seconds( networkRetryBackoffMultiplierLongLong );
	}
	catch( std::invalid_argument& )
	{
		return false;
	}
	catch( std::out_of_range& )
	{
		return false;
	}

	createPatchParams.downloadSettings.retrySeconds = std::chrono::seconds( retrySeconds );

	createPatchParams.indexFolder = m_argumentParser->get( m_indexFolderArgumentId );

    bool skipCompressionCalculation = m_argumentParser->get<bool>( m_skipCompressionCalculation );

    if (skipCompressionCalculation && createPatchParams.resourcePatchBinaryDestinationSettings.destinationType == CarbonResources::ResourceDestinationType::REMOTE_CDN)
    {
		returnErrorMessage = "Cannot skip compression when patch desination type is REMOTE_CDN.";

		return false;
    }

    createPatchParams.calculateCompressions = !skipCompressionCalculation;

    createPatchParams.resourceNewFilesResourceGroupDestinationSettings.basePath = m_argumentParser->get<std::string>( m_newFilesResourceGroupDestinationPathArgumentId );

	std::string newFilesResourceGroupDestinationType = m_argumentParser->get<std::string>( m_newFilesResourceGroupDestinationTypeArgumentId );

	if( !StringToResourceDestinationType( newFilesResourceGroupDestinationType, createPatchParams.resourceNewFilesResourceGroupDestinationSettings.destinationType ) )
	{
		returnErrorMessage = "Invalid new file destination type";

		return false;
	}

    try
	{
		unsigned long in = std::stoul( m_argumentParser->get( m_maxOverallPatchArgumentId ) );
		if( in > std::numeric_limits<uint32_t>::max() )
		{
			returnErrorMessage = "Invalid overall patch size";
			return false;
		}
		createPatchParams.maxTotalPatchSize = static_cast<uint32_t>( in );
	}
	catch( std::invalid_argument& )
	{
		returnErrorMessage = "Invalid overall patch size";
		return false;
	}
	catch( std::out_of_range& )
	{
		returnErrorMessage = "Invalid overall patch size";
		return false;
	}

	if( ShowCliStatusUpdates() )
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

    std::cout << "Resource Group New Files Relative Path: " << createPatchParams.resourceGroupNewFilesRelativePath << std::endl;

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

	std::cout << "Network retry backoff multiplier ( Seconds ):" << createPatchParams.downloadSettings.retrySeconds.count() << std::endl;

    std::cout << "Download Retry Count: " << createPatchParams.downloadSettings.retryCount << std::endl;

	std::cout << "Index File Folder: " << createPatchParams.indexFolder << std::endl;

    if( createPatchParams.calculateCompressions )
	{
		std::cout << "Calculate Compression: Off" << std::endl;
	}
	else
	{
		std::cout << "Calculate Compression: On" << std::endl;
	}

    std::cout << "New File Resource Group Destination Settings Base Path: " << createPatchParams.resourceNewFilesResourceGroupDestinationSettings.basePath << std::endl;

	std::cout << "New File Resource Group Destination Settings Destination Type: " << DestinationTypeToString( createPatchParams.resourceNewFilesResourceGroupDestinationSettings.destinationType ) << std::endl;

    std::cout << "Max overall patch size: " << createPatchParams.maxTotalPatchSize << std::endl;

	std::cout << "----------------------------\n"
			  << std::endl;
}

bool CreatePatchCliOperation::CreatePatch( CarbonResources::ResourceGroupImportFromFileParams& previousResourceGroupParams, CarbonResources::ResourceGroupImportFromFileParams& nextResourceGroupParams, CarbonResources::PatchCreateParams& createPatchParams ) const
{
	CarbonResources::StatusCallback statusCallback = GetStatusCallback();

	// Get status callback relevant to verbosity level
	createPatchParams.callbackSettings.statusCallback = statusCallback;
	createPatchParams.callbackSettings.verbosityLevel = GetVerbosityLevel();

	// Previous ResourceGroup
	CarbonResources::ResourceGroup resourceGroupPrevious;

	previousResourceGroupParams.callbackSettings.statusCallback = statusCallback;
	previousResourceGroupParams.callbackSettings.verbosityLevel = GetVerbosityLevel();

    if( ShowCliStatusUpdates() )
	{
		CliStatusUpdate( "Importing previous Resource Group from file." );
	}

	CarbonResources::Result importPreviousFromFileResult = resourceGroupPrevious.ImportFromFile( previousResourceGroupParams );

	if( importPreviousFromFileResult.type != CarbonResources::ResultType::SUCCESS )
	{
		PrintCarbonResourcesError( importPreviousFromFileResult );

		return false;
	}

	// Latest ResourceGroup
	CarbonResources::ResourceGroup resourceGroupLatest;

	nextResourceGroupParams.callbackSettings.statusCallback = statusCallback;
	nextResourceGroupParams.callbackSettings.verbosityLevel = GetVerbosityLevel();

    if( ShowCliStatusUpdates() )
	{
		CliStatusUpdate( "Importing next Resource Group from file." );
	}

	CarbonResources::Result importNextFromFileResult = resourceGroupLatest.ImportFromFile( nextResourceGroupParams );

	if( importNextFromFileResult.type != CarbonResources::ResultType::SUCCESS )
	{
		PrintCarbonResourcesError( importNextFromFileResult );

		return false;
	}

	createPatchParams.previousResourceGroup = &resourceGroupPrevious;

	// Create Patch
	if( ShowCliStatusUpdates() )
	{
		CliStatusUpdate( "Creating Patch." );
	}

	CarbonResources::Result createPatchResult = resourceGroupLatest.CreatePatch( createPatchParams );

	if( createPatchResult.type != CarbonResources::ResultType::SUCCESS )
	{
		PrintCarbonResourcesError( createPatchResult );

		return false;
	}

    if( ShowCliStatusUpdates() )
	{
		CliStatusUpdate( "Operation complete." );
	}

	return true;
}