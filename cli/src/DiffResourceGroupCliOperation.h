// Copyright © 2025 CCP ehf.

#pragma once

#include "CliOperation.h"
#include <PatchResourceGroup.h>

class DiffResourceGroupCliOperation : public CliOperation
{
public:
	DiffResourceGroupCliOperation();

	bool Execute( std::string& returnErrorMessage ) const final;

private:
	void PrintStartBanner( const CarbonResources::ResourceGroupImportFromFileParams& importParamsBase, const CarbonResources::ResourceGroupImportFromFileParams& importParamsDiff, std::filesystem::path& outputPath ) const;

	bool Diff( CarbonResources::ResourceGroupImportFromFileParams& importParamsBase, CarbonResources::ResourceGroupImportFromFileParams& importParamsDiff, std::filesystem::path& outputPath ) const;

	std::string m_baseResourceGroupPathArgumentId;
	std::string m_diffResourceGroupPathArgumentId;
	std::string m_diffOutputPath;
};
