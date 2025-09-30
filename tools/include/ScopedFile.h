// Copyright © 2025 CCP ehf.

#pragma once
#ifndef ScopedFile_H
#define ScopedFile_H

#include <filesystem>


namespace ResourceTools
{
class ScopedFile
{
public:
	ScopedFile( std::filesystem::path location );

	~ScopedFile();

private:
	std::filesystem::path m_location;
};
}

#endif // ScopedFile_H