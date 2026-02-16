// Copyright © 2025 CCP ehf.

#include "RemoveResourcesCliOperation.h"

#include <PatchResourceGroup.h>

#include <iostream>
#include <argparse/argparse.hpp>
#include <fstream>

RemoveResourcesCliOperation::RemoveResourcesCliOperation() :
	CliOperation( "remove-resources", "Remove resources from a ResourceGroup identified by supplied text file containing a list of RelativePaths to remove." ),
	m_resourceGroupPathArgumentId( "resource-group-path" ),
	m_resourceListPath( "resource-list-path" ),
	m_outputResourceGroupDocumentVersionArgumentId( "--document-version" ),
	m_outputResourceGroupPath( "--output-resource-group-path" ),
	m_ignoreMissingResources( "--ignore-missing-resources" )
{
	AddRequiredPositionalArgument( m_resourceGroupPathArgumentId, "The path to the Resource Group to remove resources from." );

	AddRequiredPositionalArgument( m_resourceListPath, "Path to text file containing list of RelativePaths of resources to remove, separated by newlines." );

	CarbonResources::ResourceGroupExportToFileParams defaultParams;

	AddArgument( m_outputResourceGroupDocumentVersionArgumentId, "Document version for created resource group.", false, false, VersionToString( defaultParams.outputDocumentVersion ) );

	AddArgument( m_outputResourceGroupPath, "Filename for created resource group.", false, false, defaultParams.filename.string() );

	AddArgumentFlag( m_ignoreMissingResources, "Set to ignore 'resource not found' errors caused by supplying a list with Resources not present in ResourceGroup." );
}

bool RemoveResourcesCliOperation::Execute( std::string& returnErrorMessage ) const
{
	CarbonResources::ResourceGroupImportFromFileParams importParams;

	std::optional<std::string> baseResourceGroupFilename = m_argumentParser->present<std::string>( m_resourceGroupPathArgumentId );
	if( !baseResourceGroupFilename.has_value() )
	{
		returnErrorMessage = "Failed to parse base Resource Group filename.";

		return false;
	}
	importParams.filename = baseResourceGroupFilename.value();

	std::optional<std::string> resourceListFilepath = m_argumentParser->present<std::string>( m_resourceListPath );
	if( !resourceListFilepath.has_value() )
	{
		returnErrorMessage = "Failed to parse base Resource Group filename.";

		return false;
	}
	std::filesystem::path resourcesToRemovePath = resourceListFilepath.value();

	CarbonResources::ResourceGroupExportToFileParams exportParams;

	std::string outResourceGroupFilename = m_argumentParser->get<std::string>( m_outputResourceGroupPath );

	exportParams.filename = outResourceGroupFilename;

	std::string version = m_argumentParser->get( m_outputResourceGroupDocumentVersionArgumentId );

	CarbonResources::Version documentVersion;

	bool versionIsValid = ParseDocumentVersion( version, documentVersion );

	if( !versionIsValid )
	{
		returnErrorMessage = "Invalid document version";

		return false;
	}

	exportParams.outputDocumentVersion = documentVersion;

	bool ignoreMissingResources = m_argumentParser->get<bool>( m_ignoreMissingResources );

    if (ShowCliStatusUpdates())
    {
		PrintStartBanner( importParams, resourcesToRemovePath, exportParams, ignoreMissingResources, version );
    }

	return RemoveResources( importParams, resourcesToRemovePath, exportParams, ignoreMissingResources );
}

void RemoveResourcesCliOperation::PrintStartBanner( const CarbonResources::ResourceGroupImportFromFileParams& importParams, std::filesystem::path& resourcesToRemoveFile, CarbonResources::ResourceGroupExportToFileParams& exportParams, bool ignoreMissingResources, const std::string& version ) const
{
	std::cout << "---Removing Resources---" << std::endl;

	PrintCommonOperationHeaderInformation();

	std::cout << "Resource Group: " << importParams.filename << std::endl;
	std::cout << "Resources to remove Path: " << resourcesToRemoveFile << std::endl;
	std::cout << "Output Resource Group Path: " << exportParams.filename << std::endl;
	std::cout << "Output Document Version: " << version << std::endl;
	if( ignoreMissingResources )
	{
		std::cout << "Ignore missing Resources: On" << exportParams.filename << std::endl;
	}
	else
	{
		std::cout << "Ignore missing Resources: Off" << exportParams.filename << std::endl;
	}

	std::cout << "----------------------------\n"
			  << std::endl;
}

bool RemoveResourcesCliOperation::ReadResourcesToRemoveFile( std::filesystem::path& pathToResourcesToRemoveFile, std::vector<std::filesystem::path>& resourcesToRemoveOut ) const
{
	std::ifstream removeListFile( pathToResourcesToRemoveFile );

	if( !removeListFile )
	{
		return false;
	}

	std::string line;
	while( std::getline( removeListFile, line ) )
	{
		resourcesToRemoveOut.push_back( line );
	}

	removeListFile.close();

	return true;
}

bool RemoveResourcesCliOperation::RemoveResources( CarbonResources::ResourceGroupImportFromFileParams& importParams, std::filesystem::path& resourcesToRemoveFile, CarbonResources::ResourceGroupExportToFileParams& exportParams, bool ignoreMissingResources ) const
{
	CarbonResources::StatusCallback statusCallback = GetStatusCallback();

	// Import base Resource Group
	CarbonResources::ResourceGroup resourceGroup;

    importParams.callbackSettings.statusCallback = statusCallback;

    if( ShowCliStatusUpdates() )
	{
		CliStatusUpdate( "Import Resource Group from file." );
	}

	CarbonResources::Result importResourceGroupResult = resourceGroup.ImportFromFile( importParams );

	if( importResourceGroupResult.type != CarbonResources::ResultType::SUCCESS )
	{
		PrintCarbonResourcesError( importResourceGroupResult );

		return false;
	}

	// Get resources to remove
	if( ShowCliStatusUpdates() )
	{
		CliStatusUpdate( "Reading resources to remove file." );
	}

	std::vector<std::filesystem::path> resourcesToRemove;

	if( !ReadResourcesToRemoveFile( resourcesToRemoveFile, resourcesToRemove ) )
	{
		std::cerr << "Failed to read resources to remove file." << std::endl;

		return false;
	}

	CarbonResources::ResourceGroupRemoveResourcesParams removeParams;

	removeParams.resourcesToRemove = &resourcesToRemove;

	removeParams.errorIfResourceNotFound = !ignoreMissingResources;

    removeParams.callbackSettings.statusCallback = statusCallback;

    if( ShowCliStatusUpdates() )
	{
		CliStatusUpdate( "Removing Resources." );
	}

	CarbonResources::Result removeResult = resourceGroup.RemoveResources( removeParams );

	if( removeResult.type != CarbonResources::ResultType::SUCCESS )
	{
		PrintCarbonResourcesError( removeResult );

		return false;
	}

    exportParams.callbackSettings.statusCallback = statusCallback;

    if( ShowCliStatusUpdates() )
	{
		CliStatusUpdate( "Exporting result Resource Group to file." );
	}

	CarbonResources::Result exportResult = resourceGroup.ExportToFile( exportParams );

	if( exportResult.type != CarbonResources::ResultType::SUCCESS )
	{
		PrintCarbonResourcesError( exportResult );

		return false;
	}

    if( ShowCliStatusUpdates() )
	{
		CliStatusUpdate( "Operation complete." );
	}

	return true;
}
