// Copyright © 2025 CCP ehf.

#include "CliTestFixture.h"

#include <process.hpp>

#include <iostream>

int CliTestFixture::RunCli( std::vector<std::string>& arguments, std::string& output, std::string& errorOutput )
{
	std::string processOutput;
	std::string processError;

	arguments.insert( arguments.begin(), CARBON_RESOURCES_CLI_EXE_NAME );

	// TODO: Add debug information on parameters
	std::cout << "RunCli arguments: " << std::endl;
	for( const auto& arg : arguments )
	{
		std::cout << " " << arg << std::endl;
	}

	TinyProcessLib::Process process1a(
		arguments, "",
		[&processOutput]( const char* bytes, size_t n ) { processOutput += std::string( bytes, n ); },
		[&processError]( const char* bytes, size_t n ) { processError += std::string( bytes, n ); }
	);

	auto exit_status = process1a.get_exit_status();

	output = processOutput;
	errorOutput = processError;

	return exit_status;
}

void CliTestFixture::CleanupTestOutputFiles( const std::vector<std::filesystem::path>& filesToRemove )
{
	for( const auto& filePath : filesToRemove )
	{
		if( std::filesystem::exists( filePath ) )
		{
			std::filesystem::remove( filePath );
		}
	}
}
