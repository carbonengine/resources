/* 
	*************************************************************************

	CliTestFixture.h

	Author:    James Hawk
	Created:   February. 2025
	Project:   Carbon-Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/
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