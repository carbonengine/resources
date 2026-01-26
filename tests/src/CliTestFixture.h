// Copyright © 2025 CCP ehf.

#pragma once
#ifndef CliTestFixture_H
#define CliTestFixture_H

#include <vector>
#include <string>

#include "ResourcesTestFixture.h"

struct CliTestFixture : public ResourcesTestFixture
{

	int RunCli( std::vector<std::string>& arguments, std::string& output );

	void CleanupTestOutputFiles( const std::vector<std::filesystem::path>& filesToRemove );
};

#endif // CliTestFixture_H