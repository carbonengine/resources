// Copyright © 2025 CCP ehf.

#include "DiffResourceGroupCliOperation.h"

#include <iostream>
#include <argparse/argparse.hpp>
#include <fstream>

DiffResourceGroupCliOperation::DiffResourceGroupCliOperation() :
	CliOperation( "diff-group", "Outputs a list of additions and subtractions between the two provided ResourceGroups." ),
	m_baseResourceGroupPathArgumentId( "base-resource-group-path" ),
	m_diffResourceGroupPathArgumentId( "diff-resource-group-path" ),
	m_diffOutputPath( "--diff-output-path" )
{
	AddRequiredPositionalArgument( m_baseResourceGroupPathArgumentId, "The path to the Resource Group to act as a base for the diff." );

	AddRequiredPositionalArgument( m_diffResourceGroupPathArgumentId, "The path to the Resource Group to act as a target for the diff." );

	AddArgument( m_diffOutputPath, "The path in which to place diff output.", false, false, "Diff.txt" );
}

bool DiffResourceGroupCliOperation::Execute( std::string& returnErrorMessage ) const
{
	CarbonResources::ResourceGroupImportFromFileParams importParamsBase;

	std::optional<std::string> filename = m_argumentParser->present<std::string>( m_baseResourceGroupPathArgumentId );
	if( !filename.has_value() )
	{
		returnErrorMessage = "Failed to parse resource group path";

		return false;
	}
	importParamsBase.filename = filename.value();

	CarbonResources::ResourceGroupImportFromFileParams importParamsDiff;

	std::optional<std::string> filenameDiff = m_argumentParser->present<std::string>( m_diffResourceGroupPathArgumentId );
	if( !filename.has_value() )
	{
		returnErrorMessage = "Failed to parse resource group path";

		return false;
	}
	importParamsDiff.filename = filenameDiff.value();


	std::filesystem::path outputPath = m_argumentParser->get( m_diffOutputPath );


	PrintStartBanner( importParamsBase, importParamsDiff, outputPath );

	return Diff( importParamsBase, importParamsDiff, outputPath );
}



void DiffResourceGroupCliOperation::PrintStartBanner( const CarbonResources::ResourceGroupImportFromFileParams& importParamsBase, const CarbonResources::ResourceGroupImportFromFileParams& importParamsDiff, std::filesystem::path& outputPath ) const
{
	if( s_verbosityLevel == CarbonResources::StatusLevel::OFF )
	{
		return;
	}

	std::cout << "---Running Diff---" << std::endl;

	PrintCommonOperationHeaderInformation();

	std::cout << "Base Resource Group: " << importParamsBase.filename << std::endl;
	std::cout << "Diff Resource Group: " << importParamsDiff.filename << std::endl;
	std::cout << "Diff Output path: " << outputPath.string() << std::endl;

	std::cout << "----------------------------\n"
			  << std::endl;
}

bool DiffResourceGroupCliOperation::Diff( const CarbonResources::ResourceGroupImportFromFileParams& importParamsBase, const CarbonResources::ResourceGroupImportFromFileParams& importParamsDiff, std::filesystem::path& outputPath ) const
{
	CarbonResources::StatusCallback statusCallback = GetStatusCallback();

	if( statusCallback )
	{
		statusCallback( CarbonResources::StatusLevel::OVERVIEW, CarbonResources::StatusProgressType::PERCENTAGE, 0, "Calculating Diff." );
	}

	// Import base resource group
	CarbonResources::ResourceGroup baseResourceGroup;

	CarbonResources::Result importBaseGroupResult = baseResourceGroup.ImportFromFile( importParamsBase );

	if( importBaseGroupResult.type != CarbonResources::ResultType::SUCCESS )
	{
		PrintCarbonResourcesError( importBaseGroupResult );

		return false;
	}

	// Import diff resource group
	CarbonResources::ResourceGroup diffResourceGroup;

	CarbonResources::Result importDiffGroupResult = diffResourceGroup.ImportFromFile( importParamsDiff );

	if( importDiffGroupResult.type != CarbonResources::ResultType::SUCCESS )
	{
		PrintCarbonResourcesError( importDiffGroupResult );

		return false;
	}

	// Run diff
	std::vector<std::filesystem::path> additions;

	std::vector<std::filesystem::path> subtractions;

	CarbonResources::ResourceGroupDiffAgainstGroupParams diffParams;

	diffParams.resourceGroupToDiffAgainst = &baseResourceGroup;

	diffParams.additions = &additions;

	diffParams.subtractions = &subtractions;

	CarbonResources::Result diffResult = diffResourceGroup.DiffAgainstGroup( diffParams );

	if( diffResult.type != CarbonResources::ResultType::SUCCESS )
	{
		PrintCarbonResourcesError( diffResult );

		return false;
	}

	// Output the results to file
	std::ofstream out;

	out.open( outputPath, std::ios::out | std::ios::binary );

	if( !out )
	{
		std::string out;

		std::cerr << "Failed to open output file for writing: " << outputPath << std::endl;

		return false;
	}

	for( auto addition : additions )
	{
		out << "+ " << addition.string() << "\n";
	}

	for( auto subtraction : subtractions )
	{
		out << "- " << subtraction.string() << "\n";
	}

	out.close();


	if( statusCallback )
	{
		statusCallback( CarbonResources::StatusLevel::OVERVIEW, CarbonResources::StatusProgressType::PERCENTAGE, 100, "Diff complete" );
	}

	return true;
}
