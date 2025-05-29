#include <iostream>
#include <ResourceGroup.h>
#include <BundleResourceGroup.h>
#include <PatchResourceGroup.h>
#include <argparse/argparse.hpp>
#include <filesystem>

#include "Cli.h"
#include "ApplyPatchCliOperation.h"
#include "CreateResourceGroupCliOperation.h"
#include "CreatePatchCliOperation.h"
#include "CreateBundleCliOperation.h"
#include "UnpackBundleCliOperation.h"

std::string CalculateVersionString()
{
	std::stringstream ss;

	ss << CarbonResources::S_LIBRARY_VERSION.major << "." << CarbonResources::S_LIBRARY_VERSION.minor << "." << CarbonResources::S_LIBRARY_VERSION.patch;

#ifdef DEV_FEATURES
	ss << " [EXTENDED FEATURE DEVELOPMENT BUILD]";
#endif

    return ss.str();
}

int main( int argc, char** argv )
{
	Cli cli( "carbon-resources", CalculateVersionString() );

    CreateResourceGroupCliOperation createResourceGroupOperation;

    cli.AddOperation( &createResourceGroupOperation );

    CreatePatchCliOperation createPatchOperation;

    cli.AddOperation( &createPatchOperation );

    CreateBundleCliOperation createBundleOperation;

    cli.AddOperation( &createBundleOperation );

#ifdef DEV_FEATURES
	ApplyPatchCliOperation addPatchOperation;

	cli.AddOperation( &addPatchOperation );

	UnpackBundleCliOperation unpackBundleCliOperation;

	cli.AddOperation( &unpackBundleCliOperation );
#endif

    // Check no arguments
    if (argc == 1)
    {
		cli.PrintError();

		std::exit( 1 );
    }

    // Process commandline
	if( !cli.ProcessCommandLine( argc, argv ) )
	{
		std::exit( 1 );
	}

    return 0;
}