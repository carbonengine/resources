/* 
	*************************************************************************

	FileDataStreamIn.h

	Author:    James Hawk
	Created:   March. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/

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
		FileDataStreamIn( unsigned long chunkSize = -1);

		~FileDataStreamIn();

        void Finish();

        bool IsFinished();

        bool StartRead( std::filesystem::path filepath );

        size_t GetCurrentPosition();

		bool operator>>( std::string& data );


	private:

		bool m_readInProgress;

		unsigned long m_chunkSize;

        std::ifstream m_inputStream;

        size_t m_currentPosition;

        size_t m_fileSize;
	};

}

#endif // FileDataStreamIn_H