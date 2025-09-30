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
	m_createResourceGroupResourcePrefixArgumentId( "--resource-prefix" )
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
}

bool CreateResourceGroupCliOperation::Execute( std::string& returnErrorMessage ) const
{
	std::string inputDirectory = m_argumentParser->get<std::string>( m_createResourceGroupPathArgumentId );

	std::string outputFile = m_argumentParser->get<std::string>( m_createResourceGroupOutputFileArgumentId );

	std::string version = m_argumentParser->get( m_createResourceGroupDocumentVersionArgumentId );

	std::string resourcePrefix = m_argumentParser->get( m_createResourceGroupResourcePrefixArgumentId );

	CarbonResources::Version documentVersion;

	PrintStartBanner( inputDirectory, outputFile, version, resourcePrefix );

	bool versionIsValid = ParseDocumentVersion( version, documentVersion );

	if( !versionIsValid )
	{
		returnErrorMessage = "Invalid document version";

		return false;
	}
	return CreateResourceGroup( inputDirectory, outputFile, documentVersion, resourcePrefix );
}

void CreateResourceGroupCliOperation::PrintStartBanner( const std::filesystem::path& inputDirectory, const std::filesystem::path& outputFile, const std::string& version, const std::string& resourcePrefix ) const
{
	if( s_verbosityLevel == CarbonResources::StatusLevel::OFF )
	{
		return;
	}

	std::cout << "---Creating Resource Group---" << std::endl;

	PrintCommonOperationHeaderInformation();

	std::cout << "Input Directory: " << inputDirectory << std::endl;

	std::cout << "Output File: " << outputFile << std::endl;

	std::cout << "Output Document Version: " << version << std::endl;

	std::cout << "Resource Prefix: " << resourcePrefix << std::endl;

	std::cout << "----------------------------\n"
			  << std::endl;
}

bool CreateResourceGroupCliOperation::CreateResourceGroup( const std::filesystem::path& inputDirectory, const std::filesystem::path& resourceGroupOutputFile, CarbonResources::Version documentVersion, const std::string& resourcePrefix ) const
{
	CarbonResources::ResourceGroup resourceGroup;

	CarbonResources::CreateResourceGroupFromDirectoryParams createResourceGroupParams;

	createResourceGroupParams.directory = inputDirectory;

	createResourceGroupParams.outputDocumentVersion = documentVersion;

	createResourceGroupParams.resourcePrefix = resourcePrefix;

	createResourceGroupParams.statusCallback = GetStatusCallback();


	if( createResourceGroupParams.statusCallback )
	{
		createResourceGroupParams.statusCallback( CarbonResources::StatusLevel::OVERVIEW, CarbonResources::StatusProgressType::PERCENTAGE, 0, "Creating Resource Group from directory" );
	}

	CarbonResources::Result createFromDirectoryResult = resourceGroup.CreateFromDirectory( createResourceGroupParams );

	if( createFromDirectoryResult.type != CarbonResources::ResultType::SUCCESS )
	{
		PrintCarbonResourcesError( createFromDirectoryResult );

		return false;
	}

	CarbonResources::ResourceGroupExportToFileParams exportParams;

	exportParams.filename = resourceGroupOutputFile;

	exportParams.outputDocumentVersion = documentVersion;

	exportParams.statusCallback = GetStatusCallback();

	if( exportParams.statusCallback )
	{
		exportParams.statusCallback( CarbonResources::StatusLevel::OVERVIEW, CarbonResources::StatusProgressType::PERCENTAGE, 50, "Exporting Resource Group to file." );
	}

	CarbonResources::Result exportToFileResult = resourceGroup.ExportToFile( exportParams );

	if( exportToFileResult.type != CarbonResources::ResultType::SUCCESS )
	{
		PrintCarbonResourcesError( exportToFileResult );

		return false;
	}

	if( exportParams.statusCallback )
	{
		exportParams.statusCallback( CarbonResources::StatusLevel::OVERVIEW, CarbonResources::StatusProgressType::PERCENTAGE, 100, "Resource Group successfully created from directory." );
	}

	return true;
}