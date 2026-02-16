// Copyright © 2025 CCP ehf.

#include "MergeResourceGroupCliOperation.h"

#include <PatchResourceGroup.h>

#include <iostream>
#include <argparse/argparse.hpp>

MergeResourceGroupCliOperation::MergeResourceGroupCliOperation() :
	CliOperation( "merge-group", "Merge two Resource Groups together" ),
	m_baseResourceGroupPathArgumentId( "base-resource-group-path" ),
	m_mergeResourceGroupPathArgumentId( "merge-resource-group-path" ),
	m_mergedResourceGroupDocumentVersionArgumentId( "--document-version" ),
	m_mergedResourceGroupOutputArgumentId( "--merge-output-resource-group-path" )
{
	AddRequiredPositionalArgument( m_baseResourceGroupPathArgumentId, "The path to the Resource Group to act as a base for the merge." );

	AddRequiredPositionalArgument( m_mergeResourceGroupPathArgumentId, "The path to the Resource Group to act as a target for the merge." );

	CarbonResources::ResourceGroupExportToFileParams defaultParams;

	AddArgument( m_mergedResourceGroupDocumentVersionArgumentId, "Document version for created resource group.", false, false, VersionToString( defaultParams.outputDocumentVersion ) );

	AddArgument( m_mergedResourceGroupOutputArgumentId, "The path in which to place the merged Resource Group.", false, false, defaultParams.filename.string() );
}

bool MergeResourceGroupCliOperation::Execute( std::string& returnErrorMessage ) const
{

	CarbonResources::ResourceGroupImportFromFileParams importParamsBase;

	std::optional<std::string> baseResourceGroupFilename = m_argumentParser->present<std::string>( m_baseResourceGroupPathArgumentId );
	if( !baseResourceGroupFilename.has_value() )
	{
		returnErrorMessage = "Failed to parse base Resource Group filename.";

		return false;
	}
	importParamsBase.filename = baseResourceGroupFilename.value();



	CarbonResources::ResourceGroupImportFromFileParams importParamsMerge;

	std::optional<std::string> mergeResourceGroupFilename = m_argumentParser->present<std::string>( m_mergeResourceGroupPathArgumentId );
	if( !mergeResourceGroupFilename.has_value() )
	{
		returnErrorMessage = "Failed to parse merge Resource Group filename.";

		return false;
	}
	importParamsMerge.filename = mergeResourceGroupFilename.value();

	CarbonResources::ResourceGroupExportToFileParams exportParams;

	exportParams.filename = m_argumentParser->get<std::string>( m_mergedResourceGroupOutputArgumentId );

	std::string version = m_argumentParser->get( m_mergedResourceGroupDocumentVersionArgumentId );

	CarbonResources::Version documentVersion;

	bool versionIsValid = ParseDocumentVersion( version, documentVersion );

	if( !versionIsValid )
	{
		returnErrorMessage = "Invalid document version";

		return false;
	}

	exportParams.outputDocumentVersion = documentVersion;

    if (ShowCliStatusUpdates())
    {
		PrintStartBanner( importParamsBase, importParamsMerge, exportParams, version );
    }

	return Merge( importParamsBase, importParamsMerge, exportParams );
}

void MergeResourceGroupCliOperation::PrintStartBanner( const CarbonResources::ResourceGroupImportFromFileParams& importParamsBase, const CarbonResources::ResourceGroupImportFromFileParams& importParamsMerge, CarbonResources::ResourceGroupExportToFileParams& exportParams, const std::string& version ) const
{
	std::cout << "---Merging Groups---" << std::endl;

	PrintCommonOperationHeaderInformation();

	std::cout << "Base Resource Group: " << importParamsBase.filename << std::endl;
	std::cout << "Merge Resource Group: " << importParamsMerge.filename << std::endl;
	std::cout << "Output merged Path: " << exportParams.filename << std::endl;
	std::cout << "Output Document Version: " << version << std::endl;

	std::cout << "----------------------------\n"
			  << std::endl;
}

bool MergeResourceGroupCliOperation::Merge( CarbonResources::ResourceGroupImportFromFileParams& importParamsBase, CarbonResources::ResourceGroupImportFromFileParams& importParamsMerge, CarbonResources::ResourceGroupExportToFileParams& exportParams ) const
{
	CarbonResources::StatusCallback statusCallback = GetStatusCallback();

	// Import base Resource Group
	CarbonResources::ResourceGroup baseResourceGroup;

    importParamsBase.callbackSettings.statusCallback = statusCallback;
	importParamsBase.callbackSettings.verbosityLevel = GetVerbosityLevel();

    if( ShowCliStatusUpdates() )
	{
		CliStatusUpdate( "Importing base Resource Group from file." );
	}

	CarbonResources::Result importBaseGroupResult = baseResourceGroup.ImportFromFile( importParamsBase );

	if( importBaseGroupResult.type != CarbonResources::ResultType::SUCCESS )
	{
		PrintCarbonResourcesError( importBaseGroupResult );

		return false;
	}

	// Import merge Resource Group
	CarbonResources::ResourceGroup mergeResourceGroup;

    importParamsMerge.callbackSettings.statusCallback = statusCallback;
	importParamsMerge.callbackSettings.verbosityLevel = GetVerbosityLevel();

    if( ShowCliStatusUpdates() )
	{
		CliStatusUpdate( "Importing merge Resource Group from file." );
	}

	CarbonResources::Result importMergeGroupResult = mergeResourceGroup.ImportFromFile( importParamsMerge );

	if( importMergeGroupResult.type != CarbonResources::ResultType::SUCCESS )
	{
		PrintCarbonResourcesError( importMergeGroupResult );

		return false;
	}

	// Perform merge
	CarbonResources::ResourceGroupMergeParams mergeParams;

	CarbonResources::ResourceGroup mergedResultResourceGroup;

	mergeParams.resourceGroupToMerge = &mergeResourceGroup;

	mergeParams.mergedResourceGroup = &mergedResultResourceGroup;

    mergeParams.callbackSettings.statusCallback = statusCallback;
	mergeParams.callbackSettings.verbosityLevel = GetVerbosityLevel();

    if( ShowCliStatusUpdates() )
	{
		CliStatusUpdate( "Merging Resource Groups." );
	}

	CarbonResources::Result mergeResult = baseResourceGroup.Merge( mergeParams );

	if( mergeResult.type != CarbonResources::ResultType::SUCCESS )
	{
		PrintCarbonResourcesError( mergeResult );

		return false;
	}

	// Export
	exportParams.callbackSettings.statusCallback = statusCallback;
	exportParams.callbackSettings.verbosityLevel = GetVerbosityLevel();

    if( ShowCliStatusUpdates() )
	{
		CliStatusUpdate( "Exporting resulting Resource Group to file." );
	}

	CarbonResources::Result exportResult = mergedResultResourceGroup.ExportToFile( exportParams );

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
