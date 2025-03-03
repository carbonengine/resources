// Your First C++ Program

#include <iostream>
#include <ResourceGroup.h>
#include <BundleResourceGroup.h>
#include <PatchResourceGroup.h>
#include <argparse/argparse.hpp>


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
	argparse::ArgumentParser cli( "carbon-resources", CarbonResources::S_LIBRARY_VERSION.ToString() );
	


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
    else
    {
		std::cerr << cli;

		std::exit( 1 );
    }

    return result == true ? 0 : -1;
}