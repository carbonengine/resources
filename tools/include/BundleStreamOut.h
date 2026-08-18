// Copyright © 2025 CCP ehf.

#pragma once
#ifndef BundleStreamOut_H
#define BundleStreamOut_H

#include <FileDataStreamIn.h>
#include <FileDataStreamOut.h>
#include <ScopedFile.h>
#include <Md5ChecksumStream.h>
#include <string>

#include "GzipCompressionStream.h"

namespace ResourceTools
{

struct ChunkInfo
{
	std::string checksum = "";

    uintmax_t uncompressedSize = 0;

	uintmax_t compressedSize = 0;

    std::filesystem::path path = "";
};

class BundleStreamOut
{
public:
	BundleStreamOut( uintmax_t chunkSize, std::filesystem::path outputDirectory, bool outputCompressed, bool splitOnCompressed, bool calculateCompressions );

	~BundleStreamOut();

	bool operator<<( std::shared_ptr<ResourceTools::FileDataStreamIn> streamIn );

	bool GetChunkInfo( int index, ChunkInfo& chunkInfo );

	bool Finish();

    size_t GetNumberOfChunksCreated();

private:
    bool Flush();

    bool InitializeOutputStream();

	uintmax_t m_chunkSize;

	std::string m_cache;

	std::string m_uncompressedData;

	std::string m_compressedData;

	std::unique_ptr<GzipCompressionStream> m_compressionStream;

	std::unique_ptr<ResourceTools::FileDataStreamOut> m_chunkOut;

	std::filesystem::path m_outputDirectory;

    std::unique_ptr<ResourceTools::Md5ChecksumStream> m_checksumStream;

    bool m_outputCompressed;

    std::vector<ChunkInfo> m_chunkInfos;

    ChunkInfo m_currentChunk;

    bool m_splitOnCompressedSize;

    bool m_calculateCompressions;

};

}

#endif // BundleStreamOut_H