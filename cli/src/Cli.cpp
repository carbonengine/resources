#include "Cli.h"

#include <argparse/argparse.hpp>

Cli::Cli( const std::string& name, const std::string& version ):
	m_version(version)
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

    for (auto operation : m_operations)
    {
		std::string operationName = operation->GetName();

		std::cout << "\t" << operation->GetName();

		std::cout << "\t\t" << operation->GetDescription() << std::endl;
    }
}

bool Cli::ProcessCommandLine( int argc, char** argv )
{
	std::string baseCommandName = argv[1];

    for (auto operation : m_operations)
    {
		if( baseCommandName == operation->GetName() )
		{
			if( operation->ProcessCommandLine( argc, argv ) )
			{
				std::string optionalReturnMessage = "";

				if( !operation->Execute( optionalReturnMessage ) )
				{
					operation->PrintError( optionalReturnMessage );

                    return false;
				}
                else
                {
					return true;
                }
			}
            else
            {
				operation->PrintError("Error processing arguments.Ensure all required arguments are supplied.");

				return false;
            }
		}
    }

    // Print general error message
    PrintError();

    return false;

}

void Cli::PrintCliHeader()
{
	std::cout << "====================" << std::endl;
	std::cout << "carbon-resources-cli" << std::endl;
	std::cout << "Version: " << m_version << std::endl;
    std::cout << "====================\n" << std::endl;

}