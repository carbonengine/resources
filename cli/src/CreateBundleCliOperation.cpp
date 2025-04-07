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

    // TODO: The interface here is totally WIP, it needs actually designing to be easy to use.
    // This is linked to having sensible default values so that not all options are always required and large complex commands are opt in.
	AddRequiredPositionalArgument( m_inputResourceGroupPathArgumentId, "Path to ResourceGroup to bundle." );

	AddArgument( m_resourceGroupRelativePathArgumentId, "-r", "Relative path to save a ResourceGroup the Bundle was based off", true, "ResourceGroup.yaml" );

    AddArgument( m_bundleResourceGroupRelativePathArgumentId, "-br", "Relative path to save a Bundle ResourceGroup", true, "BundleResourceGroup.yaml" );

    AddArgument( m_resourceSourceTypeArgumentId, "-rt", "Represents the type of repository where resources will be sourced.", true, "LOCAL_RELATIVE" );

	AddArgument( m_resourceSourceBasePathArgumentId, "-rb", "Represents the base path where the resources will be sourced.", true, "" );

    AddArgument( m_chunkDestinationTypeArgumentId, "-ct", "Represents the type of repository where chunks will be saved.", true, "LOCAL_RELATIVE" );

	AddArgument( m_chunkDestinationBasePathArgumentId, "-cb", "Represents the base path where the chunks will be saved.", true, "" );

    AddArgument( m_bundleResourceGroupDestinationTypeArgumentId, "-brt", "Represents the type of repository where the bundle ResourceGroup will be saved.", true, "LOCAL_RELATIVE" );

	AddArgument( m_bundleResourceGroupDestinationBasePathArgumentId, "-brp", "Represents the base path where the bundle ResourceGroup will be saved.", true, "" );

    AddArgument( m_chunkSizeArgumentId, "-brp", "Represents the base path where the bundle ResourceGroup will be saved.", true, "10000000" );
}

bool CreateBundleCliOperation::Execute() const
{
	CarbonResources::ResourceGroupImportFromFileParams resourceGroupParams;

    resourceGroupParams.filename = m_argumentParser->get<std::string>( m_inputResourceGroupPathArgumentId );

    CarbonResources::BundleCreateParams bundleCreateParams;

    bundleCreateParams.resourceGroupRelativePath = m_argumentParser->get<std::string>( m_resourceGroupRelativePathArgumentId );

    bundleCreateParams.resourceGroupBundleRelativePath = m_argumentParser->get<std::string>( m_bundleResourceGroupRelativePathArgumentId );

    std::string resourceSourceType = m_argumentParser->get<std::string>( m_resourceSourceTypeArgumentId );

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

    bundleCreateParams.chunkSize = m_argumentParser->get<uintmax_t>( m_chunkSizeArgumentId );

	return CreateBundle( resourceGroupParams, bundleCreateParams );
}

bool CreateBundleCliOperation::CreateBundle( const CarbonResources::ResourceGroupImportFromFileParams& resourceGroupParams, CarbonResources::BundleCreateParams bundleCreateParams ) const
{
	// Import ResourceGroup
	CarbonResources::ResourceGroup resourceGroup;

    CarbonResources::Result importResourceGroupResult = resourceGroup.ImportFromFile( resourceGroupParams );

    if (importResourceGroupResult != CarbonResources::Result::SUCCESS)
    {
		PrintCarbonResourcesError( importResourceGroupResult );

        return false;
    }

    CarbonResources::Result createBundleResult = resourceGroup.CreateBundle( bundleCreateParams );

    if (createBundleResult != CarbonResources::Result::SUCCESS)
    {
		PrintCarbonResourcesError( createBundleResult );

        return false;
    }

    return true;
}