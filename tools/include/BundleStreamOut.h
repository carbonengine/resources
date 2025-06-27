/* 
	*************************************************************************

	BundleStreamOut.h

	Author:    James Hawk
	Created:   March. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/

#pragma once
#ifndef BundleStreamOut_H
#define BundleStreamOut_H

#include <FileDataStreamIn.h>
#include <FileDataStreamOut.h>
#include <ScopedFile.h>

#include <string>
#include "GzipCompressionStream.h"

namespace ResourceTools
{

    struct GetChunk
	{
    	std::shared_ptr<ResourceTools::FileDataStreamIn> uncompressedChunkIn;

    	std::shared_ptr<ResourceTools::FileDataStreamIn> compressedChunkIn;

        bool clearCache{false};

    	bool outOfChunks{false};
	};

    class BundleStreamOut
    {
	public:
		BundleStreamOut( uintmax_t chunkSize, std::filesystem::path outputDirectory );

		~BundleStreamOut();

        bool operator<<( std::shared_ptr<ResourceTools::FileDataStreamIn> streamIn );

        // Outputs chunks
		bool operator>>( GetChunk& data );

		bool Flush();

    private:
		bool AddChunkFilesToGetChunk( GetChunk& data );

		bool InitializeOutputStreams();

		std::vector<std::shared_ptr<ResourceTools::ScopedFile>> m_chunkFiles;

		uintmax_t m_chunkSize;

        std::string m_cache;

        std::string m_uncompressedData;

		std::string m_compressedData;

		std::unique_ptr<GzipCompressionStream> m_compressionStream;

		std::unique_ptr<ResourceTools::FileDataStreamOut> m_compressedOut;

		std::unique_ptr<ResourceTools::FileDataStreamOut> m_uncompressedOut;

		std::filesystem::path m_outputDirectory;

		uint32_t m_chunksCreated{0};

		uint32_t m_chunksExported{0};
    };
 
}

#endif // BundleStreamOut_H