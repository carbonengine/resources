// Copyright © 2025 CCP ehf.

#pragma once

#include "CliOperation.h"
#include <ResourceGroup.h>

class MergeResourceGroupCliOperation : public CliOperation
{
public:
	MergeResourceGroupCliOperation();

	bool Execute( std::string& returnErrorMessage ) const final;

private:
	void PrintStartBanner( const CarbonResources::ResourceGroupImportFromFileParams& importParamsBase, const CarbonResources::ResourceGroupImportFromFileParams& importParamsMerge, CarbonResources::ResourceGroupExportToFileParams& exportParams, const std::string& version ) const;

	bool Merge( CarbonResources::ResourceGroupImportFromFileParams& importParamsBase, CarbonResources::ResourceGroupImportFromFileParams& importParamsMerge, CarbonResources::ResourceGroupExportToFileParams& exportParams ) const;

	std::string m_baseResourceGroupPathArgumentId;
	std::string m_mergeResourceGroupPathArgumentId;
	std::string m_mergedResourceGroupOutputArgumentId;
	std::string m_mergedResourceGroupDocumentVersionArgumentId;
};
