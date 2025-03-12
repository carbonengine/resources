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

namespace ResourceTools
{

    // TODO expect all these signatures to move around, just made off the top of my head so far
    // Not settled on correct approach to manage data etc

	// Initialize CURL.
	// Should be called once at program startup, but if you are initializing
	// from a Windows DLL you should not initialize it from DllMain or a static initializer
	// because Windows holds the loader lock during that time and it could cause a deadlock.
	// See: https://curl.se/libcurl/c/curl_global_init.html
	bool Initialize();

	bool ShutDown();  // Shut down CURL.

    bool GenerateMd5Checksum( const std::string& data, std::string& checksum );

    bool GenerateFowlerNollVoChecksum( const std::string& input, std::string& checksum );

    bool GetLocalFileData( const std::filesystem::path& filepath, std::string& data );

    bool DownloadFile( const std::string& url, const std::filesystem::path& outputPath );

    bool GZipCompressData( const std::string& dataToCompress, std::string& compressedData );

    bool GZipUncompressData( const std::string& dataToUncompress, std::string& uncompressedData );

    bool ApplyPatch( const std::string& data, const std::string& patchData, std::string& out );

    bool CreatePatch(const std::string& data1, const std::string& data2, std::string& patchData);

    bool SaveFile( const std::filesystem::path& path, const std::string& data);


    // TODO Not sure where this should live

    struct GetChunk
	{
		std::string* data = nullptr;

        bool clearCache = false;
	};

    struct GetFile
    {
		unsigned long fileSize = 0;

        std::string* data = nullptr;
    };

    class ChunkStream
    {
	public:
		ChunkStream( unsigned long chunkSize);

		~ChunkStream();

        bool operator<<( const std::string& data ); 

        // Outputs chunks
		bool operator>>( GetChunk& data );

        // Outputs files
        bool operator>>( GetFile& file );

    private:

		unsigned long m_chunkSize;

        std::string m_cache;

    };




    
}

#endif // ResourceTools_H