// Copyright © 2025 CCP ehf.

#pragma once
#ifndef FileDataStreamOut_H
#define FileDataStreamOut_H

#include <filesystem>
#include <string>
#include <fstream>

namespace ResourceTools
{

class FileDataStreamOut
{
public:
	FileDataStreamOut();

	virtual ~FileDataStreamOut();

	virtual bool Finish();

	bool IsFinished();

	virtual bool StartWrite( std::filesystem::path filepath );

	bool operator<<( const std::string& data );

	size_t GetFileSize();


private:
	bool m_writeInProgress;

	std::ofstream m_outputStream;

	size_t m_fileSize;
};


}

#endif // FileDataStreamOut_H