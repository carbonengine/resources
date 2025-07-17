// Copyright © 2025 CCP ehf.

#include "UnpackBundleCliOperation.h"

#include <iostream>
#include <argparse/argparse.hpp>

UnpackBundleCliOperation::UnpackBundleCliOperation() :
	CliOperation("unpack-bundle", "Extracts a bundle to original files given a Bundle Resource Group and a source for chunks [Only available in extended feature development build]"),
	m_bundleResourceGroupPathArgumentId("bundle-resource-group-path"),
	m_chunkSourceBasePathsArgumentId("--chunk-source-base-path"),
	m_chunkSourceTypeArgumentId("--chunk-source-type"),
	m_resourceDestinationBasePathArgumentId("--resource-destination-base-path"),
	m_resourceDestinationTypeArgumentId("--resource-destination-type")
{
    AddRequiredPositionalArgument( m_bundleResourceGroupPathArgumentId, "The path to the BundleResourceGroup.yaml file" );

    CarbonResources::BundleUnpackParams defaultParams;

	AddArgument( m_chunkSourceBasePathsArgumentId, "The path to the directory containing the bundled files.", true, true, PathsToString( defaultParams.chunkSourceSettings.basePaths ) );

	AddArgument( m_chunkSourceTypeArgumentId, "The type of repository from which to retrieve the bundle files.", false, false, SourceTypeToString( defaultParams.chunkSourceSettings.sourceType ), ResourceSourceTypeChoicesAsString());

	AddArgument( m_resourceDestinationBasePathArgumentId,"The path to the directory in which to place the unbundled files.", false, false, "UnpackBundleOut");

	AddArgument( m_resourceDestinationTypeArgumentId, "The type of repository in which to place the bundle files.", false, false, DestinationTypeToString( defaultParams.resourceDestinationSettings.destinationType ), ResourceDestinationTypeChoicesAsString() );
}

bool UnpackBundleCliOperation::Execute( std::string& returnErrorMessage ) const
{

	CarbonResources::ResourceGroupImportFromFileParams importParams;

	std::optional<std::string> name = m_argumentParser->present( m_bundleResourceGroupPathArgumentId );
	if( !name.has_value() )
	{
		returnErrorMessage = "Failed to parse bundle resource group path";

		return false;
	}
	importParams.filename = name.value();

    // Unpack the bundle
	CarbonResources::BundleUnpackParams unpackParams;

	std::string chunkSourceType = m_argumentParser->get( m_chunkSourceTypeArgumentId );
	if( !StringToResourceSourceType( chunkSourceType, unpackParams.chunkSourceSettings.sourceType ) )
	{
		returnErrorMessage = "Invalid chunk source type";

		return false;
	}

	auto chunkSourceBasePathStrings = m_argumentParser->present<std::vector<std::string>>( m_chunkSourceBasePathsArgumentId );
	if( !chunkSourceBasePathStrings.has_value() )
	{
		returnErrorMessage = "Failed to parse chunk source path";

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
		returnErrorMessage = "Invalid resource destination type";

		return false;
	}

	unpackParams.resourceDestinationSettings.basePath = m_argumentParser->get( m_resourceDestinationBasePathArgumentId );

	PrintStartBanner( importParams, unpackParams );

	return Unpack( importParams, unpackParams );
}

void UnpackBundleCliOperation::PrintStartBanner( const CarbonResources::ResourceGroupImportFromFileParams& importParams, const CarbonResources::BundleUnpackParams& unpackParams ) const
{
	if( s_verbosityLevel == CarbonResources::StatusLevel::OFF )
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

bool UnpackBundleCliOperation::Unpack( CarbonResources::ResourceGroupImportFromFileParams& importParams, CarbonResources::BundleUnpackParams& unpackParams ) const
{
	CarbonResources::StatusCallback statusCallback = GetStatusCallback();

    if( statusCallback )
	{
		statusCallback( CarbonResources::StatusLevel::OVERVIEW, CarbonResources::StatusProgressType::PERCENTAGE, 0, "Unpacking Bundle." );
	}

	// Load the bundle file
	CarbonResources::BundleResourceGroup bundleResourceGroup;

    importParams.statusCallback = statusCallback;

	if( bundleResourceGroup.ImportFromFile( importParams).type != CarbonResources::ResultType::SUCCESS )
	{
		return false;
	}

    if( statusCallback )
	{
		statusCallback( CarbonResources::StatusLevel::OVERVIEW, CarbonResources::StatusProgressType::PERCENTAGE, 50, "Unpacking" );
	}

    unpackParams.statusCallback = statusCallback;

	auto unpackResult = bundleResourceGroup.Unpack( unpackParams );

    if( unpackResult.type != CarbonResources::ResultType::SUCCESS)
	{
    	std::string out;
    	CarbonResources::ResultTypeToString( unpackResult.type, out );
    	std::cerr << "Failed to unpack bundle: " << out << std::endl;
    	if( s_verbosityLevel >= CarbonResources::StatusLevel::DETAIL && !unpackResult.info.empty() )
    	{
    		std::cerr << unpackResult.info;
    	}
    	exit(1);
	}

    if( statusCallback )
	{
		statusCallback( CarbonResources::StatusLevel::OVERVIEW, CarbonResources::StatusProgressType::PERCENTAGE, 0, "Successfully unpacked Bundle." );
	}

	return true;
}