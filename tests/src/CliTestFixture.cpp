// Copyright © 2025 CCP ehf.

#include "CliTestFixture.h"

#include <process.hpp>

#include <iostream>

int CliTestFixture::RunCli( std::vector<std::string>& arguments,
							std::string* standardOutput /* = nullptr */,
							std::string* errorOutput /* = nullptr */ )
{
	arguments.insert( arguments.begin(), CARBON_RESOURCES_CLI_EXE_FULLPATH );

	std::cout << "--- RunCli() arguments: ---" << std::endl;
	for( const auto& arg : arguments )
	{
		std::cout << " " << arg << std::endl;
	}
	std::cout << "---------------------------" << std::endl;

	// Only populate the output and errorOutput if the caller provided non-nullptr for them, otherwise discard them
	TinyProcessLib::Process process1a(
		arguments,
		"",
		[standardOutput]( const char* bytes, size_t n ) { if (standardOutput != nullptr) { *standardOutput += std::string( bytes, n ); } },
		[errorOutput]( const char* bytes, size_t n ) { if (errorOutput != nullptr) { *errorOutput += std::string( bytes, n ); } }
	);

	auto exit_status = process1a.get_exit_status();

	return exit_status;
}

// -------------------------------------------------------------
// Description:
//   Helper function to remove intermediate files generated as
//   part of a test being run.
// Arguments:
//   filesToRemove - Vector of file paths to remove.
// Return Value:
//   Nothing (void)
// -------------------------------------------------------------
void CliTestFixture::RemoveFiles( const std::vector<std::filesystem::path>& filesToRemove )
{
	for( const auto& filePath : filesToRemove )
	{
		if( std::filesystem::exists( filePath ) )
		{
			std::filesystem::remove( filePath );
		}
	}
}
