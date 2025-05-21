#pragma once

#include "CliOperation.h"
#include <PatchResourceGroup.h>

class ApplyPatchCliOperation : public CliOperation
{
public:
	ApplyPatchCliOperation();
	bool Execute() const final;
private:
	void PrintStartBanner(const CarbonResources::ResourceGroupImportFromFileParams& importParamsPrevious, const CarbonResources::PatchApplyParams patchApplyParams) const;
	bool ApplyPatch(const CarbonResources::ResourceGroupImportFromFileParams& importParamsPrevious, const CarbonResources::PatchApplyParams patchApplyParams) const;
	std::string m_patchResourceGroupPathArgumentId;
	std::string m_patchBinariesBasePathsArgumentId;
	std::string m_patchBinariesSourceTypeArgumentId;
	std::string m_resourcesToPatchBasePathsArgumentId;
	std::string m_resourcesToPatchSourceTypeArgumentId;
	std::string m_nextResourcesBasePathArgumentId;
	std::string m_nextResourcesSourceTypeArgumentId;
	std::string m_outputBasePathArgumentId;
	std::string m_outputDestinationTypeArgumentId;
};

