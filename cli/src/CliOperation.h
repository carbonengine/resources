// Copyright © 2025 CCP ehf.

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

	void PrintError( std::string message = "" ) const;

	virtual bool Execute( std::string& returnErrorMessage ) const = 0;

	bool ProcessCommandLine( int argc, char** argv );

	std::string GetName() const;

	std::string GetDescription() const;

protected:
	void PrintCommonOperationHeaderInformation() const;

	void PrintCarbonResourcesError( CarbonResources::Result result ) const;

	bool AddRequiredPositionalArgument( const std::string& argumentId, const std::string& helpString );

	bool AddArgument( const std::string& argumentId, const std::string& helpString, bool required = false, bool append = false, std::string defaultValue = "", std::string choicesString = "" );

	bool AddArgumentFlag( const std::string& argumentId, const std::string& helpString );

	argparse::ArgumentParser* m_argumentParser;

	CarbonResources::StatusCallback GetStatusCallback() const;

	bool StringToResourceSourceType( const std::string& stringRepresentation, CarbonResources::ResourceSourceType& out ) const;

	bool StringToResourceDestinationType( const std::string& stringRepresentation, CarbonResources::ResourceDestinationType& out ) const;

	std::string PathListToString( std::vector<std::filesystem::path>& paths ) const;

	std::string SourceTypeToString( CarbonResources::ResourceSourceType type ) const;

	std::string DestinationTypeToString( CarbonResources::ResourceDestinationType type ) const;

	std::string SizeToString( uintmax_t size ) const;

	std::string SecondsToString( std::chrono::seconds seconds ) const;

	std::string VersionToString( CarbonResources::Version& version ) const;

	std::string ResourceSourceTypeChoicesAsString() const;

	std::string ResourceDestinationTypeChoicesAsString() const;

	bool ParseDocumentVersion( const std::string& version, CarbonResources::Version& documentVersion ) const;

    bool ShowCliStatusUpdates() const;

    void CliStatusUpdate( const std::string& info ) const;

private:

	static void CarbonResourcesStatusUpdate( CarbonResources::StatusProgressType type, float processProgress, float overallProgress, float sizeOfJob, unsigned int nestingLevel, const std::string& info );

	bool SetVerbosityLevel();

protected:

    int GetVerbosityLevel() const;

private:
	int m_verbosityLevel;

	std::string m_verbosityLevelId;

	std::string m_name;

	std::string m_description;
};

std::string PathsToString( const std::vector<std::filesystem::path>& v );

#endif // CliOperation_H