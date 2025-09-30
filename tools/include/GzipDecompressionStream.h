// Copyright © 2025 CCP ehf.

#pragma once
#ifndef GzipDecompressionStream_H
#define GzipDecompressionStream_H

#include <string>
#include <zlib.h>

namespace ResourceTools
{
class GzipDecompressionStream
{
public:
	GzipDecompressionStream( std::string* out );

	~GzipDecompressionStream();

	bool Start();

	bool operator<<( std::string* toDecompress );

	bool Finish();


private:
	bool m_decompressionInProgress;
	z_stream m_stream;
	std::string m_buffer;
	std::string* m_out;

	bool ProcessBuffer( bool finish );
};


}

#endif // GzipDecompressionStream_H