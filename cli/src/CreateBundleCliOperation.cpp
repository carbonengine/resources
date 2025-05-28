#include "CreateBundleCliOperation.h"

#include <string>
#include <argparse/argparse.hpp>
#include <ResourceGroup.h>
#include <PatchResourceGroup.h>

CreateBundleCliOperation::CreateBundleCliOperation() :
	CliOperation( "create-bundle", "Creates bundle from supplied ResourceGroup." ),
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

	AddArgument( m_resourceGroupRelativePathArgumentId, "Relative path to save a ResourceGroup the Bundle was based off", false, false, "ResourceGroup.yaml" );

    AddArgument( m_bundleResourceGroupRelativePathArgumentId, "Relative path to save a Bundle ResourceGroup", false, false, "BundleResourceGroup.yaml" );

    AddArgument( m_resourceSourceTypeArgumentId, "Represents the type of repository where resources will be sourced.", false, false, "LOCAL_RELATIVE" );

	AddArgument( m_resourceSourceBasePathArgumentId, "Represents the base path where the resources will be sourced.", false, true, "." );

    AddArgument( m_chunkDestinationTypeArgumentId, "Represents the type of repository where chunks will be saved.", false, false, "LOCAL_RELATIVE" );

	AddArgument( m_chunkDestinationBasePathArgumentId, "Represents the base path where the chunks will be saved.", false, false, "BundleOut" );

    AddArgument( m_bundleResourceGroupDestinationTypeArgumentId, "Represents the type of repository where the bundle ResourceGroup will be saved.", false, false, "LOCAL_RELATIVE" );

	AddArgument( m_bundleResourceGroupDestinationBasePathArgumentId, "Represents the base path where the bundle ResourceGroup will be saved.", false, false, "." );

    AddArgument( m_chunkSizeArgumentId, "Represents the maximum size of the produced chunks in bytes.", false, false, "10000000" );

	AddArgument( m_downloadRetrySecondsArgumentId, "The number of seconds before attempt to download a resource fails with a network related error", false, false, "120");

    AddArgument( m_resourceGroupType, "Type of resource group supplied", false, false, "ResourceGroup" );
}

bool CreateBundleCliOperation::Execute() const
{
	CarbonResources::ResourceGroupImportFromFileParams resourceGroupParams;

    resourceGroupParams.filename = m_argumentParser->get<std::string>( m_inputResourceGroupPathArgumentId );

    CarbonResources::BundleCreateParams bundleCreateParams;

	bundleCreateParams.resourceGroupRelativePath = m_argumentParser->get<std::string>( m_resourceGroupRelativePathArgumentId );

	bundleCreateParams.resourceGroupBundleRelativePath = m_argumentParser->get<std::string>( m_bundleResourceGroupRelativePathArgumentId );

	std::string resourceSourceType = m_argumentParser->get(m_resourceSourceTypeArgumentId);

	if( resourceSourceType == "LOCAL_CDN" )
	{
		bundleCreateParams.resourceSourceSettings.sourceType = CarbonResources::ResourceSourceType::LOCAL_CDN;
	}
	else if( resourceSourceType == "REMOTE_CDN" )
	{
		bundleCreateParams.resourceSourceSettings.sourceType = CarbonResources::ResourceSourceType::REMOTE_CDN;
	}
	else if( resourceSourceType == "LOCAL_RELATIVE" )
	{
		bundleCreateParams.resourceSourceSettings.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;
	}
	else
	{
		return false;
	}

    for (const std::string basePath : m_argumentParser->get<std::vector<std::string>>(m_resourceSourceBasePathArgumentId))
    {
		bundleCreateParams.resourceSourceSettings.basePaths.push_back(basePath);
    }

	std::string chunkDesinationType = m_argumentParser->get<std::string>( m_chunkDestinationTypeArgumentId );

	if( chunkDesinationType == "LOCAL_CDN" )
	{
		bundleCreateParams.chunkDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_CDN;
	}
	else if( chunkDesinationType == "REMOTE_CDN" )
	{
		bundleCreateParams.chunkDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::REMOTE_CDN;
	}
	else if( chunkDesinationType == "LOCAL_RELATIVE" )
	{
		bundleCreateParams.chunkDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_RELATIVE;
	}
	else
	{
		return false;
	}

	bundleCreateParams.chunkDestinationSettings.basePath = m_argumentParser->get<std::string>( m_chunkDestinationBasePathArgumentId );

	std::string bundleResourceGroupDestinationType = m_argumentParser->get<std::string>( m_bundleResourceGroupDestinationTypeArgumentId );

	if( bundleResourceGroupDestinationType == "LOCAL_CDN" )
	{
		bundleCreateParams.resourceBundleResourceGroupDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_CDN;
	}
	else if( bundleResourceGroupDestinationType == "REMOTE_CDN" )
	{
		bundleCreateParams.resourceBundleResourceGroupDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::REMOTE_CDN;
	}
	else if( bundleResourceGroupDestinationType == "LOCAL_RELATIVE" )
	{
		bundleCreateParams.resourceBundleResourceGroupDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_RELATIVE;
	}
	else
	{
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
	if( s_verbosityLevel <= 0 )
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

bool CreateBundleCliOperation::CreateBundle( const CarbonResources::ResourceGroupImportFromFileParams& resourceGroupParams, CarbonResources::BundleCreateParams bundleCreateParams ) const
{
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

    CarbonResources::Result importResourceGroupResult = resourceGroup->ImportFromFile( resourceGroupParams );

    if (importResourceGroupResult.type != CarbonResources::ResultType::SUCCESS)
    {
		PrintCarbonResourcesError( importResourceGroupResult );

        return false;
    }

    bundleCreateParams.statusCallback = GetStatusCallback();

    CarbonResources::Result createBundleResult = resourceGroup->CreateBundle( bundleCreateParams );

    delete resourceGroup;

    if (createBundleResult.type != CarbonResources::ResultType::SUCCESS)
    {
		PrintCarbonResourcesError( createBundleResult );

        return false;
    }

    return true;
}