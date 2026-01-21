// Copyright © 2025 CCP ehf.

#pragma once
#ifndef ResourceFilterTest_H
#define ResourceFilterTest_H

#include "ResourcesTestFixture.h"
#include <filesystem>

// Inherit from ResourcesTestFixture to gain access to file and directory helper functions
class ResourceFilterTest : public ResourcesTestFixture
{
};

// RAII helper class to change the current working directory temporarily (within a scope)
class CurrentWorkingDirectoryChanger {
public:
	// Constructor acquires the current path and changes it
	explicit CurrentWorkingDirectoryChanger(const std::filesystem::path& new_path) :
		original_path_(std::filesystem::current_path())
	{
		// Acquire original path
		try
		{
			std::cout << "CurrentWorkingDirectoryChanger - Original directory: " << original_path_.generic_string() << std::endl;
			std::filesystem::current_path(new_path); // Change to new path
			std::cout << "CurrentWorkingDirectoryChanger - Changed directory to: " << std::filesystem::current_path().generic_string() << std::endl;
		}
		catch (const std::filesystem::filesystem_error& e)
		{
			std::cerr << "CurrentWorkingDirectoryChanger - Error changing directory: " << e.what() << std::endl;
			// Handle error, maybe throw an exception or set a flag
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

	// Disable copy and move operations to ensure single ownership and prevent issues
	CurrentWorkingDirectoryChanger(const CurrentWorkingDirectoryChanger&) = delete;

	CurrentWorkingDirectoryChanger& operator=(const CurrentWorkingDirectoryChanger&) = delete;

	CurrentWorkingDirectoryChanger(CurrentWorkingDirectoryChanger&&) = delete;

	CurrentWorkingDirectoryChanger& operator=(CurrentWorkingDirectoryChanger&&) = delete;

private:
	std::filesystem::path original_path_;
};

#endif // ResourceFilterTest_H