// Copyright © 2025 CCP ehf.

#include "CliTestFixture.h"

#include <process.hpp>

#include <iostream>

int CliTestFixture::RunCli( std::vector<std::string>& arguments, std::string& output )
{
    std::string processOutput;

    arguments.insert( arguments.begin(), CARBON_RESOURCES_CLI_EXE_NAME );

    TinyProcessLib::Process process1a( arguments, "", [&processOutput]( const char* bytes, size_t n ) {
	    processOutput += std::string( bytes, n );
    } );

    auto exit_status = process1a.get_exit_status();

    output = processOutput;

    return exit_status;
}