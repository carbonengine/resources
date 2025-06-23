/* 
	*************************************************************************

	CreateBundleCliOperation.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/
#pragma once
#ifndef CreateBundleCliOperation_H
#define CreateBundleCliOperation_H

#include <filesystem>

#include "CliOperation.h"

#include <ResourceGroup.h>

class CreateBundleCliOperation : public CliOperation
{
public:
	CreateBundleCliOperation();

	virtual bool Execute( std::string& returnErrorMessage ) const override;

private:
    void PrintStartBanner( const CarbonResources::ResourceGroupImportFromFileParams& resourceGroupParams, CarbonResources::BundleCreateParams bundleCreateParams ) const;
	bool CreateBundle( CarbonResources::ResourceGroupImportFromFileParams& resourceGroupParams, CarbonResources::BundleCreateParams bundleCreateParams, std::string& returnErrorMessage ) const;

private:
	void CreateResourceGroupFromFileType();

private:
	std::string m_inputResourceGroupPathArgumentId;

	std::string m_resourceGroupRelativePathArgumentId;

    std::string m_bundleResourceGroupRelativePathArgumentId;

    std::string m_resourceSourceTypeArgumentId;

    std::string m_resourceSourceBasePathArgumentId;

    std::string m_chunkDestinationTypeArgumentId;

	std::string m_chunkDestinationBasePathArgumentId;

    std::string m_bundleResourceGroupDestinationTypeArgumentId;

	std::string m_bundleResourceGroupDestinationBasePathArgumentId;

    std::string m_chunkSizeArgumentId;

	std::string m_downloadRetrySecondsArgumentId;
};

#endif // CreateBundleCliOperation_H