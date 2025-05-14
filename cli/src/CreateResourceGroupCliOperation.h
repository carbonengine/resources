/* 
	*************************************************************************

	CreateResourceGroupCliOperation.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/
#pragma once
#ifndef CreateResourceGroupCliOperation_H
#define CreateResourceGroupCliOperation_H

#include <filesystem>

#include "CliOperation.h"

class CreateResourceGroupCliOperation : public CliOperation
{
public:
	CreateResourceGroupCliOperation();

	virtual bool Execute() const final;

private:
    void PrintStartBanner( const std::filesystem::path& inputDirectory, const std::filesystem::path& resourceGroupOutputDirectory ) const;
	bool CreateResourceGroup( const std::filesystem::path& inputDirectory, const std::filesystem::path& resourceGroupOutputFile ) const;

private:

	std::string m_createResourceGroupPathArgumentId;

	std::string m_createResourceGroupOutputFileArgumentId;
};

#endif // CreateResourceGroupCliOperation_H