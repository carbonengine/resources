// Copyright © 2025 CCP ehf.

#pragma once
#ifndef CarbonResourcesTestFixture_H
#define CarbonResourcesTestFixture_H

#include <vector>

#include <gtest/gtest.h>

#include <filesystem>

struct ResourcesTestFixture : public ::testing::Test
{
	void SetUp();

	void TearDown();

	std::filesystem::path GetTestFileFileAbsolutePath( const std::filesystem::path& relativePath );

	bool FileExists( const std::filesystem::path& filePath );

	bool FilesMatch( const std::filesystem::path& file1Path, const std::filesystem::path& file2Path );

	bool DirectoryIsSubset( const std::filesystem::path& dir1, const std::filesystem::path& dir2 ); // Test that all files in dir1 exist in dir2, and the contents of the files in both directories are the same.
};

// RAII helper class to change the current working directory temporarily (within a scope)
class CurrentWorkingDirectoryChanger {
public:
	// Constructor acquires the current path and changes it
	explicit CurrentWorkingDirectoryChanger(const std::filesystem::path& new_path) :
		original_path_(std::filesystem::current_path())
	{
		try
		{
			std::cout << "CurrentWorkingDirectoryChanger - Original directory: " << original_path_.generic_string() << std::endl;
			std::filesystem::current_path(new_path); // Change to new path
			std::cout << "CurrentWorkingDirectoryChanger - Changed directory to: " << std::filesystem::current_path().generic_string() << std::endl;
		}
		catch (const std::filesystem::filesystem_error& e)
		{
			std::cerr << "CurrentWorkingDirectoryChanger - Error changing directory: " << e.what() << std::endl;
		}
	}

	// Destructor restores the original path
	~CurrentWorkingDirectoryChanger()
	{
		try
		{
			std::filesystem::current_path(original_path_); // Restore original path
			std::cout << "CurrentWorkingDirectoryChanger - Restored directory to: " << std::filesystem::current_path().generic_string() << std::endl;
		}
		catch (const std::filesystem::filesystem_error& e)
		{
			std::cerr << "CurrentWorkingDirectoryChanger - Error restoring directory: " << e.what() << std::endl;
		}
	}

	// Disable copy and move operations
	CurrentWorkingDirectoryChanger(const CurrentWorkingDirectoryChanger&) = delete;

	CurrentWorkingDirectoryChanger& operator=(const CurrentWorkingDirectoryChanger&) = delete;

	CurrentWorkingDirectoryChanger(CurrentWorkingDirectoryChanger&&) = delete;

	CurrentWorkingDirectoryChanger& operator=(CurrentWorkingDirectoryChanger&&) = delete;

private:
	std::filesystem::path original_path_;
};

#endif // CarbonResourcesTestFixture_H