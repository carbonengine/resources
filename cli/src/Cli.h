// Copyright © 2025 CCP ehf.

#pragma once
#ifndef Cli_H
#define Cli_H

#include <string>
#include <vector>

#include "CliOperation.h"

namespace argparse
{
class ArgumentParser;
}

class Cli
{
public:
	Cli( const std::string& name, const std::string& version );

	~Cli();

	void AddOperation( CliOperation* operation );

	void PrintError();

	int ProcessCommandLine( int argc, char** argv );

private:
	void PrintCliHeader();

private:
	std::string m_name;

	std::string m_version;

	std::vector<CliOperation*> m_operations;
};

#endif // Cli_H