/* 
	*************************************************************************

	CliOperation.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/
#pragma once
#ifndef CliOperation_H
#define CliOperation_H

#include <string>

#include <Enums.h>

#include <filesystem>
#include <functional>

namespace CarbonResources
{
    enum class ResourceSourceType;
    enum class ResourceDestinationType;
}

namespace argparse
{
class ArgumentParser;
}

class CliOperation
{
public:

	CliOperation( const std::string& name, const std::string& description );

	~CliOperation();

	argparse::ArgumentParser* GetParser() const;

	void PrintError() const;

	virtual bool Execute() const = 0;

    bool ProcessCommandLine( int argc, char** argv );

    std::string GetName() const;

protected:

    void PrintCommonOperationHeaderInformation() const;

    void PrintCarbonResourcesError( CarbonResources::Result result ) const;

    bool AddRequiredPositionalArgument( const std::string& argumentId, const std::string& helpString );

	bool AddArgument( const std::string& argumentId, const std::string& helpString, bool required = false, bool append = false, std::string defaultValue = "" );

    argparse::ArgumentParser* m_argumentParser;

    CarbonResources::StatusCallback GetStatusCallback() const;

    std::string SourceTypeToString( CarbonResources::ResourceSourceType type ) const;

    std::string DestinationTypeToString( CarbonResources::ResourceDestinationType type ) const;

	bool StringToResourceSourceType( const std::string& stringRepresentation, CarbonResources::ResourceSourceType& out ) const;

	bool StringToResourceDestinationType( const std::string& stringRepresentation, CarbonResources::ResourceDestinationType& out ) const;

private:

    static void StatusUpdate( int layer, int progress, const std::string& info );

    static char GetBusyChar();

    bool SetVerbosityLevel();

protected:

	static inline unsigned int s_verbosityLevel = 0;

private:
	std::string m_verbosityLevelId;

	std::string m_name;

	std::string m_description;

    static inline unsigned int s_lastMessageLength = 0;
    
	static inline char s_currentBusyAnimationChar = '/';

};

std::string PathsToString( const std::vector<std::filesystem::path>& v );

#endif // CliOperation_H