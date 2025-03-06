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

#include <filesystem>

struct CarbonResourcesTestFixture : public ::testing::Test
{
	void SetUp();

	void TearDown();

    bool BundleIsValid();

    bool PatchIsValid();

    std::filesystem::path GetTestFileFileAbsolutePath( const std::filesystem::path& relativePath );

    bool FilesMatch( const std::filesystem::path& file1Path, const std::filesystem::path& file2Path );
};

#endif // CarbonResourcesTestFixture_H