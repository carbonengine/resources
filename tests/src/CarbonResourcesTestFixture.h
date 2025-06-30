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

#include <filesystem>

struct CarbonResourcesTestFixture : public ::testing::Test
{
	void SetUp();

	void TearDown();

    std::filesystem::path GetTestFileFileAbsolutePath( const std::filesystem::path& relativePath );

    bool FilesMatch( const std::filesystem::path& file1Path, const std::filesystem::path& file2Path );

	bool DirectoryIsSubset( const std::filesystem::path& dir1, const std::filesystem::path& dir2 ); // Test that all files in dir1 exist in dir2, and the contents of the files in both directories are the same.
};

#endif // CarbonResourcesTestFixture_H