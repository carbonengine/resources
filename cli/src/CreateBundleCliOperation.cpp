#include "CreateBundleCliOperation.h"

#include <string>
#include <argparse/argparse.hpp>
#include <ResourceGroup.h>

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
	m_chunkSizeArgumentId( "--chunk-size" )
{

	AddRequiredPositionalArgument( m_inputResourceGroupPathArgumentId, "Path to ResourceGroup to bundle." );

	AddArgument( m_resourceGroupRelativePathArgumentId, "Relative path to save a ResourceGroup the Bundle was based off", false, "ResourceGroup.yaml" );

    AddArgument( m_bundleResourceGroupRelativePathArgumentId, "Relative path to save a Bundle ResourceGroup", false, "BundleResourceGroup.yaml" );

    AddArgument( m_resourceSourceTypeArgumentId, "Represents the type of repository where resources will be sourced.", false, "LOCAL_RELATIVE" );

	AddArgument( m_resourceSourceBasePathArgumentId, "Represents the base path where the resources will be sourced.", false, "." );

    AddArgument( m_chunkDestinationTypeArgumentId, "Represents the type of repository where chunks will be saved.", false, "LOCAL_RELATIVE" );

	AddArgument( m_chunkDestinationBasePathArgumentId, "Represents the base path where the chunks will be saved.", false, "BundleOut" );

    AddArgument( m_bundleResourceGroupDestinationTypeArgumentId, "Represents the type of repository where the bundle ResourceGroup will be saved.", false, "LOCAL_RELATIVE" );

	AddArgument( m_bundleResourceGroupDestinationBasePathArgumentId, "Represents the base path where the bundle ResourceGroup will be saved.", false, "." );

    AddArgument( m_chunkSizeArgumentId, "Represents the maximum size of the produced chunks in bytes.", false, "10000000" );
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

	bundleCreateParams.resourceSourceSettings.basePath = m_argumentParser->get<std::string>( m_resourceSourceBasePathArgumentId );

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

	try
	{
		bundleCreateParams.chunkSize = std::stoull( m_argumentParser->get( m_chunkSizeArgumentId ) );
	}
	catch( std::invalid_argument& e )
	{
		return false;
	}
	catch( std::out_of_range& e )
	{
		return false;
	}
	PrintStartBanner( resourceGroupParams, bundleCreateParams );

	return CreateBundle( resourceGroupParams, bundleCreateParams );
}

void CreateBundleCliOperation::PrintStartBanner( const CarbonResources::ResourceGroupImportFromFileParams& resourceGroupParams, CarbonResources::BundleCreateParams bundleCreateParams ) const
{
	if( s_verbosity <= 0 )
	{
		return;
	}

	std::cout << "---Running Bundle Creation---" << std::endl;

    PrintCommonOperationHeaderInformation();

	std::cout << "Resource Group: " << resourceGroupParams.filename << std::endl;

	std::cout << "Resource Group Relative Path: " << bundleCreateParams.resourceGroupRelativePath << std::endl;

    std::cout << "Bundle Resource Group Relative Path: " << bundleCreateParams.resourceGroupBundleRelativePath << std::endl;

    std::cout << "Resource Source Type: " << SourceTypeToString( bundleCreateParams.resourceSourceSettings.sourceType ) << std::endl;

	std::cout << "Resource Source Base Path: " << bundleCreateParams.resourceSourceSettings.basePath << std::endl;

    std::cout << "Chunk Destination Type: " << DestinationTypeToString( bundleCreateParams.chunkDestinationSettings.destinationType ) << std::endl;

	std::cout << "Chunk Destination Base Path: " << bundleCreateParams.chunkDestinationSettings.basePath << std::endl;

    std::cout << "Bundle Resource Group Destination Type: " << DestinationTypeToString( bundleCreateParams.resourceBundleResourceGroupDestinationSettings.destinationType ) << std::endl;

	std::cout << "Bundle Resource Group Destination Base Path: " << bundleCreateParams.resourceBundleResourceGroupDestinationSettings.basePath << std::endl;

    std::cout << "Chunk Size: " << bundleCreateParams.chunkSize << " Bytes" << std::endl;

	std::cout << "----------------------------\n" << std::endl;
}

bool CreateBundleCliOperation::CreateBundle( const CarbonResources::ResourceGroupImportFromFileParams& resourceGroupParams, CarbonResources::BundleCreateParams bundleCreateParams ) const
{
	// Import ResourceGroup
	CarbonResources::ResourceGroup resourceGroup;

    CarbonResources::Result importResourceGroupResult = resourceGroup.ImportFromFile( resourceGroupParams );

    if (importResourceGroupResult.type != CarbonResources::ResultType::SUCCESS)
    {
		PrintCarbonResourcesError( importResourceGroupResult );

        return false;
    }

    bundleCreateParams.statusCallback = GetStatusCallback();

    CarbonResources::Result createBundleResult = resourceGroup.CreateBundle( bundleCreateParams );

    if (createBundleResult.type != CarbonResources::ResultType::SUCCESS)
    {
		PrintCarbonResourcesError( createBundleResult );

        return false;
    }

    return true;
}