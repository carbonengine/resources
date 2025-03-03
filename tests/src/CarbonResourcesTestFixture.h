/* 
	*************************************************************************

	CarbonResourcesTestFixture.h

	Author:    James Hawk
	Created:   February. 2025
	Project:   Carbon-Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/
#pragma once
#ifndef CarbonResourcesTestFixture_H
#define CarbonResourcesTestFixture_H

#include <vector>

#include <gtest/gtest.h>

#include "Paths.h"

struct CarbonResourcesTestFixture : public ::testing::Test
{
	void SetUp();

	void TearDown();

    bool BundleIsValid();

    bool PatchIsValid();

    std::string GetTestFileFileAbsolutePath( const std::string& relativePath );

    bool FilesMatch( const std::string& file1Path, const std::string& file2Path );
};

#endif // CarbonResourcesTestFixture_H