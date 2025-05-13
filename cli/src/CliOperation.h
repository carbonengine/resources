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

protected:

    void PrintCommonOperationHeaderInformation() const;

    void PrintCarbonResourcesError( CarbonResources::Result result ) const;

    bool AddRequiredPositionalArgument( const std::string& argumentId, const std::string& helpString );

	bool AddArgument( const std::string& argumentId, const std::string& helpString, bool required = false, std::string defaultValue = "" );

    argparse::ArgumentParser* m_argumentParser;

    CarbonResources::StatusCallback GetStatusCallback() const;

    std::string SourceTypeToString( CarbonResources::ResourceSourceType type ) const;

    std::string DestinationTypeToString( CarbonResources::ResourceDestinationType type ) const;

private:

    static void StatusUpdate( int layer, int progress, const std::string& info );

    static char GetBusyChar();

protected:

	static inline unsigned int s_verbosity = 0;

private:
	std::string m_name;

	std::string m_description;

    static inline unsigned int s_lastMessageLength = 0;
    
	static inline char s_currentBusyAnimationChar = '/';

};

#endif // CliOperation_H