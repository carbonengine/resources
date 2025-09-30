// Copyright © 2025 CCP ehf.

#include "Cli.h"

#include <argparse/argparse.hpp>

#include "Defines.h"

Cli::Cli( const std::string& name, const std::string& version ) :
	m_name( name ),
	m_version( version )
{
}

Cli::~Cli()
{
}

void Cli::AddOperation( CliOperation* operation )
{
	m_operations.push_back( operation );
}

void Cli::PrintError()
{
	PrintCliHeader();

	std::cout << "Operations:" << std::endl;

	for( auto operation : m_operations )
	{
		std::string operationName = operation->GetName();

		std::cout << "\t" << operation->GetName();

		std::cout << "\t\t" << operation->GetDescription() << std::endl;
	}
}

int Cli::ProcessCommandLine( int argc, char** argv )
{
	std::string baseCommandName = argv[1];

	for( auto operation : m_operations )
	{
		if( baseCommandName == operation->GetName() )
		{
			if( argc == 2 )
			{
				// Command was used but no further arguments supplied
				operation->PrintError();

				return FAILED_INVALID_OPERATION_ARGUMENTS_RETURN;
			}

			if( operation->ProcessCommandLine( argc, argv ) )
			{
				std::string optionalReturnMessage = "";

				if( !operation->Execute( optionalReturnMessage ) )
				{
					operation->PrintError( optionalReturnMessage );

					return FAILED_OPERATION_RETURN;
				}
				else
				{
					return SUCCESSFUL_RETURN;
				}
			}
			else
			{
				operation->PrintError( "Error processing arguments.Ensure all required arguments are supplied." );

				return FAILED_INVALID_OPERATION_ARGUMENTS_RETURN;
			}
		}
	}

	// Print general error message
	PrintError();

	return FAILED_INVALID_OPERATION_SPECIFIED_RETURN;
}

void Cli::PrintCliHeader()
{
	std::cout << "====================" << std::endl;
	std::cout << "resources-cli" << std::endl;
	std::cout << "Name:" << m_name << std::endl;
	std::cout << "Version: " << m_version << std::endl;
	std::cout << "====================\n"
			  << std::endl;
}