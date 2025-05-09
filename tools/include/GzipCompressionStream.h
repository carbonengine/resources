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
    class GzipCompressionStream
	{
	public:
		GzipCompressionStream( std::string* out);

		~GzipCompressionStream();

        bool Start();

		bool operator<<( std::string* toCompress );

        bool Finish();


	private:

        bool m_compressionInProgress;
        z_stream m_stream;
    	std::string m_buffer;
    	std::string* m_out;

    	bool ProcessBuffer( bool finish );
	};


}

#endif // GzipCompressionStream_H