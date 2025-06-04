#include "CreateBundleCliOperation.h"

#include <string>
#include <argparse/argparse.hpp>
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
	m_downloadRetrySecondsArgumentId( "--download-retry-seconds" ),
	m_resourceGroupType( "--resourcegroup-type" )
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

    AddArgument( m_chunkSizeArgumentId, "Represents the maximum size of the produced chunks in bytes.", false, false, SizeToString( defaultParams.chunkSize ) );

	AddArgument( m_downloadRetrySecondsArgumentId, "The number of seconds before attempt to download a resource fails with a network related error", false, false, SecondsToString( defaultParams.downloadRetrySeconds ) );

    AddArgument( m_resourceGroupType, "Type of resource group supplied", false, false, "ResourceGroup", "ResourceGroup, PatchResourceGroup" );
}

bool CreateBundleCliOperation::Execute( std::string& returnErrorMessage ) const
{
	CarbonResources::ResourceGroupImportFromFileParams resourceGroupParams;

    resourceGroupParams.filename = m_argumentParser->get<std::string>( m_inputResourceGroupPathArgumentId );

    CarbonResources::BundleCreateParams bundleCreateParams;

	bundleCreateParams.resourceGroupRelativePath = m_argumentParser->get<std::string>( m_resourceGroupRelativePathArgumentId );

	bundleCreateParams.resourceGroupBundleRelativePath = m_argumentParser->get<std::string>( m_bundleResourceGroupRelativePathArgumentId );

	std::string resourceSourceType = m_argumentParser->get(m_resourceSourceTypeArgumentId);

    if( !StringToResourceSourceType( resourceSourceType, bundleCreateParams.resourceSourceSettings.sourceType ) )
    {
		returnErrorMessage = "Invalid resources source type";

		return false;
    }

    for (const std::string basePath : m_argumentParser->get<std::vector<std::string>>(m_resourceSourceBasePathArgumentId))
    {
		bundleCreateParams.resourceSourceSettings.basePaths.push_back(basePath);
    }

	std::string chunkDesinationType = m_argumentParser->get<std::string>( m_chunkDestinationTypeArgumentId );

    if (!StringToResourceDestinationType(chunkDesinationType, bundleCreateParams.chunkDestinationSettings.destinationType))
    {
		returnErrorMessage = "Invalid chunk destination type";

		return false;
    }

	bundleCreateParams.chunkDestinationSettings.basePath = m_argumentParser->get<std::string>( m_chunkDestinationBasePathArgumentId );

	std::string bundleResourceGroupDestinationType = m_argumentParser->get<std::string>( m_bundleResourceGroupDestinationTypeArgumentId );

    if (!StringToResourceDestinationType(bundleResourceGroupDestinationType, bundleCreateParams.resourceBundleResourceGroupDestinationSettings.destinationType))
    {
		returnErrorMessage = "Invalid Resource Group destination type";

		return false;
    }

	bundleCreateParams.resourceBundleResourceGroupDestinationSettings.basePath = m_argumentParser->get<std::string>( m_bundleResourceGroupDestinationBasePathArgumentId );

	long long retrySeconds{120};
	try
	{
		bundleCreateParams.chunkSize = std::stoull( m_argumentParser->get( m_chunkSizeArgumentId ) );
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
	bundleCreateParams.downloadRetrySeconds = std::chrono::seconds( retrySeconds );

	PrintStartBanner( resourceGroupParams, bundleCreateParams );

	return CreateBundle( resourceGroupParams, bundleCreateParams );
}

void CreateBundleCliOperation::PrintStartBanner( const CarbonResources::ResourceGroupImportFromFileParams& resourceGroupParams, CarbonResources::BundleCreateParams bundleCreateParams ) const
{
	if( s_verbosityLevel == CarbonResources::STATUS_LEVEL::OFF )
	{
		return;
	}

	std::cout << "---Running Bundle Creation---" << std::endl;

    PrintCommonOperationHeaderInformation();

	std::cout << "Resource Group: " << resourceGroupParams.filename << std::endl;

	std::cout << "Resource Group Relative Path: " << bundleCreateParams.resourceGroupRelativePath << std::endl;

    std::cout << "Bundle Resource Group Relative Path: " << bundleCreateParams.resourceGroupBundleRelativePath << std::endl;

    std::cout << "Resource Source Type: " << SourceTypeToString( bundleCreateParams.resourceSourceSettings.sourceType ) << std::endl;

    for (std::filesystem::path basePath : bundleCreateParams.resourceSourceSettings.basePaths)
    {
		std::cout << "Resource Source Base Path: " << basePath << std::endl;
    }

    std::cout << "Chunk Destination Type: " << DestinationTypeToString( bundleCreateParams.chunkDestinationSettings.destinationType ) << std::endl;

	std::cout << "Chunk Destination Base Path: " << bundleCreateParams.chunkDestinationSettings.basePath << std::endl;

    std::cout << "Bundle Resource Group Destination Type: " << DestinationTypeToString( bundleCreateParams.resourceBundleResourceGroupDestinationSettings.destinationType ) << std::endl;

	std::cout << "Bundle Resource Group Destination Base Path: " << bundleCreateParams.resourceBundleResourceGroupDestinationSettings.basePath << std::endl;

    std::cout << "Chunk Size: " << bundleCreateParams.chunkSize << " Bytes" << std::endl;

	std::cout << "Download Retry Seconds: " << bundleCreateParams.downloadRetrySeconds.count() << std::endl;

	std::cout << "----------------------------\n" << std::endl;
}

bool CreateBundleCliOperation::CreateBundle( CarbonResources::ResourceGroupImportFromFileParams& resourceGroupParams, CarbonResources::BundleCreateParams bundleCreateParams ) const
{
	CarbonResources::StatusCallback statusCallback = GetStatusCallback();

    if( statusCallback )
	{
		statusCallback( CarbonResources::STATUS_LEVEL::OVERVIEW, CarbonResources::STATUS_PROGRESS_TYPE::PERCENTAGE, 0, "Creating Bundle." );
	}

	// Import ResourceGroup
	std::string inputResourceGroupType = m_argumentParser->get<std::string>( m_resourceGroupType );

    CarbonResources::ResourceGroup* resourceGroup;

	// Note: This information could be ascertained directly from file
    if (inputResourceGroupType == "ResourceGroup")
    {
		resourceGroup = new CarbonResources::ResourceGroup();
    }
    else if (inputResourceGroupType == "PatchResourceGroup")
    {
		resourceGroup = new CarbonResources::PatchResourceGroup();
    }
    else
    {
        // Unknown resource group type provided
		return false;
    }

    resourceGroupParams.statusCallback = statusCallback;

    CarbonResources::Result importResourceGroupResult = resourceGroup->ImportFromFile( resourceGroupParams );

    if (importResourceGroupResult.type != CarbonResources::ResultType::SUCCESS)
    {
		PrintCarbonResourcesError( importResourceGroupResult );

        return false;
    }

    bundleCreateParams.statusCallback = GetStatusCallback();

    if( statusCallback )
	{
		statusCallback( CarbonResources::STATUS_LEVEL::OVERVIEW, CarbonResources::STATUS_PROGRESS_TYPE::PERCENTAGE, 50, "Processing bundle files." );
	}

    CarbonResources::Result createBundleResult = resourceGroup->CreateBundle( bundleCreateParams );

    delete resourceGroup;

    if (createBundleResult.type != CarbonResources::ResultType::SUCCESS)
    {
		PrintCarbonResourcesError( createBundleResult );

        return false;
    }

    if( statusCallback )
	{
		statusCallback( CarbonResources::STATUS_LEVEL::OVERVIEW, CarbonResources::STATUS_PROGRESS_TYPE::PERCENTAGE, 100, "Bundle created succesfully" );
	}

    return true;
}