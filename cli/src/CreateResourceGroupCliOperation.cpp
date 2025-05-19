#include "CreateResourceGroupCliOperation.h"

#include <string>
#include <argparse/argparse.hpp>
#include <ResourceGroup.h>

CreateResourceGroupCliOperation::CreateResourceGroupCliOperation() :
	CliOperation( "create-group", "Create a resource group from a given directory." ),
	m_createResourceGroupPathArgumentId( "input-directory" ),
	m_createResourceGroupOutputFileArgumentId( "--output-file" ),
	m_createResourceGroupDocumentVersionArgumentId( "--document-version" ),
	m_createResourceGroupResourcePrefixArgumentId("--resource-prefix")
{

	AddRequiredPositionalArgument( m_createResourceGroupPathArgumentId, "Base directory to create resource group from." );

	AddArgument( m_createResourceGroupOutputFileArgumentId, "Filename for created resource group.", false, false, "ResourceGroup.yaml" );
	AddArgument( m_createResourceGroupDocumentVersionArgumentId, "Document version for created resource group.", false, false, "0.1.0" );
	AddArgument( m_createResourceGroupResourcePrefixArgumentId, R"(Blue resource path prefix, such as "res" or "app")", false, false, "" );
}

bool ParseDocumentVersion( const std::string& version, CarbonResources::Version& documentVersion )
{
	try
	{
		auto first = version.find( "." );
		documentVersion.major = std::stoul( version.substr( 0, first ) );
		auto second = version.find( ".", first + 1 );
		documentVersion.minor = std::stoul( version.substr( first + 1, second - first ) );
		documentVersion.patch = std::stoul( version.substr( second + 1 ) );
	}
	catch( std::invalid_argument& )
	{
		return false;
	}
	catch( std::out_of_range& )
	{
		return false;
	}
	return true;
}

bool CreateResourceGroupCliOperation::Execute() const
{
	std::string inputDirectory = m_argumentParser->get<std::string>( m_createResourceGroupPathArgumentId );

	std::string outputFile = m_argumentParser->get<std::string>( m_createResourceGroupOutputFileArgumentId );

	std::string version = m_argumentParser->get( m_createResourceGroupDocumentVersionArgumentId );

	std::string resourcePrefix = m_argumentParser->get( m_createResourceGroupResourcePrefixArgumentId );

	CarbonResources::Version documentVersion;

	PrintStartBanner( inputDirectory, outputFile, version, resourcePrefix );

	bool versionIsValid = ParseDocumentVersion( version, documentVersion);
	if( !versionIsValid )
	{
		std::cout << "Invalid Document Version: " << version << std::endl;
		return false;
	}
	return CreateResourceGroup( inputDirectory, outputFile, documentVersion, resourcePrefix );
}

void CreateResourceGroupCliOperation::PrintStartBanner( const std::filesystem::path& inputDirectory, const std::filesystem::path& outputFile, const std::string& version, const std::string& resourcePrefix ) const
{
	if( !s_verbosity )
	{
		return;
	}

	std::cout << "---Creating Resource Group---" << std::endl;

    PrintCommonOperationHeaderInformation();

	std::cout << "Input Directory: " << inputDirectory << std::endl;

	std::cout << "Output File: " << outputFile<< std::endl;

	std::cout << "Output Document Version: " << version << std::endl;

	if( !resourcePrefix.empty() )
	{
		std::cout << "Resource Prefix: " << resourcePrefix << std::endl;
	}

	std::cout << "----------------------------\n" << std::endl;
}

bool CreateResourceGroupCliOperation::CreateResourceGroup( const std::filesystem::path& inputDirectory, const std::filesystem::path& resourceGroupOutputFile, CarbonResources::Version documentVersion, const std::string& resourcePrefix ) const
{
	CarbonResources::ResourceGroup resourceGroup;

	CarbonResources::CreateResourceGroupFromDirectoryParams createResourceGroupParams;

	createResourceGroupParams.directory = inputDirectory;

	createResourceGroupParams.outputDocumentVersion = documentVersion;

	createResourceGroupParams.resourcePrefix = resourcePrefix;

	createResourceGroupParams.statusCallback = GetStatusCallback();

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

	CarbonResources::Result exportToFileResult = resourceGroup.ExportToFile( exportParams );

	if( exportToFileResult.type != CarbonResources::ResultType::SUCCESS )
	{
		PrintCarbonResourcesError( exportToFileResult );

		return false;
	}

	return true;
}