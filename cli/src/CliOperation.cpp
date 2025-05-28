#include "CliOperation.h"

#include <argparse/argparse.hpp>

#include <ResourceGroup.h>

CliOperation::CliOperation( const std::string& name, const std::string& description ) :
	m_verbosityLevelId( "--verbosity-level" ),
	m_name( name ),
	m_description( description ),
	m_argumentParser( nullptr )
{
	m_argumentParser = new argparse::ArgumentParser( name );

    AddArgument( m_verbosityLevelId, "Set verbosity to level", false, false, "0" );

}

CliOperation ::~CliOperation()
{
	if( m_argumentParser )
	{
		delete m_argumentParser;
	}
}

bool CliOperation::AddRequiredPositionalArgument( const std::string& argumentId, const std::string& helpString )
{
	if( !m_argumentParser )
	{
		return false;
	}

	m_argumentParser->add_argument( argumentId )
		.help( helpString );
}

bool CliOperation::AddArgument( const std::string& argumentId, const std::string& helpString, bool required /* = false*/, bool append /* = false*/, std::string defaultValue /*= ""*/ )
{
	if( !m_argumentParser )
	{
		return false;
	}

	argparse::Argument& argument = m_argumentParser->add_argument( argumentId )
									  .help( helpString );

	if( required )
	{
		argument.required();
	}
	else
	{
		argument.default_value( defaultValue );
	}

    if (append)
    {
		argument.append();
    }
}

argparse::ArgumentParser* CliOperation::GetParser() const
{
	return m_argumentParser;
}

void CliOperation::PrintError() const
{
	std::cout << *m_argumentParser;
}

void CliOperation::PrintCommonOperationHeaderInformation() const
{
	std::cout << "Verbosity Level: " << s_verbosityLevel << std::endl;
}

void CliOperation::PrintCarbonResourcesError( CarbonResources::Result result ) const
{
	std::string errorMessage;

	bool ret = CarbonResources::resultToString( result, errorMessage );

	std::cout << errorMessage << std::endl;

    if (result.info != "")
    {
		std::cout << "\n======ERROR INFORMATION======\n";
		std::cout << result.info << "\n";
		std::cout << "============================\n";
    }

	std::cout << std::endl;
}

char CliOperation::GetBusyChar()
{
    switch (s_currentBusyAnimationChar)
    {
	    case '/':
        {
		    s_currentBusyAnimationChar = '-';

		    break;
        }
		
	    case '-':
        {
		    s_currentBusyAnimationChar = '\\';

            break;
        }
		
	    case '\\':
        {
		    s_currentBusyAnimationChar = '|';

            break;
        }
		
	    case '|':
        {
		    s_currentBusyAnimationChar = '/';

            break;
        }
		
	    default:
        {
		    '--';
        }
		
    }

    return s_currentBusyAnimationChar;
}

CarbonResources::StatusCallback CliOperation::GetStatusCallback() const
{
	return s_verbosityLevel > 0 ? &StatusUpdate : nullptr;
}

void CliOperation::StatusUpdate( int layer, int progress, const std::string& info )
{
    // No update is shown for status updates for layers greater than the verbosity level
	if( layer > s_verbosityLevel )
    {
		return;
    }

    // Verbosity level affects if a progres bar is shown or detail.
    // This level can be incremented to get final detail output
    // Processes with layers less than the verbosity level will print full log details
    // Processes which match verbosity level will show temp progress of step in a collapsed fashion
    // Processes with greater verbosity level will not show. To see these level must be increased from command line.
	if( layer < s_verbosityLevel )
	{
		// Show detail of the process
		std::stringstream ss;

        // Apply indent based on layer
		for( int i = 0; i < layer; i++ )
		{
			ss << "    ";
		}

        ss << "[";

        // If progress is less than 0 then show busy
        // Else print percentage progress
        if (progress < 0)
        {
			ss << GetBusyChar();	
        }
        else
        {
			ss << progress << "%";
        }
		ss << "] " << info;

        std::string message = ss.str();

        std::cout << "\r";

		std::cout << message;
		
        // Overwrite extra data from last message
        if( message.size() < s_lastMessageLength )
		{
			for( int i = message.size(); i < s_lastMessageLength; i++ )
			{
				std::cout << " ";
			}
		}

        std::cout << std::endl;

        // Set to zero as this line will not be overwritten
        s_lastMessageLength = 0;
	}
	else if( layer == s_verbosityLevel )
    {
		std::stringstream ss;

        // Apply indent based on layer
		for( int i = 0; i < layer; i++ )
		{
			ss << "    ";
		}

        ss << "[";

		// If progress is less than 0 then show busy
		// Else print percentage progress
		if( progress < 0 )
		{
			ss << GetBusyChar();
		}
		else
		{
			ss << progress << "%";
		}
		ss << "] ";

        std::string message = ss.str();

        std::cout << "\r";

		std::cout << message;

        // Overwrite extra data from last message
		if( message.size() < s_lastMessageLength )
		{
			for( int i = message.size(); i < s_lastMessageLength; i++ )
			{
				std::cout << " ";
			}
		}

        std::cout.flush();

        s_lastMessageLength = message.size();
    }
	
    

}

std::string CliOperation::SourceTypeToString( CarbonResources::ResourceSourceType type ) const
{
    switch (type)
    {
	case CarbonResources::ResourceSourceType::LOCAL_CDN:
		return "Local Cdn";
	case CarbonResources::ResourceSourceType::LOCAL_RELATIVE:
		return "Local Relative";
	case CarbonResources::ResourceSourceType::REMOTE_CDN:
		return "Remote Cdn";
	default:
		return "Unknown source type";
    }
}

std::string CliOperation::DestinationTypeToString( CarbonResources::ResourceDestinationType type ) const
{
	switch( type )
	{
	case CarbonResources::ResourceDestinationType::LOCAL_CDN:
		return "Local Cdn";
	case CarbonResources::ResourceDestinationType::LOCAL_RELATIVE:
		return "Local Relative";
	case CarbonResources::ResourceDestinationType::REMOTE_CDN:
		return "Remote Cdn";
	default:
		return "Unknown source type";
	}
}

bool CliOperation::ProcessCommandLine( int argc, char** argv )
{
	try
	{
		std::vector<std::string> arguments;

        arguments.push_back( argv[0] );

        if (argc > 2)
        {
			for( int i = 2; i < argc; i++ )
			{
				std::string argument( argv[i] );

				arguments.push_back( argument );
			}
        }

		m_argumentParser->parse_args( arguments );

	}
	catch( const std::runtime_error& e )
	{

		return false;

	}

    if (!SetVerbosityLevel())
    {
		return false;
    }

	return true;
}

std::string CliOperation::GetName() const
{
    return m_name;
}
bool CliOperation::StringToResourceSourceType( const std::string& stringRepresentation, CarbonResources::ResourceSourceType& out ) const
{
	if( stringRepresentation == "LOCAL_CDN" )
	{
		out = CarbonResources::ResourceSourceType::LOCAL_CDN;
	}
	else if( stringRepresentation == "REMOTE_CDN" )
	{
		out = CarbonResources::ResourceSourceType::REMOTE_CDN;
	}
	else if( stringRepresentation == "LOCAL_RELATIVE" )
	{
		out = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;
	}
	else
	{
		return false;
	}
	return true;
}

bool CliOperation::StringToResourceDestinationType( const std::string& stringRepresentation, CarbonResources::ResourceDestinationType& out ) const
{
	if( stringRepresentation == "LOCAL_CDN" )
	{
		out = CarbonResources::ResourceDestinationType::LOCAL_CDN;
	}
	else if( stringRepresentation == "REMOTE_CDN" )
	{
		out = CarbonResources::ResourceDestinationType::REMOTE_CDN;
	}
	else if( stringRepresentation == "LOCAL_RELATIVE" )
	{
		out = CarbonResources::ResourceDestinationType::LOCAL_RELATIVE;
	}
	else
	{
		return false;
	}
	return true;
}

std::string PathsToString( const std::vector<std::filesystem::path>& v )
{
	std::string result;
	bool first{true};
	for( const auto& s : v )
	{
		if(!first)
		{
			result += ",";
		}
		first = false;
		result += s.string();
	}
	return result;
}

bool CliOperation::SetVerbosityLevel()
{
	try
	{
		s_verbosityLevel = std::stoi( m_argumentParser->get( m_verbosityLevelId ) );
	}
	catch( std::invalid_argument& e )
	{
		return false;
	}
	catch( std::out_of_range& e )
	{
		return false;
	}

	return true;
}