// Copyright © 2025 CCP ehf.

#pragma once

#include "CliOperation.h"
#include <PatchResourceGroup.h>

class ApplyPatchCliOperation : public CliOperation
{
public:
	ApplyPatchCliOperation();

	bool Execute( std::string& returnErrorMessage ) const final;

private:
	void PrintStartBanner( const CarbonResources::ResourceGroupImportFromFileParams& importParamsPrevious, const CarbonResources::PatchApplyParams patchApplyParams ) const;

	bool ApplyPatch( CarbonResources::ResourceGroupImportFromFileParams& importParamsPrevious, CarbonResources::PatchApplyParams patchApplyParams ) const;

	std::string m_patchResourceGroupPathArgumentId;
	std::string m_patchBinariesSourceBasePathsArgumentId;
	std::string m_patchBinariesSourceTypeArgumentId;
	std::string m_resourcesToPatchSourceBasePathsArgumentId;
	std::string m_resourcesToPatchSourceTypeArgumentId;
	std::string m_nextResourcesBasePathsArgumentId;
	std::string m_nextResourcesSourceTypeArgumentId;
	std::string m_resourcesToPatchDestinationPathArgumentId;
	std::string m_resourcesToPatchDestinationTypeArgumentId;
};
