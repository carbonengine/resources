// Copyright © 2025 CCP ehf.

#pragma once

#include "CliOperation.h"
#include <BundleResourceGroup.h>

class UnpackBundleCliOperation : public CliOperation
{
public:
	UnpackBundleCliOperation();

	bool Execute( std::string& returnErrorMessage ) const final;

private:
	void PrintStartBanner( const CarbonResources::ResourceGroupImportFromFileParams& importParams, const CarbonResources::BundleUnpackParams& unpackParams ) const;

	bool Unpack( CarbonResources::ResourceGroupImportFromFileParams& importParams, CarbonResources::BundleUnpackParams& unpackParams ) const;

	std::string m_bundleResourceGroupPathArgumentId;
	std::string m_chunkSourceBasePathsArgumentId;
	std::string m_chunkSourceTypeArgumentId;
	std::string m_resourceDestinationBasePathArgumentId;
	std::string m_resourceDestinationTypeArgumentId;
};

