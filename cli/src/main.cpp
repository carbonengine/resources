// Your First C++ Program

#include <iostream>
#include <ResourceGroup.h>
#include <BundleResourceGroup.h>
#include <PatchResourceGroup.h>
#include <argparse/argparse.hpp>
#include <BinaryResourceGroup.h>
#include <filesystem>

// TODO this interface needs work
void StatusUpdate(int progress, const std::string& info)
{
    if (info == "Percentage Update")
    {
		std::cout << "[";

        for (int i = 0; i < 100; i++)
        {
			if( i < progress )
            {
				std::cout << "=";
            }
			else if( i == progress )
            {
				std::cout << ">";
            }
            else
            {
				std::cout << " ";
            }
        }

        std::cout << "]" << progress << " %\r";

		std::cout.flush();
    }
    else
    {
		std::cout << info << std::endl;
    }
	
}

void PrintError(CarbonResources::Result result)
{
	std::string errorMessage;

    bool ret = CarbonResources::resultToString( result, errorMessage );

    std::cout << errorMessage << "\n" << std::endl;
}

bool CreateResourceGroup(const std::filesystem::path& inputDirectory, const std::filesystem::path& resourceGroupOutputDirectory)
{
    CarbonResources::ResourceGroup resourceGroup;

    CarbonResources::CreateResourceGroupFromDirectoryParams createResourceGroupParams;

    createResourceGroupParams.directory = inputDirectory;

    createResourceGroupParams.statusCallback = &StatusUpdate;

    CarbonResources::Result createFromDirectoryResult = resourceGroup.CreateFromDirectory( createResourceGroupParams );

    if( createFromDirectoryResult != CarbonResources::Result::SUCCESS )
    {
		PrintError( createFromDirectoryResult );

		return false;
    }

    CarbonResources::ResourceGroupExportToFileParams exportParams;

    exportParams.filename = resourceGroupOutputDirectory;

    exportParams.statusCallback = &StatusUpdate;

    CarbonResources::Result exportToFileResult = resourceGroup.ExportToFile( exportParams );

    if( exportToFileResult != CarbonResources::Result::SUCCESS )
    {
		PrintError( exportToFileResult );

		return false;
    }

	return true;
}

bool CreateBundle( const std::string& inputResourceListPath )
{
    
    return false;
}

bool CreatePatch( const std::string& inputResourceListPath )
{
    return false;
}

int main( int argc, char** argv )
{

    // Create CLI parser
	std::stringstream ss;
	ss << CarbonResources::S_LIBRARY_VERSION.major << "." << CarbonResources::S_LIBRARY_VERSION.minor << "." << CarbonResources::S_LIBRARY_VERSION.patch;
	argparse::ArgumentParser cli( "carbon-resources", ss.str() );
	
    // Create ResourceGroup
	std::string createResourceGroupName = "create-group";

    argparse::ArgumentParser createResourceGroupCli( createResourceGroupName );

    createResourceGroupCli.add_description( "Create a resource group from a given directory." );

    // Required arguments
	std::string createResourceGroupPathArgumentId = "input-directory";
	createResourceGroupCli.add_argument( createResourceGroupPathArgumentId )
		.help( "Base directory to create resource group from." );

    std::string createResourceGroupOutputFilenameArgumentId = "--output-filename";
	createResourceGroupCli.add_argument( "-o", createResourceGroupOutputFilenameArgumentId )
		.required()
		.default_value( "ResourceGroup.yaml" )
		.append()
		.help( "Filename for created resource group." );





    // Bundle
    std::string create_bundle_cli_command_name = "create-bundle";

    argparse::ArgumentParser create_bundle_cli( create_bundle_cli_command_name );

    create_bundle_cli.add_description( "Create a bundle from a supplied Resource List." );

    // Required arguments
	std::string bundleInputResourceListPathArgumentId = "input-resource-list-path";
	create_bundle_cli.add_argument( bundleInputResourceListPathArgumentId )
		.help( "Path to resource list to create bundle from." );



    // patch
	std::string create_patch_cli_command_name = "create-patch";

	argparse::ArgumentParser create_patch_cli( create_patch_cli_command_name );

	create_patch_cli.add_description( "Create a patch from a supplied Resource List." );

	// Required arguments
	std::string patchInputResourceListPathArgumentId = "input-resource-list-path";
	create_patch_cli.add_argument( patchInputResourceListPathArgumentId )
		.help( "Path to resource list to create patch from." );

        

    // Add subparsers
	cli.add_subparser( createResourceGroupCli );
    cli.add_subparser( create_bundle_cli );
	cli.add_subparser( create_patch_cli );

    // No arguments passed
    if (argc == 1)
    {
		std::cout << cli;

		std::exit( 1 );
    }

    try
    {
		cli.parse_args( argc, argv );
    }
    catch (const std::runtime_error& e)
    {
		std::cerr << e.what() << std::endl;

        if( cli.is_subcommand_used( create_bundle_cli ) )
        {
			std::cerr << create_bundle_cli;
        }
        else if (cli.is_subcommand_used(createResourceGroupCli))
        {
			std::cerr << createResourceGroupCli;
        }
		else if( cli.is_subcommand_used( create_patch_cli ) )
		{
			std::cerr << create_patch_cli;
		}

        std::exit( 1 );
    }

    bool result = false;

    // Exec
	if( cli.is_subcommand_used( create_bundle_cli ) )
	{
		std::string resourceListPath = create_bundle_cli.get<std::string>( bundleInputResourceListPathArgumentId );

		result = CreateBundle( resourceListPath );
	}
    else if (cli.is_subcommand_used(create_patch_cli))
    {
		std::string resourceListPath = create_patch_cli.get<std::string>( patchInputResourceListPathArgumentId );

		result = CreatePatch( resourceListPath );
    }
	else if( cli.is_subcommand_used( createResourceGroupCli ) )
	{
		std::string inputDirectory = createResourceGroupCli.get<std::string>( createResourceGroupPathArgumentId );

        std::string resourceGroupOutputDirectory = createResourceGroupCli.get<std::string>( createResourceGroupOutputFilenameArgumentId );

		result = CreateResourceGroup( inputDirectory, resourceGroupOutputDirectory );

        if (!result)
        {
			std::cerr << createResourceGroupCli;
        }
	}
    else
    {
		std::cerr << cli;

		std::exit( 1 );
    }

    return result == true ? 0 : -1;
}