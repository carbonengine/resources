// Copyright © 2025 CCP ehf.

#pragma once
#ifndef CreatePatchCliOperation_H
#define CreatePatchCliOperation_H


#include <filesystem>

#include "CliOperation.h"

#include <ResourceGroup.h>

class CreatePatchCliOperation : public CliOperation
{
public:

	CreatePatchCliOperation();

	virtual bool Execute( std::string& returnErrorMessage ) const override;

private:

    void PrintStartBanner( const CarbonResources::ResourceGroupImportFromFileParams& previousResourceGroupParams, const CarbonResources::ResourceGroupImportFromFileParams& nextResourceGroupParams, CarbonResources::PatchCreateParams& createPatchParams ) const;

	bool CreatePatch( CarbonResources::ResourceGroupImportFromFileParams& previousResourceGroupParams, CarbonResources::ResourceGroupImportFromFileParams& nextResourceGroupParams, CarbonResources::PatchCreateParams& createPatchParams ) const;

private:

	std::string m_previousResourceGroupPathArgumentId;

	std::string m_nextResourceGroupPathArgumentId;

    std::string m_resourceGroupRelativePathArgumentId;

    std::string m_patchResourceGroupRelativePathArgumentId;

    std::string m_resourceSourceTypePreviousArgumentId;

    std::string m_resourceSourceBasePathPreviousArgumentId;

    std::string m_resourceSourceTypeNextArgumentId;

	std::string m_resourceSourceBasePathNextArgumentId;

    std::string m_patchBinaryDestinationTypeArgumentId;

	std::string m_patchBinaryDestinationBasePathArgumentId;

    std::string m_patchResourceGroupDestinationTypeArgumentId;

	std::string m_patchResourceGroupDestinationBasePathArgumentId;

    std::string m_patchFileRelativePathPrefixArgumentId;

    std::string m_maxInputChunkSizeArgumentId;

	std::string m_downloadRetrySecondsArgumentId;

	std::string m_indexFolderArgumentId;
};

#endif // CreatePatchCliOperation_H