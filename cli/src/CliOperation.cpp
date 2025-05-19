#include "CliOperation.h"

#include <argparse/argparse.hpp>

#include <ResourceGroup.h>

CliOperation::CliOperation( const std::string& name, const std::string& description ) :
	m_name( name ),
	m_description( description ),
	m_argumentParser( nullptr )
{
	m_argumentParser = new argparse::ArgumentParser( name );

	m_argumentParser->add_argument( "-V", "--verbose" )
		.action( [&]( const auto& ) { ++s_verbosity; } )
		.append()
		.default_value( false )
		.implicit_value( true )
		.nargs( 0 );
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

    if (append)
    {
		argument.append();
    }

	argument.default_value( defaultValue );
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
	std::cout << "Verbosity Level: " << s_verbosity << std::endl;
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
	return s_verbosity > 0 ? &StatusUpdate : nullptr;
}

void CliOperation::StatusUpdate( int layer, int progress, const std::string& info )
{
    // No update is shown for status updates for layers greater than the verbosity level
    if (layer > s_verbosity)
    {
		return;
    }

    // Verbosity level affects if a progres bar is shown or detail.
    // This level can be incremented to get final detail output
    // Processes with layers less than the verbosity level will print full log details
    // Processes which match verbosity level will show temp progress of step in a collapsed fashion
    // Processes with greater verbosity level will not show. To see these level must be increased from command line.
	if( layer < s_verbosity )
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
	else if(layer == s_verbosity)
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