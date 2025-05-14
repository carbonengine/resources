#include "CreateResourceGroupCliOperation.h"

#include <string>
#include <argparse/argparse.hpp>
#include <ResourceGroup.h>

CreateResourceGroupCliOperation::CreateResourceGroupCliOperation() :
	CliOperation( "create-group", "Create a resource group from a given directory." ),
	m_createResourceGroupPathArgumentId( "input-directory" ),
	m_createResourceGroupOutputFileArgumentId( "--output-file" )
{

	AddRequiredPositionalArgument( m_createResourceGroupPathArgumentId, "Base directory to create resource group from." );

	AddArgument( m_createResourceGroupOutputFileArgumentId, "Filename for created resource group.", false, "ResourceGroup.yaml" );
}

bool CreateResourceGroupCliOperation::Execute() const
{
	std::string inputDirectory = m_argumentParser->get<std::string>( m_createResourceGroupPathArgumentId );

	std::string outputFile = m_argumentParser->get<std::string>( m_createResourceGroupOutputFileArgumentId );

	PrintStartBanner( inputDirectory, outputFile );

	return CreateResourceGroup( inputDirectory, outputFile );
}

void CreateResourceGroupCliOperation::PrintStartBanner( const std::filesystem::path& inputDirectory, const std::filesystem::path& outputFile ) const
{
	if( !s_verbosity )
	{
		return;
	}
	std::cout << "---Creating Resource Group---" << std::endl;

    PrintCommonOperationHeaderInformation();

	std::cout << "Input Directory: " << inputDirectory << std::endl;

	std::cout << "Output File: " << outputFile<< std::endl;

	std::cout << "----------------------------\n" << std::endl;
}

bool CreateResourceGroupCliOperation::CreateResourceGroup( const std::filesystem::path& inputDirectory, const std::filesystem::path& resourceGroupOutputFile ) const
{
	CarbonResources::ResourceGroup resourceGroup;

	CarbonResources::CreateResourceGroupFromDirectoryParams createResourceGroupParams;

	createResourceGroupParams.directory = inputDirectory;

	createResourceGroupParams.statusCallback = GetStatusCallback();

	CarbonResources::Result createFromDirectoryResult = resourceGroup.CreateFromDirectory( createResourceGroupParams );

	if( createFromDirectoryResult.type != CarbonResources::ResultType::SUCCESS )
	{
		PrintCarbonResourcesError( createFromDirectoryResult );

		return false;
	}

	CarbonResources::ResourceGroupExportToFileParams exportParams;

	exportParams.filename = resourceGroupOutputFile;

	exportParams.statusCallback = GetStatusCallback();

	CarbonResources::Result exportToFileResult = resourceGroup.ExportToFile( exportParams );

	if( exportToFileResult.type != CarbonResources::ResultType::SUCCESS )
	{
		PrintCarbonResourcesError( exportToFileResult );

		return false;
	}

	return true;
}