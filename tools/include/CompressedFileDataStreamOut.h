/* 
	*************************************************************************

	CompressedFileDataStreamOut.h

	Author:    James Hawk
	Created:   June. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/

#pragma once
#ifndef CompressedFileDataStreamOut_H
#define CompressedFileDataStreamOut_H

#include <filesystem>
#include <string>
#include <fstream>

#include "FileDataStreamOut.h"

#include "GzipCompressionStream.h"

namespace ResourceTools
{

    class CompressedFileDataStreamOut : public FileDataStreamOut
	{
	public:

		CompressedFileDataStreamOut( );

		virtual ~CompressedFileDataStreamOut();

        bool StartWrite( std::filesystem::path filepath ) override;

        virtual bool Finish() override;

        bool operator<<( const std::string& data );

	private:

        std::string m_compressionBuffer;

        std::unique_ptr<GzipCompressionStream> m_compressionStream;
	};

    
}

#endif // CompressedFileDataStreamOut_H