/* 
	*************************************************************************

	FileDataStreamOut.h

	Author:    James Hawk
	Created:   March. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/

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
		FileDataStreamOut( );

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