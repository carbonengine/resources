#include "UnpackBundleCliOperation.h"

#include <iostream>
#include <argparse/argparse.hpp>

UnpackBundleCliOperation::UnpackBundleCliOperation() :
	CliOperation("unpack-bundle", "Extract a resource group from a bundle"),
	m_bundleResourceGroupPathArgumentId("--bundle-resource-group-path"),
	m_chunkSourceBasePathsArgumentId("--chunk-source-base-path"),
	m_chunkSourceTypeArgumentId("--chunk-source-type"),
	m_resourceDestinationBasePathArgumentId("--resource-destination-base-path"),
	m_resourceDestinationTypeArgumentId("--resource-destination-type")
{
	AddArgument( m_bundleResourceGroupPathArgumentId, "The path to the BundleResourceGroup.yaml file", true);
	AddArgument( m_chunkSourceBasePathsArgumentId,"The path to the directory containing the bundled files.", true, true);
	AddArgument( m_chunkSourceTypeArgumentId,"The type of repository from which to retrieve the bundle files.", false, false, "LOCAL_RELATIVE");
	AddArgument( m_resourceDestinationBasePathArgumentId,"The path to the directory in which to place the unbundled files.", false, false, "UnpackBundleOut");
	AddArgument( m_resourceDestinationTypeArgumentId,"The type of repository in which to place the bundle files.", false, false, "LOCAL_RELATIVE");
}

bool UnpackBundleCliOperation::Execute() const
{

	CarbonResources::ResourceGroupImportFromFileParams importParams;

	std::optional<std::string> name = m_argumentParser->present( m_bundleResourceGroupPathArgumentId );
	if( !name.has_value() )
	{
		return false;
	}
	importParams.filename = name.value();

    // Unpack the bundle
	CarbonResources::BundleUnpackParams unpackParams;

	std::string chunkSourceType = m_argumentParser->get( m_chunkSourceTypeArgumentId );
	if( !StringToResourceSourceType( chunkSourceType, unpackParams.chunkSourceSettings.sourceType ) )
	{
		return false;
	}

	auto chunkSourceBasePathStrings = m_argumentParser->present<std::vector<std::string>>( m_chunkSourceBasePathsArgumentId );
	if( !chunkSourceBasePathStrings.has_value() )
	{
		return false;
	}
	std::vector<std::filesystem::path> chunkSourceBasePaths;
	for( const auto& path : chunkSourceBasePathStrings.value() )
	{
		chunkSourceBasePaths.push_back( path );
	}

	unpackParams.chunkSourceSettings.basePaths = chunkSourceBasePaths;

	std::string resourceDestinationType = m_argumentParser->get( m_resourceDestinationTypeArgumentId );
	if( !StringToResourceDestinationType( resourceDestinationType, unpackParams.resourceDestinationSettings.destinationType ) )
	{
		return false;
	}

	unpackParams.resourceDestinationSettings.basePath = m_argumentParser->get( m_resourceDestinationBasePathArgumentId );

	PrintStartBanner( importParams, unpackParams );
	return Unpack( importParams, unpackParams );
}

void UnpackBundleCliOperation::PrintStartBanner( const CarbonResources::ResourceGroupImportFromFileParams& importParams, const CarbonResources::BundleUnpackParams& unpackParams ) const
{
	if( s_verbosity <= 0 )
	{
		return;
	}

	std::cout << "---Unpacking Bundle---" << std::endl;

    PrintCommonOperationHeaderInformation();

	std::cout << "Bundle Resource Group Path:" << importParams.filename << std::endl;
	std::cout << "Chunk Source Base Paths: " << PathsToString( unpackParams.chunkSourceSettings.basePaths ) << std::endl;
	std::cout << "Chunk Source Type: " << SourceTypeToString( unpackParams.chunkSourceSettings.sourceType ) << std::endl;
	std::cout << "Resource Destination Base Path: " << unpackParams.resourceDestinationSettings.basePath << std::endl;
	std::cout << "Resource Destination Type: " << DestinationTypeToString( unpackParams.resourceDestinationSettings.destinationType ) << std::endl;

	std::cout << "----------------------------\n" << std::endl;
}

bool UnpackBundleCliOperation::Unpack( const CarbonResources::ResourceGroupImportFromFileParams& importParams, const CarbonResources::BundleUnpackParams& unpackParams ) const
{
	// Load the bundle file
	CarbonResources::BundleResourceGroup bundleResourceGroup;

	if( bundleResourceGroup.ImportFromFile( importParams).type != CarbonResources::ResultType::SUCCESS )
	{
		return false;
	}

	auto unpackResult = bundleResourceGroup.Unpack( unpackParams );
	return unpackResult.type == CarbonResources::ResultType::SUCCESS;
}