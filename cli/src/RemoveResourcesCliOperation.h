// Copyright © 2025 CCP ehf.

#pragma once

#include "CliOperation.h"
#include <ResourceGroup.h>

class RemoveResourcesCliOperation : public CliOperation
{
public:
	RemoveResourcesCliOperation();

	bool Execute( std::string& returnErrorMessage ) const final;

private:
	void PrintStartBanner( const CarbonResources::ResourceGroupImportFromFileParams& importParams, std::filesystem::path& resourcesToRemoveFile, CarbonResources::ResourceGroupExportToFileParams& exportParams, bool ignoreMissingResources, const std::string& version ) const;

	bool RemoveResources( const CarbonResources::ResourceGroupImportFromFileParams& importParams, std::filesystem::path& resourcesToRemoveFile, CarbonResources::ResourceGroupExportToFileParams& exportParams, bool ignoreMissingResources ) const;

	bool ReadResourcesToRemoveFile( std::filesystem::path& pathToResourcesToRemoveFile, std::vector<std::filesystem::path>& resourcesToRemoveOut ) const;

	std::string m_resourceGroupPathArgumentId;
	std::string m_resourceListPath;
	std::string m_outputResourceGroupPath;
	std::string m_outputResourceGroupDocumentVersionArgumentId;
	std::string m_ignoreMissingResources;
};
