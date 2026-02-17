// Copyright © 2025 CCP ehf.

#pragma once
#ifndef CliTestFixture_H
#define CliTestFixture_H

#include <vector>
#include <string>

#include "ResourcesTestFixture.h"

struct CliTestFixture : public ResourcesTestFixture
{

	int RunCli( std::vector<std::string>& arguments, std::string* standardOutput = nullptr, std::string* errorOutput = nullptr );

	// Helper to remove files as part of a test run and cleanup
	void RemoveFiles( const std::vector<std::filesystem::path>& filesToRemove );
};

#endif // CliTestFixture_H