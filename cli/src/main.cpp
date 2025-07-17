// Copyright © 2025 CCP ehf.

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
#include "Defines.h"

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
        // Prints help

		cli.PrintError();

		std::exit( FAILED_NO_OPERATION_SPECIFIED_RETURN );
    }

    // Process commandline
	int res = cli.ProcessCommandLine( argc, argv );

    if (res != 0)
    {
		std::exit( res );
    }

    return SUCCESSFUL_RETURN;
}