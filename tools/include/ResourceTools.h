/* 
	*************************************************************************

	ResourceTools.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/

#pragma once
#ifndef ResourceTools_H
#define ResourceTools_H

#include <memory>
#include <string>
#include <filesystem>
#include <fstream>
#include <list>
#include <sstream>

namespace CryptoPP
{
    class HexEncoder;

    namespace Weak1
    {
	    class MD5;
    }
}


#include <curl/curl.h>

namespace ResourceTools
{

    // TODO expect all these signatures to move around, just made off the top of my head so far
    // Not settled on correct approach to manage data etc

	class BundleStreamOut;
	struct RollingChecksum
	{
		uint64_t alpha;
		uint64_t beta;
		uint64_t checksum;
	};

	struct ChunkMatch
	{
		uint64_t sourceOffset;
		uint64_t destinationOffset;
		uint64_t length;
	};

	// A utility class for downloading files.
	// Reuse is encouraged for multiple downloads, but do not share across threads.
	class Downloader
	{
		public:
			Downloader();
			~Downloader();
			bool DownloadFile( const std::string& url, const std::filesystem::path& outputPath );
	private:
		CURL* m_curlHandle{ nullptr };
	};

    bool GenerateMd5Checksum( const std::string& data, std::string& checksum );

    bool GenerateFowlerNollVoChecksum( const std::string& input, std::string& checksum );

	// Generate a weak checksum using the rsync algorithm https://rsync.samba.org/tech_report/node3.html
	RollingChecksum GenerateRollingAdlerChecksum( const std::string& input, uint64_t start, uint64_t end );

	// Generate a weak checksum using the rsync algorithm https://rsync.samba.org/tech_report/node3.html
	RollingChecksum GenerateRollingAdlerChecksum( const std::string& input, uint64_t start, uint64_t end, RollingChecksum previous );

	std::list<ChunkMatch> FindMatchingChunks( const std::string& source, std::string& destination );

    bool FindMatchingChunk( const std::string& chunk, std::filesystem::path filePath, size_t& chunkOffset );

	size_t CountMatchingChunks( const std::filesystem::path& fileA, size_t offsetA, std::filesystem::path fileB, size_t offsetB, size_t chunkSize );

    bool GetLocalFileData( const std::filesystem::path& filepath, std::string& data );

    bool GZipCompressData( const std::string& dataToCompress, std::string& compressedData );

    bool GZipUncompressData( const std::string& dataToUncompress, std::string& uncompressedData );

    bool SaveFile( const std::filesystem::path& path, const std::string& data );

    int64_t CalculateBinaryOperation( const std::filesystem::path& path );
}

#endif // ResourceTools_H