// Copyright © 2025 CCP ehf.

#pragma once
#ifndef FileDataStreamIn_H
#define FileDataStreamIn_H

#include <filesystem>
#include <string>
#include <fstream>

namespace ResourceTools
{

class FileDataStreamIn
{
public:
	FileDataStreamIn( uintmax_t chunkSize = -1 );

	~FileDataStreamIn();

	void Finish();

	bool IsFinished();

	bool StartRead( std::filesystem::path filepath );

	std::filesystem::path GetPath();

	size_t GetCurrentPosition();

	bool ReadBytes( size_t n, std::string& data );

	void Seek( size_t position );

	size_t Size();

	bool operator>>( std::string& data );


private:
	bool m_readInProgress;

	uintmax_t m_chunkSize;

	std::ifstream m_inputStream;

	size_t m_currentPosition;

	size_t m_fileSize;

	std::filesystem::path m_path;
};

}

#endif // FileDataStreamIn_H