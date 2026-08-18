// Copyright © 2025 CCP ehf.

#include "BundleStreamOut.h"
#include "FileDataStreamIn.h"
#include "ResourceTools.h"
#include "ScopedFile.h"

namespace ResourceTools
{

// outputCompressed will only create compressed output from this job
// if split on compressed is on
// If it isn't then this work is deferred until the processing step
// This allows the work to be done asynchronously
BundleStreamOut::BundleStreamOut( uintmax_t chunkSize, std::filesystem::path outputDirectory, bool outputCompressed, bool splitOnCompressedSize, bool calculateCompressions ) :
	m_chunkSize( chunkSize ),
	m_outputDirectory( outputDirectory ),
	m_outputCompressed( outputCompressed ),
	m_splitOnCompressedSize( splitOnCompressedSize ),
	m_calculateCompressions( calculateCompressions ),
	m_checksumStream( std::make_unique<ResourceTools::Md5ChecksumStream>() )
{
   
}

BundleStreamOut::~BundleStreamOut()
{
}

std::filesystem::path RawFilename( std::filesystem::path outputDirectory, size_t chunkNumber )
{
	std::string filename = "chunk" + std::to_string( chunkNumber ) + ".tmp";

	return outputDirectory / filename;
}


bool BundleStreamOut::InitializeOutputStream()
{
	std::filesystem::path rawPath = RawFilename( m_outputDirectory, m_chunkInfos.size() );

	m_chunkOut = std::make_unique<ResourceTools::FileDataStreamOut>();

	if( !m_chunkOut->StartWrite( rawPath ) )
	{
		return false;
	}

    m_currentChunk.path = rawPath;
	m_currentChunk.checksum = "";
	m_currentChunk.compressedSize = 0;
	m_currentChunk.uncompressedSize = 0;

    // Setup compression stream
	if( m_splitOnCompressedSize )
    {
		m_compressionStream = std::make_unique<GzipCompressionStream>( &m_compressedData );

		m_compressedData.clear();

		if( !m_compressionStream->Start() )
		{
			return false;
		}
    }
    
    return true;
}

bool BundleStreamOut::Finish()
{
	return Flush();
}

bool BundleStreamOut::Flush()
{
	
	// Chunk size achieved after compression
	if( m_splitOnCompressedSize )
    {
		if( !m_compressionStream )
		{
			return true;
		}

		if( !m_compressionStream->Finish() )
		{
			return false;
		}

		if( m_outputCompressed )
		{
			*m_chunkOut << m_compressedData;
		}

        if (m_calculateCompressions)
        {
			m_currentChunk.compressedSize = m_compressedData.size();
        }

        m_compressedData.clear();

		m_compressionStream.reset();
    }
	

	m_chunkOut->Finish();

    m_chunkOut.reset();

    // Retrieve checksum and store
    std::string chunkChecksum;

    if (!m_checksumStream->Retrieve(chunkChecksum))
    {
		return false;
    }

    m_currentChunk.checksum = chunkChecksum;

    m_checksumStream = std::make_unique<ResourceTools::Md5ChecksumStream>();

    m_chunkInfos.push_back( m_currentChunk );

	return true;
}

size_t BundleStreamOut::GetNumberOfChunksCreated()
{
	return m_chunkInfos.size();
}

bool BundleStreamOut::operator<<( std::shared_ptr<FileDataStreamIn> streamIn )
{
	std::string data;

	while( *streamIn >> data )
	{
		if( !m_chunkOut )
		{
			if( !InitializeOutputStream() )
			{
				return false;
			}
		}

        m_currentChunk.uncompressedSize += data.size();

        if( m_splitOnCompressedSize )
        {
			if(!m_compressionStream->operator<<( &data ))
			{
				return false;
			}
        }

        // Output uncompressed data
		bool outputUncompressed = !m_outputCompressed;

        if (m_outputCompressed && !m_splitOnCompressedSize)
        {
            // Destination should be compressed but as this doesn't
            // split on compressed then the compression will be defferred
			outputUncompressed = true;
        }

        if( outputUncompressed )
        {
			if( !( *m_chunkOut << data ) )
			{
				return false;
			}
        }
        
        // Checksum calculation
		if(!(m_checksumStream->operator<<(data)))
        {
			return false;
        }

        // Can either be split based on the current compressed or uncompressed total size
        // If using uncompressed then it is expected that the output chunks will be less efficient
        // But the split will be more accurate and it can be faster if also not processing compression
        size_t splitSize = m_splitOnCompressedSize ? m_compressedData.size() : m_currentChunk.uncompressedSize;

        // See if the current output is greater than the chunk size indicator
		if( splitSize >= m_chunkSize )
		{
			if( !Flush() )
			{
				return false;
			}

            // Finish
			return true;
		}
		
	}

	return true;
}

bool BundleStreamOut::GetChunkInfo( int index, ChunkInfo& chunkInfo )
{
	if( index >= m_chunkInfos.size() )
    {
		return false;
    }
    else
    {
		chunkInfo = m_chunkInfos.at( index );
		return true;
    }
}

}