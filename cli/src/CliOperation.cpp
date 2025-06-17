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

    m_argumentParser->add_description( description );

    AddArgument( m_verbosityLevelId, "Set verbosity to level", false, false, "0", "0,1,2,3" );

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

    return true;
}

bool CliOperation::AddArgument( const std::string& argumentId, const std::string& helpString, bool required /* = false*/, bool append /* = false*/, std::string defaultValue /*= ""*/, std::string choicesString /* = ""*/ )
{
	if( !m_argumentParser )
	{
		return false;
	}

    // Some useful information is added to help string based on argument inputs specified
    std::string helpStringExtended = helpString;

    if( choicesString != "" )
    {
		std::stringstream ss;

        ss << helpStringExtended << " [Choices: " << choicesString << "]";

        helpStringExtended = ss.str();
    }

    if (append)
    {
		helpStringExtended = helpStringExtended + " [Accepts multiple]";
    }

	argparse::Argument& argument = m_argumentParser->add_argument( argumentId )
									   .help( helpStringExtended );

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

    return true;
}

argparse::ArgumentParser* CliOperation::GetParser() const
{
	return m_argumentParser;
}

void CliOperation::PrintError(std::string message) const
{
    if (message != "")
    {
		std::cerr << "[ERROR: " << message << "]\n\n";
    }

	std::cout << *m_argumentParser;
}

void CliOperation::PrintCommonOperationHeaderInformation() const
{
	std::cout << "Verbosity Level: " << VerbosityLevelToString(s_verbosityLevel) << std::endl;
}

void CliOperation::PrintCarbonResourcesError( CarbonResources::Result result ) const
{
	std::string errorMessage;

	bool ret = CarbonResources::ResultTypeToString( result.type, errorMessage );

	std::cerr << "[ERROR: " << errorMessage << "]\n\n";

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
	return s_verbosityLevel != CarbonResources::StatusLevel::OFF ? &StatusUpdate : nullptr;
}

void CliOperation::StatusUpdate( CarbonResources::StatusLevel level, CarbonResources::StatusProgressType type, int progress, const std::string& info )
{
    // No update is shown for status updates for layers greater than the verbosity level
	if( level > s_verbosityLevel ) 
    {
		return;
    }

    // Verbosity level affects if a progres bar is shown or detail.
    // This level can be incremented to adjust detail of log output
	if( level <= s_verbosityLevel )
	{
		// Show detail of the process
		std::stringstream ss;

        // Apply indent based on layer
		ss << GetVerbosityLevelIndent( level );

        ss << "[";

        // If progress is unbounded then show busy
        // Else print percentage progress
        if (type == CarbonResources::StatusProgressType::UNBOUNDED)
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
        std::cout << std::endl;

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

std::string CliOperation::GetDescription() const
{
	return m_description;
}

std::string CliOperation::VerbosityLevelToString( CarbonResources::StatusLevel level ) const
{
    switch (level)
    {
	case CarbonResources::StatusLevel::OFF:
		return "0 - (Off)";

	case CarbonResources::StatusLevel::OVERVIEW:
		return "1 - (Overview)";

	case CarbonResources::StatusLevel::PROCEDURE:
		return "2 - (Procedure)";

	case CarbonResources::StatusLevel::DETAIL:
		return "3 - (Detail)";

	default:
		return "Unknown";
    }
}

std::string CliOperation::GetVerbosityLevelIndent(CarbonResources::StatusLevel level)
{
	switch( level )
	{
	case CarbonResources::StatusLevel::OFF:
		return "";

	case CarbonResources::StatusLevel::OVERVIEW:
		return "";

	case CarbonResources::StatusLevel::PROCEDURE:
		return "\t";

	case CarbonResources::StatusLevel::DETAIL:
		return "\t\t";

	default:
		return "";
	}
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

std::string CliOperation::PathListToString( std::vector<std::filesystem::path>& paths ) const
{
	std::stringstream ss;

	for( auto iter = paths.begin(); iter != paths.end(); iter++ )
	{
		ss << ( *iter );

		if( iter + 1 != paths.end() )
		{
			ss << ",";
		}
	}

    return ss.str();
}

std::string CliOperation::SourceTypeToString( CarbonResources::ResourceSourceType type ) const
{
	switch( type )
	{
	case CarbonResources::ResourceSourceType::LOCAL_RELATIVE :
		return "LOCAL_RELATIVE";

	case CarbonResources::ResourceSourceType::LOCAL_CDN:
		return "LOCAL_CDN";

	case CarbonResources::ResourceSourceType::REMOTE_CDN:
		return "REMOTE_CDN";

	default:
		return "Unrecognised source type";
	}
}

std::string CliOperation::SizeToString( uintmax_t size ) const
{
	std::stringstream ss;

	ss << size;

    return ss.str();
}

std::string CliOperation::SecondsToString( std::chrono::seconds seconds ) const
{
	std::stringstream ss;

	ss << seconds.count();

    return ss.str();
}

std::string CliOperation::VersionToString( CarbonResources::Version& version ) const
{
	std::stringstream ss;

	ss << version.major << "." << version.minor << "." << version.patch;

	return ss.str();
}

std::string CliOperation::ResourceSourceTypeChoicesAsString() const
{
	return "LOCAL_RELATIVE, LOCAL_CDN, REMOTE_CDN";
}

std::string CliOperation::ResourceDestinationTypeChoicesAsString() const
{
	return "LOCAL_RELATIVE, LOCAL_CDN, REMOTE_CDN";
}

std::string CliOperation::DestinationTypeToString( CarbonResources::ResourceDestinationType type ) const
{
	switch( type )
	{
	case CarbonResources::ResourceDestinationType::LOCAL_RELATIVE:
		return "LOCAL_RELATIVE";

	case CarbonResources::ResourceDestinationType::LOCAL_CDN:
		return "LOCAL_CDN";

	case CarbonResources::ResourceDestinationType::REMOTE_CDN:
		return "REMOTE_CDN";

	default:
		return "Unrecognised source type";
	}
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
	std::string verbosityLevelString = m_argumentParser->get<std::string>( m_verbosityLevelId );

    if( verbosityLevelString == "0" )
    {
		s_verbosityLevel = CarbonResources::StatusLevel::OFF;
    }
	else if( verbosityLevelString == "1" )
    {
		s_verbosityLevel = CarbonResources::StatusLevel::OVERVIEW;
    }
	else if( verbosityLevelString == "2" )
	{
		s_verbosityLevel = CarbonResources::StatusLevel::PROCEDURE;
	}
	else if( verbosityLevelString == "3" )
	{
		s_verbosityLevel = CarbonResources::StatusLevel::DETAIL;
	}
    else
    {
		return false;
    }

	return true;
}