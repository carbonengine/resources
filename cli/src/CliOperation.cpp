// Copyright © 2025 CCP ehf.

#include "CliOperation.h"

#include <argparse/argparse.hpp>

#include <ResourceGroup.h>

CliOperation::CliOperation( const std::string& name, const std::string& description ) :
	m_verbosityLevelId( "--verbosity-level" ),
	m_name( name ),
	m_description( description ),
	m_argumentParser( nullptr ),
	m_verbosityLevel(-1)
{
	m_argumentParser = new argparse::ArgumentParser( name );

	m_argumentParser->add_description( description );

	AddArgument( m_verbosityLevelId, "Set verbosity to level", false, false, "0", "1 - n to register for updates from n nested processes, -1 for all, 0 for none." );
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

bool CliOperation::AddArgumentFlag( const std::string& argumentId, const std::string& helpString )
{
	if( !m_argumentParser )
	{
		return false;
	}

	argparse::Argument& argument = m_argumentParser->add_argument( argumentId )
									   .help( helpString );

	argument.default_value( false );

	argument.implicit_value( true );

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

	if( append )
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

	if( append )
	{
		argument.append();
	}

	return true;
}

argparse::ArgumentParser* CliOperation::GetParser() const
{
	return m_argumentParser;
}

void CliOperation::PrintError( std::string message /*= ""*/ ) const
{
	if( message != "" )
	{
		std::cerr << "[ERROR: " << message << "]\n\n";
	}

	std::cout << *m_argumentParser;
}

void CliOperation::PrintCommonOperationHeaderInformation() const
{
	std::cout << "Verbosity Level: " << m_verbosityLevel  << std::endl;
}

void CliOperation::PrintCarbonResourcesError( CarbonResources::Result result ) const
{
	std::string errorMessage;

	bool ret = CarbonResources::ResultTypeToString( result.type, errorMessage );

	std::cerr << "[ERROR: " << errorMessage << "]\n\n";

	if( result.info != "" )
	{
		std::cout << "\n======ERROR INFORMATION======\n";
		std::cout << result.info << "\n";
		std::cout << "============================\n";
	}

	std::cout << std::endl;
}

CarbonResources::StatusCallback CliOperation::GetStatusCallback() const
{
	return &CarbonResourcesStatusUpdate;
}

void CliOperation::CliStatusUpdate( const std::string& info) const
{
	std::cout << "\n---" << info << "---" << std::endl;
}

void CliOperation::CarbonResourcesStatusUpdate( CarbonResources::StatusProgressType type, float processProgress, float overallProgress, float sizeOfJob, unsigned int nestingLevel, const std::string& info )
{
    // The nesting level is an internal value to carbon-resources
    // carbon-resources has no information regarding outside processes
    // This CLI also has to track its progress of overall tasks
    // The CLI values will show with zero indent
    // Therefore carbon-resources updates need shifting by one indent.

    if (type == CarbonResources::StatusProgressType::START)
    {
        // Update function doesn need to process progress start types
		return;
    }

	unsigned int localNestingLevel = nestingLevel + 1;

	// Show detail of the process
	std::stringstream ss;

	ss << "[" << static_cast<int>( overallProgress )<< "%] ";

	// Apply indent based on layer
	for( unsigned int i = 0; i < localNestingLevel; i++ )
	{
		ss << "\t";
	}

	ss << info;

	std::string message = ss.str();
	std::cout << message;
	std::cout << std::endl;
    
}

bool CliOperation::ProcessCommandLine( int argc, char** argv )
{
	try
	{
		std::vector<std::string> arguments;

		arguments.push_back( argv[0] );

		if( argc > 2 )
		{
			for( int i = 2; i < argc; i++ )
			{
				std::string argument( argv[i] );

				arguments.push_back( argument );
			}
		}

		m_argumentParser->parse_args( arguments );
	}
	catch( const std::runtime_error& )
	{

		return false;
	}

	if( !SetVerbosityLevel() )
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
	case CarbonResources::ResourceSourceType::LOCAL_RELATIVE:
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
	bool first{ true };
	for( const auto& s : v )
	{
		if( !first )
		{
			result += ",";
		}
		first = false;
		result += s.string();
	}
	return result;
}

int CliOperation::GetVerbosityLevel() const
{
	return m_verbosityLevel;
}

bool CliOperation::SetVerbosityLevel()
{
	std::string verbosityLevelString = m_argumentParser->get<std::string>( m_verbosityLevelId );
    try
    {
		int verbosityLevelInt = std::stoi( verbosityLevelString );
		m_verbosityLevel = verbosityLevelInt;
		return true;
    }
	catch( std::invalid_argument& )
    {
		return false;
    }
	catch( std::out_of_range& )
	{
		return false;
	}
	
}

bool CliOperation::ShowCliStatusUpdates() const
{
	return m_verbosityLevel > 0 || m_verbosityLevel == -1;
}


bool CliOperation::ParseDocumentVersion( const std::string& version, CarbonResources::Version& documentVersion ) const
{
	try
	{
		auto first = version.find( "." );
		unsigned long in{ 0 };

		in = std::stoul( version.substr( 0, first ) );
		if( in > std::numeric_limits<unsigned int>::max() )
		{
			return false;
		}
		documentVersion.major = static_cast<unsigned int>( in );
		auto second = version.find( ".", first + 1 );
		in = std::stoul( version.substr( first + 1, second - first ) );
		if( in > std::numeric_limits<unsigned int>::max() )
		{
			return false;
		}
		documentVersion.minor = static_cast<unsigned int>( in );

		in = std::stoul( version.substr( second + 1 ) );
		if( in > std::numeric_limits<unsigned int>::max() )
		{
			return false;
		}
		documentVersion.patch = static_cast<unsigned int>( in );
	}
	catch( std::invalid_argument& )
	{
		return false;
	}
	catch( std::out_of_range& )
	{
		return false;
	}
	return true;
}