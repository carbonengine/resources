// Copyright © 2025 CCP ehf.

#include "CreateResourceGroupCliOperation.h"

#include <string>
#include <argparse/argparse.hpp>
#include <ResourceGroup.h>

CreateResourceGroupCliOperation::CreateResourceGroupCliOperation() :
	CliOperation( "create-group", "Create a Resource Group from a given directory." ),
	m_createResourceGroupPathArgumentId( "input-directory" ),
	m_createResourceGroupOutputFileArgumentId( "--output-file" ),
	m_createResourceGroupDocumentVersionArgumentId( "--document-version" ),
	m_createResourceGroupResourcePrefixArgumentId( "--resource-prefix" ),
	m_createResourceGroupSkipCompressionCalculationId( "--skip-compression" ),
	m_createResourceGroupExportResourcesId( "--export-resources" ),
	m_createResourceGroupExportResourcesDestinationTypeId( "--export-resources-destination-type" ),
	m_createResourceGroupExportResourcesDestinationPathId( "--export-resources-destination-path" )
{

	AddRequiredPositionalArgument( m_createResourceGroupPathArgumentId, "Base directory to create resource group from." );

	// Struct is inspected to ascertain default values
	// This keeps default value settings in one place
	// Lib defaults matches CLI
	CarbonResources::CreateResourceGroupFromDirectoryParams defaultImportParams;

	CarbonResources::ResourceGroupExportToFileParams defaultExportParams;

	AddArgument( m_createResourceGroupOutputFileArgumentId, "Filename for created resource group.", false, false, defaultExportParams.filename.string() );

	AddArgument( m_createResourceGroupDocumentVersionArgumentId, "Document version for created resource group.", false, false, VersionToString( defaultImportParams.outputDocumentVersion ) );

	AddArgument( m_createResourceGroupResourcePrefixArgumentId, R"(Optional resource path prefix, such as "res" or "app")", false, false, "" );
	
    AddArgumentFlag( m_createResourceGroupSkipCompressionCalculationId, "Set skip compression calculations on resources." );

    AddArgumentFlag( m_createResourceGroupExportResourcesId, "Export resources after processing. see --export-resources-destination-type and --export-resources-destination-path" );

    AddArgument( m_createResourceGroupExportResourcesDestinationTypeId, "Represents the type of repository where exported resources will be saved. Requires --export-resources", false, false, DestinationTypeToString( defaultImportParams.exportResourcesDestinationSettings.destinationType ), ResourceDestinationTypeChoicesAsString() );

	AddArgument( m_createResourceGroupExportResourcesDestinationPathId, "Represents the base path where the exported resources will be saved. Requires --export-resources", false, false, defaultImportParams.exportResourcesDestinationSettings.basePath.string() );
}

bool CreateResourceGroupCliOperation::Execute( std::string& returnErrorMessage ) const
{
	CarbonResources::CreateResourceGroupFromDirectoryParams createResourceGroupParams;

    CarbonResources::ResourceGroupExportToFileParams exportParams;

    createResourceGroupParams.directory = m_argumentParser->get<std::string>( m_createResourceGroupPathArgumentId );

    bool versionIsValid = ParseDocumentVersion( m_argumentParser->get( m_createResourceGroupDocumentVersionArgumentId ), createResourceGroupParams.outputDocumentVersion );

	if( !versionIsValid )
	{
		returnErrorMessage = "Invalid document version";

		return false;
	}

	createResourceGroupParams.resourcePrefix = m_argumentParser->get( m_createResourceGroupResourcePrefixArgumentId );

	createResourceGroupParams.calculateCompressions = !m_argumentParser->get<bool>( m_createResourceGroupSkipCompressionCalculationId );

    createResourceGroupParams.exportResources = m_argumentParser->get<bool>( m_createResourceGroupExportResourcesId );

	if( createResourceGroupParams.exportResources )
	{
		std::string exportResourcesDesinationType = m_argumentParser->get<std::string>( m_createResourceGroupExportResourcesDestinationTypeId );

		if( !StringToResourceDestinationType( exportResourcesDesinationType, createResourceGroupParams.exportResourcesDestinationSettings.destinationType ) )
		{
			returnErrorMessage = "Invalid chunk destination type";

			return false;
		}

		createResourceGroupParams.exportResourcesDestinationSettings.basePath = m_argumentParser->get<std::string>( m_createResourceGroupExportResourcesDestinationPathId );
	}


    exportParams.filename = m_argumentParser->get<std::string>( m_createResourceGroupOutputFileArgumentId );

    exportParams.outputDocumentVersion = createResourceGroupParams.outputDocumentVersion;

	PrintStartBanner( createResourceGroupParams, exportParams );

	return CreateResourceGroup( createResourceGroupParams, exportParams );
}

void CreateResourceGroupCliOperation::PrintStartBanner( CarbonResources::CreateResourceGroupFromDirectoryParams& createResourceGroupFromDirectoryParams, CarbonResources::ResourceGroupExportToFileParams& ResourceGroupExportToFileParams ) const
{
	if( s_verbosityLevel == CarbonResources::StatusLevel::OFF )
	{
		return;
	}

	std::cout << "---Creating Resource Group---" << std::endl;

	PrintCommonOperationHeaderInformation();

	std::cout << "Input Directory: " << createResourceGroupFromDirectoryParams.directory << std::endl;

	std::cout << "Output File: " << ResourceGroupExportToFileParams.filename << std::endl;

	std::cout << "Output Document Version: " << VersionToString(ResourceGroupExportToFileParams.outputDocumentVersion) << std::endl;

	std::cout << "Resource Prefix: " << createResourceGroupFromDirectoryParams.resourcePrefix << std::endl;

    if( createResourceGroupFromDirectoryParams.calculateCompressions)
    {
		std::cout << "Calculate Compression: On" << std::endl;
    }
	else
	{
		std::cout << "Calculate Compression: Off" << std::endl;
	}

    if( createResourceGroupFromDirectoryParams.exportResources )
	{
		std::cout << "Export Resources: On" << std::endl;

        std::cout << "Export Resources Type: " << DestinationTypeToString( createResourceGroupFromDirectoryParams.exportResourcesDestinationSettings.destinationType ) << std::endl;

        std::cout << "Export Resources Base Path: " << createResourceGroupFromDirectoryParams.exportResourcesDestinationSettings.basePath << std::endl;
	}
	else
	{
		std::cout << "Export Resources: Off" << std::endl;
	}

	std::cout << "----------------------------\n"
			  << std::endl;
}

bool CreateResourceGroupCliOperation::CreateResourceGroup( CarbonResources::CreateResourceGroupFromDirectoryParams& createResourceGroupFromDirectoryParams, CarbonResources::ResourceGroupExportToFileParams& ResourceGroupExportToFileParams ) const
{
	CarbonResources::ResourceGroup resourceGroup;

    createResourceGroupFromDirectoryParams.statusCallback = GetStatusCallback();

	if( createResourceGroupFromDirectoryParams.statusCallback )
	{
		createResourceGroupFromDirectoryParams.statusCallback( CarbonResources::StatusLevel::OVERVIEW, CarbonResources::StatusProgressType::PERCENTAGE, 0, "Creating Resource Group from directory" );
	}

	CarbonResources::Result createFromDirectoryResult = resourceGroup.CreateFromDirectory( createResourceGroupFromDirectoryParams );

	if( createFromDirectoryResult.type != CarbonResources::ResultType::SUCCESS )
	{
		PrintCarbonResourcesError( createFromDirectoryResult );

		return false;
	}

	ResourceGroupExportToFileParams.statusCallback = GetStatusCallback();

	if( ResourceGroupExportToFileParams.statusCallback )
	{
		ResourceGroupExportToFileParams.statusCallback( CarbonResources::StatusLevel::OVERVIEW, CarbonResources::StatusProgressType::PERCENTAGE, 50, "Exporting Resource Group to file." );
	}

	CarbonResources::Result exportToFileResult = resourceGroup.ExportToFile( ResourceGroupExportToFileParams );

	if( exportToFileResult.type != CarbonResources::ResultType::SUCCESS )
	{
		PrintCarbonResourcesError( exportToFileResult );

		return false;
	}

	if( ResourceGroupExportToFileParams.statusCallback )
	{
		ResourceGroupExportToFileParams.statusCallback( CarbonResources::StatusLevel::OVERVIEW, CarbonResources::StatusProgressType::PERCENTAGE, 100, "Resource Group successfully created from directory." );
	}

	return true;
}