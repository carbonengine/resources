/* 
	*************************************************************************

	GzipCompressionStream.h

	Author:    James Hawk
	Created:   March. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/

#pragma once
#ifndef GzipCompressionStream_H
#define GzipCompressionStream_H

#include <string>
#include <zlib.h>

namespace ResourceTools
{

    struct CompressionChunk
    {
		std::string* uncompressedData;

        std::string* compressedData;
    };

    class GzipCompressionStream
	{
	public:
		GzipCompressionStream( );

		~GzipCompressionStream();

        bool Start();

		bool operator<<( CompressionChunk& compressionChunk );

        bool Finish();


	private:

        bool m_compressionInProgress;
        z_stream m_stream;
	};


}

#endif // GzipCompressionStream_H