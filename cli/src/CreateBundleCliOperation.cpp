// Copyright © 2025 CCP ehf.

#include "CreateBundleCliOperation.h"

#include <string>

#include <argparse/argparse.hpp>
#include <yaml-cpp/yaml.h>

#include <ResourceGroup.h>
#include <PatchResourceGroup.h>

CreateBundleCliOperation::CreateBundleCliOperation() :
	CliOperation( "create-bundle", "Creates a bundle from supplied ResourceGroup. Bundle takes the form of individual binary chunks." ),
	m_inputResourceGroupPathArgumentId( "resourcegroup-path" ),
	m_resourceGroupRelativePathArgumentId( "--resourcegroup-relative-path" ),
	m_bundleResourceGroupRelativePathArgumentId( "--bundle-resourcegroup-relative-path" ),
	m_resourceSourceTypeArgumentId( "--resource-source-type" ),
	m_resourceSourceBasePathArgumentId( "--resource-source-path" ),
	m_chunkDestinationTypeArgumentId( "--chunk-destination-type" ),
	m_chunkDestinationBasePathArgumentId( "--chunk-destination-path" ),
	m_bundleResourceGroupDestinationTypeArgumentId( "--bundle-resourcegroup-destination-type" ),
	m_bundleResourceGroupDestinationBasePathArgumentId( "--bundle-resourcegroup-destination-path" ),
	m_chunkSizeArgumentId( "--chunk-size" ),
	m_streamChunkSizeId( "--stream-chunk-size" ),
	m_networkRetryBackoffMultiplierId( "--download-retry-seconds" ),
	m_networkRetryCountId( "--network-retry-count" )
{
	AddRequiredPositionalArgument( m_inputResourceGroupPathArgumentId, "Path to ResourceGroup to bundle." );

	// Struct is inspected to ascertain default values
	// This keeps default value settings in one place
	// Lib defaults matches CLI
	CarbonResources::BundleCreateParams defaultParams;

	AddArgument( m_resourceSourceBasePathArgumentId, "Represents the base path where the resources will be sourced.", true, true, PathListToString( defaultParams.resourceSourceSettings.basePaths ) );

	AddArgument( m_resourceSourceTypeArgumentId, "Represents the type of repository where resources will be sourced.", false, false, SourceTypeToString( defaultParams.resourceSourceSettings.sourceType ), ResourceSourceTypeChoicesAsString() );

	AddArgument( m_resourceGroupRelativePathArgumentId, "Relative path to save a ResourceGroup the Bundle was based off", false, false, defaultParams.resourceGroupRelativePath.string() );

	AddArgument( m_bundleResourceGroupRelativePathArgumentId, "Relative path to save a Bundle ResourceGroup", false, false, defaultParams.resourceGroupBundleRelativePath.string() );

	AddArgument( m_chunkDestinationTypeArgumentId, "Represents the type of repository where chunks will be saved.", false, false, DestinationTypeToString( defaultParams.chunkDestinationSettings.destinationType ), ResourceDestinationTypeChoicesAsString() );

	AddArgument( m_chunkDestinationBasePathArgumentId, "Represents the base path where the chunks will be saved.", false, false, defaultParams.chunkDestinationSettings.basePath.string() );

	AddArgument( m_bundleResourceGroupDestinationTypeArgumentId, "Represents the type of repository where the bundle ResourceGroup will be saved.", false, false, DestinationTypeToString( defaultParams.resourceBundleResourceGroupDestinationSettings.destinationType ), ResourceDestinationTypeChoicesAsString() );

	AddArgument( m_bundleResourceGroupDestinationBasePathArgumentId, "Represents the base path where the bundle ResourceGroup will be saved.", false, false, defaultParams.resourceBundleResourceGroupDestinationSettings.basePath.string() );

	AddArgument( m_chunkSizeArgumentId, "Represents the target size of the produced chunks in bytes. Note that produced chunks may not exactly match this value.", false, false, SizeToString( defaultParams.chunkSize ) );

    AddArgument( m_streamChunkSizeId, "Chunks stream size in bytes for streaming data.", false, false, SizeToString( defaultParams.fileReadChunkSize ) );

    AddArgument( m_networkRetryCountId, "Number of retries to attempt when encountering a failed download.", false, false, SizeToString( defaultParams.downloadSettings.retryCount ) );

	AddArgument( m_networkRetryBackoffMultiplierId, "Multiplier in seconds to wait for when retrying, value will multiply on each retry to backoff.", false, false, SecondsToString( defaultParams.downloadSettings.retrySeconds ) );
}

bool CreateBundleCliOperation::Execute( std::string& returnErrorMessage ) const
{
	CarbonResources::ResourceGroupImportFromFileParams resourceGroupParams;

	resourceGroupParams.filename = m_argumentParser->get<std::string>( m_inputResourceGroupPathArgumentId );

	CarbonResources::BundleCreateParams bundleCreateParams;

	bundleCreateParams.resourceGroupRelativePath = m_argumentParser->get<std::string>( m_resourceGroupRelativePathArgumentId );

	bundleCreateParams.resourceGroupBundleRelativePath = m_argumentParser->get<std::string>( m_bundleResourceGroupRelativePathArgumentId );

	std::string resourceSourceType = m_argumentParser->get( m_resourceSourceTypeArgumentId );

	if( !StringToResourceSourceType( resourceSourceType, bundleCreateParams.resourceSourceSettings.sourceType ) )
	{
		returnErrorMessage = "Invalid resources source type";

		return false;
	}

	for( const std::string& basePath : m_argumentParser->get<std::vector<std::string>>( m_resourceSourceBasePathArgumentId ) )
	{
		bundleCreateParams.resourceSourceSettings.basePaths.push_back( basePath );
	}

	std::string chunkDesinationType = m_argumentParser->get<std::string>( m_chunkDestinationTypeArgumentId );

	if( !StringToResourceDestinationType( chunkDesinationType, bundleCreateParams.chunkDestinationSettings.destinationType ) )
	{
		returnErrorMessage = "Invalid chunk destination type";

		return false;
	}

	bundleCreateParams.chunkDestinationSettings.basePath = m_argumentParser->get<std::string>( m_chunkDestinationBasePathArgumentId );

	std::string bundleResourceGroupDestinationType = m_argumentParser->get<std::string>( m_bundleResourceGroupDestinationTypeArgumentId );

	if( !StringToResourceDestinationType( bundleResourceGroupDestinationType, bundleCreateParams.resourceBundleResourceGroupDestinationSettings.destinationType ) )
	{
		returnErrorMessage = "Invalid Resource Group destination type";

		return false;
	}

	bundleCreateParams.resourceBundleResourceGroupDestinationSettings.basePath = m_argumentParser->get<std::string>( m_bundleResourceGroupDestinationBasePathArgumentId );

	try
	{
		bundleCreateParams.chunkSize = std::stoull( m_argumentParser->get( m_chunkSizeArgumentId ) );

        bundleCreateParams.fileReadChunkSize = std::stoull( m_argumentParser->get( m_streamChunkSizeId ) );
		
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
		unsigned long long networkRetryCountUnsignedLongLong = std::stoull( m_argumentParser->get( m_networkRetryCountId ) );

		if( networkRetryCountUnsignedLongLong > std::numeric_limits<uint32_t>::max() )
		{
			return false;
		}

		bundleCreateParams.downloadSettings.retryCount = static_cast<uint32_t>( networkRetryCountUnsignedLongLong );
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

		bundleCreateParams.downloadSettings.retrySeconds = std::chrono::seconds( networkRetryBackoffMultiplierLongLong );
	}
	catch( std::invalid_argument& )
	{
		return false;
	}
	catch( std::out_of_range& )
	{
		return false;
	}

	if( ShowCliStatusUpdates() )
	{
		PrintStartBanner( resourceGroupParams, bundleCreateParams );
	}

	return CreateBundle( resourceGroupParams, bundleCreateParams, returnErrorMessage );
}

void CreateBundleCliOperation::PrintStartBanner( const CarbonResources::ResourceGroupImportFromFileParams& resourceGroupParams, CarbonResources::BundleCreateParams& bundleCreateParams ) const
{
	std::cout << "---Running Bundle Creation---" << std::endl;

	PrintCommonOperationHeaderInformation();

	std::cout << "Resource Group: " << resourceGroupParams.filename << std::endl;

	std::cout << "Resource Group Relative Path: " << bundleCreateParams.resourceGroupRelativePath << std::endl;

	std::cout << "Bundle Resource Group Relative Path: " << bundleCreateParams.resourceGroupBundleRelativePath << std::endl;

	std::cout << "Resource Source Type: " << SourceTypeToString( bundleCreateParams.resourceSourceSettings.sourceType ) << std::endl;

	for( std::filesystem::path basePath : bundleCreateParams.resourceSourceSettings.basePaths )
	{
		std::cout << "Resource Source Base Path: " << basePath << std::endl;
	}

	std::cout << "Chunk Destination Type: " << DestinationTypeToString( bundleCreateParams.chunkDestinationSettings.destinationType ) << std::endl;

	std::cout << "Chunk Destination Base Path: " << bundleCreateParams.chunkDestinationSettings.basePath << std::endl;

	std::cout << "Bundle Resource Group Destination Type: " << DestinationTypeToString( bundleCreateParams.resourceBundleResourceGroupDestinationSettings.destinationType ) << std::endl;

	std::cout << "Bundle Resource Group Destination Base Path: " << bundleCreateParams.resourceBundleResourceGroupDestinationSettings.basePath << std::endl;

	std::cout << "Target Chunk Size: " << bundleCreateParams.chunkSize << " Bytes" << std::endl;

    std::cout << "File stream chunk Size: " << bundleCreateParams.fileReadChunkSize << " Bytes" << std::endl;

    std::cout << "Network retry count: " << bundleCreateParams.downloadSettings.retryCount << std::endl;

	std::cout << "Network retry backoff multiplier ( Seconds ): " << bundleCreateParams.downloadSettings.retrySeconds.count() << std::endl;

	std::cout << "----------------------------\n"
			  << std::endl;
}

bool CreateBundleCliOperation::CreateBundle( CarbonResources::ResourceGroupImportFromFileParams& resourceGroupParams, CarbonResources::BundleCreateParams& bundleCreateParams, std::string& returnErrorMessage ) const
{
	CarbonResources::StatusCallback statusCallback = GetStatusCallback();

	if( !std::filesystem::exists( resourceGroupParams.filename ) )
	{
		returnErrorMessage = resourceGroupParams.filename.string() + " not found";
		return false;
	}

	std::string extension = resourceGroupParams.filename.extension().string();
	std::string inputResourceGroupType;
	if( extension == ".txt" )
	{
		inputResourceGroupType = "ResourceGroup";
	}
	else if( extension == ".yml" || extension == ".yaml" || extension.empty() )
	{
		YAML::Node root;
		try
		{
			root = YAML::LoadFile( resourceGroupParams.filename.string() );
		}
		catch( YAML::ParserException& )
		{
			returnErrorMessage = resourceGroupParams.filename.string() + " does not contain valid YAML.";
			return false;
		}
		YAML::Node typeNode = root["Type"];
		if( !typeNode.IsDefined() )
		{
			returnErrorMessage = "Could not read type from resource group.";
			return false;
		}
		inputResourceGroupType = typeNode.as<std::string>();
	}
	else
	{
		std::stringstream ss;

		ss << "Unexpected file extension for resource group '" << extension << "'.";
		returnErrorMessage = ss.str();
		return false;
	}

	// Import ResourceGroup
	CarbonResources::ResourceGroup* resourceGroup;

	// Note: This information could be ascertained directly from file
	if( inputResourceGroupType == "ResourceGroup" )
	{
		resourceGroup = new CarbonResources::ResourceGroup();
	}
	else if( inputResourceGroupType == "PatchGroup" )
	{
		resourceGroup = new CarbonResources::PatchResourceGroup();
	}
	else
	{
		// Unknown resource group type provided
		returnErrorMessage = "Unexpected resource group type : " + inputResourceGroupType;
		return false;
	}

	resourceGroupParams.callbackSettings.statusCallback = statusCallback;
	resourceGroupParams.callbackSettings.verbosityLevel = GetVerbosityLevel();

    if( ShowCliStatusUpdates() )
	{
		CliStatusUpdate( "Importing Resource Group from file." );
	}

	CarbonResources::Result importResourceGroupResult = resourceGroup->ImportFromFile( resourceGroupParams );

	if( importResourceGroupResult.type != CarbonResources::ResultType::SUCCESS )
	{
		PrintCarbonResourcesError( importResourceGroupResult );

		return false;
	}

    bundleCreateParams.callbackSettings.statusCallback = GetStatusCallback();
	bundleCreateParams.callbackSettings.verbosityLevel = GetVerbosityLevel();

	if( ShowCliStatusUpdates() )
	{
		CliStatusUpdate( "Creating Bundle." );
	}

	CarbonResources::Result createBundleResult = resourceGroup->CreateBundle( bundleCreateParams );

	delete resourceGroup;

	if( createBundleResult.type != CarbonResources::ResultType::SUCCESS )
	{
		PrintCarbonResourcesError( createBundleResult );

		return false;
	}

    if( ShowCliStatusUpdates() )
	{
		CliStatusUpdate( "Operation complete." );
	}

	return true;
}