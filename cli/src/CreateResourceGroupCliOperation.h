// Copyright © 2025 CCP ehf.

#pragma once
#ifndef CreateResourceGroupCliOperation_H
#define CreateResourceGroupCliOperation_H

#include <filesystem>

#include "CliOperation.h"

#include <ResourceGroup.h>

class CreateResourceGroupCliOperation : public CliOperation
{
public:
	CreateResourceGroupCliOperation();

	virtual bool Execute( std::string& returnErrorMessage ) const final;

private:
	void PrintStartBanner( CarbonResources::CreateResourceGroupFromDirectoryParams& createResourceGroupFromDirectoryParams, CarbonResources::ResourceGroupExportToFileParams& ResourceGroupExportToFileParams ) const;

    bool CreateResourceGroup( CarbonResources::CreateResourceGroupFromDirectoryParams& createResourceGroupFromDirectoryParams, CarbonResources::ResourceGroupExportToFileParams& ResourceGroupExportToFileParams ) const;

private:
	std::string m_createResourceGroupPathArgumentId;

	std::string m_createResourceGroupOutputFileArgumentId;

	std::string m_createResourceGroupDocumentVersionArgumentId;

	std::string m_createResourceGroupResourcePrefixArgumentId;
	
    std::string m_createResourceGroupSkipCompressionCalculationId;

    std::string m_createResourceGroupExportResourcesId;

    std::string m_createResourceGroupExportResourcesDestinationTypeId;
		
    std::string m_createResourceGroupExportResourcesDestinationPathId;
};

#endif // CreateResourceGroupCliOperation_H