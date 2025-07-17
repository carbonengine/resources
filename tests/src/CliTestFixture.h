// Copyright © 2025 CCP ehf.

#pragma once
#ifndef CliTestFixture_H
#define CliTestFixture_H

#include <vector>
#include <string>

#include "CarbonResourcesTestFixture.h"

struct CliTestFixture : public CarbonResourcesTestFixture
{

    int RunCli( std::vector<std::string> &arguments, std::string &output );

};

#endif // CliTestFixture_H