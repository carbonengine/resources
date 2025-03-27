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

	// Initialize CURL.
	// Should be called once at program startup, but if you are initializing
	// from a Windows DLL you should not initialize it from DllMain or a static initializer
	// because Windows holds the loader lock during that time and it could cause a deadlock.
	// See: https://curl.se/libcurl/c/curl_global_init.html
	bool Initialize();

	bool ShutDown();  // Shut down CURL.

    bool GenerateMd5Checksum( const std::string& data, std::string& checksum );

    bool GenerateFowlerNollVoChecksum( const std::string& input, std::string& checksum );

	// Generate a weak checksum using the rsync algorithm https://rsync.samba.org/tech_report/node3.html
	RollingChecksum GenerateRollingAdlerChecksum( const std::string& input, uint64_t start, uint64_t end );

	// Generate a weak checksum using the rsync algorithm https://rsync.samba.org/tech_report/node3.html
	RollingChecksum GenerateRollingAdlerChecksum( const std::string& input, uint64_t start, uint64_t end, RollingChecksum previous );

	std::list<ChunkMatch> FindMatchingChunks( const std::string& source, std::string& destination );

    bool FindMatchingChunk( const std::string& chunk, std::filesystem::path filePath, uint64_t& chunkOffset );

    bool GetLocalFileData( const std::filesystem::path& filepath, std::string& data );

    bool DownloadFile( const std::string& url, const std::filesystem::path& outputPath );

    bool GZipCompressData( const std::string& dataToCompress, std::string& compressedData );

    bool GZipUncompressData( const std::string& dataToUncompress, std::string& uncompressedData );

    bool SaveFile( const std::filesystem::path& path, const std::string& data );
}

#endif // ResourceTools_H