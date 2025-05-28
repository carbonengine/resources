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
		std::cout << "    " << operation->GetName() << std::endl;
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
				if( !operation->Execute() )
				{
					operation->PrintError();

                    return false;
				}
                else
                {
					return true;
                }
			}
            else
            {
				operation->PrintError();

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

    std::stringstream ss;

	ss << CarbonResources::S_LIBRARY_VERSION.major << "." << CarbonResources::S_LIBRARY_VERSION.minor << "." << CarbonResources::S_LIBRARY_VERSION.patch;

	std::cout << "carbon-resources version: " << ss.str() << std::endl;

    std::cout << "====================\n" << std::endl;

}