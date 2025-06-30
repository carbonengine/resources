#include "BundleStreamOut.h"
#include "FileDataStreamIn.h"
#include "ResourceTools.h"
#include "ScopedFile.h"

namespace ResourceTools
{
BundleStreamOut::BundleStreamOut( uintmax_t chunkSize, std::filesystem::path outputDirectory ) :
	m_chunkSize( chunkSize ),
	m_outputDirectory( outputDirectory )
{
}

BundleStreamOut::~BundleStreamOut()
{

}

std::filesystem::path RawFilename( std::filesystem::path outputDirectory, uint32_t chunkNumber )
{
	std::string filename = "chunk" + std::to_string( chunkNumber ) + ".raw";

	return outputDirectory / filename;
}

std::filesystem::path CompressedFilename( std::filesystem::path outputDirectory, uint32_t chunkNumber )
{
	std::string filename = "chunk" + std::to_string( chunkNumber ) + ".compressed";

	return outputDirectory / filename;
}

bool BundleStreamOut::InitializeOutputStreams()
{
	std::filesystem::path compressedPath = CompressedFilename( m_outputDirectory, m_chunksCreated );

	std::filesystem::path rawPath = RawFilename( m_outputDirectory, m_chunksCreated );

	m_uncompressedOut = std::make_unique<ResourceTools::FileDataStreamOut>();

	if( !m_uncompressedOut->StartWrite( rawPath ) )
	{
		return false;
	}

	m_compressedOut = std::make_unique<ResourceTools::FileDataStreamOut>();

	if( !m_compressedOut->StartWrite( compressedPath ) )
	{
		return false;
	}

	m_chunkFiles.push_back( std::make_shared<ScopedFile>( rawPath ) );

	m_chunkFiles.push_back( std::make_shared<ScopedFile>( compressedPath ) );

	return true;
}

bool BundleStreamOut::Flush()
{
	// Chunk size achieved after compression
	if( !m_compressionStream )
	{
		return true;
	}

	if( !m_compressionStream->Finish() )
	{
		return false;
	}

	*m_compressedOut << m_compressedData;

	m_compressedData.clear();

	++m_chunksCreated;

	m_compressionStream.release();

	return true;
}

bool BundleStreamOut::operator<<( std::shared_ptr<FileDataStreamIn> streamIn )
{
	std::string data;

	while( *streamIn >> data )
	{
		if( !m_compressionStream )
		{
			m_compressionStream = std::make_unique<GzipCompressionStream>( &m_compressedData );

			m_compressedData.clear();

			if( !m_compressionStream->Start() )
			{
				return false;
			}

			if( !InitializeOutputStreams() )
			{
				return false;
			}
		}

		while( !data.empty() )
		{
			std::string chunk = data.substr( 0, m_chunkSize );

			if( !m_compressionStream->operator<<( &chunk ) )
			{
				return false;
			}

			*m_uncompressedOut << chunk;

			*m_compressedOut << m_compressedData;

			m_compressedData.clear();

			data.erase( 0, m_chunkSize );

			if( m_compressedData.size() >= m_chunkSize )
			{
				if( !Flush() )
				{
					return false;
				}

				if( !InitializeOutputStreams() )
				{
					return false;
				}
			}
		}
	}

	return true;
}

bool BundleStreamOut::AddChunkFilesToGetChunk( GetChunk& data )
{
	// Needs to be called after all changes have been made to the file,
	// since FileDataStreamIn assumes filesize does not change once created.
	auto rawDir = RawFilename( m_outputDirectory, m_chunksExported );

	auto compressedDir = CompressedFilename( m_outputDirectory, m_chunksExported );

	if( !exists( rawDir ) || !exists( compressedDir ) )
	{
		return false;
	}

	data.uncompressedChunkIn = std::make_shared<FileDataStreamIn>();

	if( !data.uncompressedChunkIn->StartRead( rawDir ) )
	{
		return false;
	}

	data.compressedChunkIn = std::make_shared<FileDataStreamIn>();

	if( !data.compressedChunkIn->StartRead( compressedDir ) )
	{
		return false;
	}

	return true;
}

bool BundleStreamOut::operator>>( GetChunk& data )
{
	if( data.clearCache )
	{
		// Clear the cache to destination
		Flush();

		data.outOfChunks = true;

		m_uncompressedOut->Finish();

		m_compressedOut->Finish();

		AddChunkFilesToGetChunk( data );

		return true;
	}

	if( m_chunksCreated == m_chunksExported )
	{
		// Not enough data to create chunk
		data.outOfChunks = true;
	}
	else
	{
		AddChunkFilesToGetChunk( data );
	}

	return true;
}
}