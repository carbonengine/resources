// Copyright © 2025 CCP ehf.

#pragma once
#ifndef ResourceTools_H
#define ResourceTools_H

#include <filesystem>
#include <list>
#include <string>

#include "Downloader.h"

namespace CryptoPP
{
    class HexEncoder;

    namespace Weak1
    {
	    class MD5;
    }
}



namespace ResourceTools
{

	class BundleStreamOut;

	struct ChunkMatch
	{
		uint64_t sourceOffset;
		uint64_t destinationOffset;
		uint64_t length;
	};

	bool GenerateMd5Checksum( const std::filesystem::path& path, std::string& checksum );

    bool GenerateMd5Checksum( const std::string& data, std::string& checksum );

	bool Md5ChecksumMatches( const std::filesystem::path& path, std::string& checksum );

    bool GenerateFowlerNollVoChecksum( const std::string& input, std::string& checksum );

	std::list<ChunkMatch> FindMatchingChunks( const std::string& source, std::string& destination );

    bool FindMatchingChunk( const std::string& chunk, std::filesystem::path filePath, size_t& chunkOffset );

	size_t CountMatchingChunks( const std::filesystem::path& fileA, size_t offsetA, std::filesystem::path fileB, size_t offsetB, size_t chunkSize );

    bool GetLocalFileData( const std::filesystem::path& filepath, std::string& data );

    bool GZipCompressData( const std::string& dataToCompress, std::string& compressedData );

    bool GZipUncompressData( const std::string& dataToUncompress, std::string& uncompressedData );

    bool SaveFile( const std::filesystem::path& path, const std::string& data );

    unsigned int CalculateBinaryOperation( const std::filesystem::path& path );
}

#endif // ResourceTools_H