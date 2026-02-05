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

// -------------------------------------------------------------
// Description:
//   CurrentWorkingDirectoryChanger is a RAII helper class that changes
//   the current working directory to a specified path on construction
//   and restores the original working directory once destructed.
//   As long as the CurrentWorkingDirectoryChanger object is in scope,
//   the working directory remains changed to its new value.
// -------------------------------------------------------------
class CurrentWorkingDirectoryChanger
{
public:
	// Constructor saves the current working directory and changes it to the new path
	explicit CurrentWorkingDirectoryChanger( const std::filesystem::path& new_path ) :
		m_original_path( std::filesystem::current_path() )
	{
		try
		{
			std::cout << "WorkingDirectory - before change: " << m_original_path.generic_string() << std::endl;
			std::filesystem::current_path( new_path );
			std::cout << "WorkingDirectory - after change: " << std::filesystem::current_path().generic_string()
					  << std::endl;
		}
		catch( const std::filesystem::filesystem_error& e )
		{
			std::cerr << "Error changing working directory: " << e.what() << std::endl;
		}
	}

	// Destructor restores working directory to the original path
	~CurrentWorkingDirectoryChanger()
	{
		try
		{
			std::filesystem::current_path( m_original_path );
			std::cout << "WorkingDirectory - restored to: " << m_original_path.generic_string() << std::endl;
		}
		catch( const std::filesystem::filesystem_error& e )
		{
			std::cerr << "Error restoring working directory: " << e.what() << std::endl;
		}
	}

	// Disable copy and move operations
	CurrentWorkingDirectoryChanger( const CurrentWorkingDirectoryChanger& ) = delete;

	CurrentWorkingDirectoryChanger& operator=( const CurrentWorkingDirectoryChanger& ) = delete;

	CurrentWorkingDirectoryChanger( CurrentWorkingDirectoryChanger&& ) = delete;

	CurrentWorkingDirectoryChanger& operator=( CurrentWorkingDirectoryChanger&& ) = delete;

private:
	// Store the original working directory path to restore upon class destruction
	std::filesystem::path m_original_path;
};

#endif // CarbonResourcesTestFixture_H