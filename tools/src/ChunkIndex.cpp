// Copyright © 2025 CCP ehf.

#include "ChunkIndex.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include "FileDataStreamIn.h"
#include "ResourceTools.h"
#include "RollingChecksum.h"

// Block contains checksum ( uint32_t ) and offset ( uint32_t )
// Index files will take up 8x the size of the original file.
constexpr size_t CHUNK_BLOCK_SIZE = sizeof( uint32_t ) + sizeof( uint32_t );
constexpr size_t TARGET_FILE_SIZE = 1024 * 1024 * 512; // 512 MB index files, each covering 64 MB of the source file.
constexpr size_t BLOCKS_PER_FILE = TARGET_FILE_SIZE / CHUNK_BLOCK_SIZE;


namespace ResourceTools
{
ChunkIndex::ChunkIndex( std::filesystem::path fileToIndex, uint32_t chunkSize, const std::filesystem::path& indexFolder, StatusCallback statusCallback ) :
	m_fileToIndex( fileToIndex ), m_chunkSize( chunkSize ), m_currentIndexFile( 0 ), m_indexFolder( indexFolder ), m_statusCallback( statusCallback )
{
}

ChunkIndex::~ChunkIndex()
{
	for( auto path : m_indexFiles )
	{
		if( std::filesystem::exists( path ) )
		{
			std::filesystem::remove( path );
		}
	}
}

bool ChunkIndex::Flush( std::vector<std::pair<uint32_t, uint32_t>>& index )
{
	if( index.empty() )
	{
		return true;
	}
	std::filesystem::path out = GenerateIndexPath();
	++m_currentIndexFile;
	std::filesystem::path directory = out.parent_path();

	if( !m_currentIndexFile && directory != "" )
	{
		try
		{
			if( !std::filesystem::exists( directory ) )
			{
				std::filesystem::create_directories( directory );
			}

			if( !std::filesystem::exists( directory ) )
			{
				return false;
			}
		}
		catch( std::filesystem::filesystem_error& )
		{
			return false;
		}
	}

	std::ofstream streamOut;
	streamOut.open( out, std::ios::out | std::ios::binary );
	if( !streamOut )
	{
		if( m_statusCallback )
		{
			std::stringstream ss;
			ss << "Index generation failed. Failed to open path for writing: " << out;
			m_statusCallback( 0, ss.str() );
		}
		return false;
	}
	m_indexFiles.push_back( out );

	std::sort( index.begin(), index.end() );
	streamOut.write( reinterpret_cast<char*>( &index[0] ), sizeof( std::pair<uint32_t, uint32_t> ) * index.size() );
	index.clear();
	return true;
}

std::filesystem::path ChunkIndex::GenerateIndexPath()
{
	std::stringstream ss;
	ss << m_currentIndexFile;
	std::filesystem::path filename = m_fileToIndex.filename();
	return m_indexFolder / ( filename.string() + ss.str() + ".index" );
}

bool ChunkIndex::GenerateChecksumFilter( const std::filesystem::path& targetFile )
{
	FileDataStreamIn targetIn( m_chunkSize );
	targetIn.StartRead( targetFile );
	size_t targetSize = std::filesystem::file_size( targetFile );
	for( uintmax_t dataOffset = 0; dataOffset < targetSize; dataOffset += m_chunkSize )
	{
		std::string nextFileData;
		if( !targetIn.IsFinished() )
		{
			if( !( targetIn >> nextFileData ) )
			{
				return false;
			}
		}
		auto nextFileDataSize = static_cast<uint32_t>( nextFileData.size() );
		uint32_t checksum = ResourceTools::GenerateRollingAdlerChecksum( nextFileData, 0, nextFileDataSize ).checksum;
		m_checksumFilter.insert( checksum );
	}
	return true;
}

bool ChunkIndex::IsRelevant( uint32_t checksum )
{
	if( m_checksumFilter.empty() )
	{
		return true;
	}
	return m_checksumFilter.find( checksum ) != m_checksumFilter.end();
}

bool ChunkIndex::Generate()
{
	size_t result{ 0 };
	FileDataStreamIn streamIn( m_chunkSize );
	if( !streamIn.StartRead( m_fileToIndex ) )
	{
		return result;
	}

	if( !std::filesystem::exists( m_indexFolder ) )
	{
		if( !std::filesystem::create_directories( m_indexFolder ) )
		{
			if( m_statusCallback )
			{
				m_statusCallback( 0, "Failed to create index directory: " + m_indexFolder.string() );
			}
			return false;
		}
	}

	size_t fileSize = std::filesystem::file_size( m_fileToIndex );
	size_t indexFileCount = fileSize / BLOCKS_PER_FILE;
	size_t currentIndexFile{ 0 };

	std::string fileData;
	std::string backlog;

	uint32_t backlogOffset{ 0 };
	uint64_t fileOffset{ 0 };
	RollingChecksum checksum;

	std::vector<std::pair<uint32_t, uint32_t>> chunkToOffsets;
	chunkToOffsets.reserve( BLOCKS_PER_FILE );
	size_t cachedChunks{ 0 };

	size_t onePercentOfFileSize = fileSize / 100;

	while( streamIn >> fileData )
	{
		backlog += fileData;
		while( backlogOffset + m_chunkSize <= backlog.size() )
		{
			if( backlogOffset == 0 )
			{
				checksum = GenerateRollingAdlerChecksum( backlog, 0, m_chunkSize );
			}
			else
			{
				checksum = GenerateRollingAdlerChecksum( backlog, backlogOffset, backlogOffset + m_chunkSize, checksum );
			}
			if( m_statusCallback && !( ( backlogOffset + fileOffset ) % onePercentOfFileSize ) )
			{
				std::stringstream ss;
				ss << "Generating index: " << GenerateIndexPath();
				auto percentage = static_cast<unsigned int>( ( backlogOffset + fileOffset ) * 100 / fileSize );
				m_statusCallback( percentage, ss.str() );
			}
			if( !IsRelevant( checksum.checksum ) )
			{
				++backlogOffset;
				continue;
			}
			auto offset = static_cast<uint32_t>( backlogOffset + fileOffset - m_currentIndexFile * BLOCKS_PER_FILE );
			chunkToOffsets.emplace_back( std::pair<uint32_t, uint32_t>( checksum.checksum, offset ) );
			++cachedChunks;
			++backlogOffset;
			if( cachedChunks >= BLOCKS_PER_FILE )
			{
				if( m_statusCallback )
				{
					std::filesystem::path filePath = GenerateIndexPath();
					std::stringstream ss;
					ss << "Generating index (" << currentIndexFile + 1 << "/" << indexFileCount << "): " << filePath;
					auto percentage = static_cast<unsigned int>( 100 * currentIndexFile / indexFileCount );
					m_statusCallback( percentage, ss.str() );
				}
				Flush( chunkToOffsets );
				++currentIndexFile;
				cachedChunks = 0;
			}
		}
		// Shrink fileData every now and then, but beware, we need to access start-1 for the rolling checksum.
		if( backlogOffset > m_chunkSize + 1 )
		{
			backlogOffset -= m_chunkSize;
			fileOffset += m_chunkSize;
			backlog = backlog.substr( m_chunkSize );
		}
	}
	if( m_statusCallback && indexFileCount > 0 )
	{
		auto percentage = static_cast<unsigned int>( currentIndexFile / indexFileCount );
		m_statusCallback( percentage, "Generating index" );
	}
	Flush( chunkToOffsets );
	if( m_statusCallback )
	{
		m_statusCallback( 100, "Index generated" );
	}
	return true;
}

bool FindMatchingChunksInFile( uint32_t chunk, const std::filesystem::path& path, size_t baseOffset, std::vector<size_t>& offsets )
{
	size_t fileSize = std::filesystem::file_size( path );
	std::ifstream chunkFile;
	chunkFile.open( path, std::ifstream::binary );
	if( !chunkFile )
	{
		return false;
	}
	size_t chunkCount = fileSize / CHUNK_BLOCK_SIZE;

	size_t beginning = 0;
	size_t end = chunkCount;
	uint32_t current;
	while( beginning <= end )
	{
		size_t pos = ( beginning + end ) / 2;
		chunkFile.seekg( CHUNK_BLOCK_SIZE * pos );
		chunkFile.read( reinterpret_cast<char*>( &current ), sizeof( current ) );
		if( current > chunk )
		{
			if( beginning == end )
			{
				return true;
			}
			end = pos;
		}
		else if( current < chunk )
		{
			if( beginning == end )
			{
				return true;
			}
			if( beginning == pos )
			{
				beginning = pos + 1;
			}
			else
			{
				beginning = pos;
			}
		}
		else
		{
			// It's a match!
			uint32_t relative{ 0 };
			chunkFile.read( reinterpret_cast<char*>( &relative ), sizeof( relative ) );
			offsets.push_back( baseOffset + relative );

			if( pos )
			{
				size_t previous = pos - 1;
				while( true )
				{
					chunkFile.seekg( CHUNK_BLOCK_SIZE * previous );
					chunkFile.read( reinterpret_cast<char*>( &current ), sizeof( current ) );
					if( current == chunk )
					{
						uint32_t relative{ 0 };
						chunkFile.read( reinterpret_cast<char*>( &relative ), sizeof( relative ) );
						offsets.push_back( baseOffset + relative );
					}
					else
					{
						break;
					}
					if( !previous )
					{
						break;
					}
					--previous;
				}
			}

			size_t next = pos + 1;
			while( next <= chunkCount )
			{
				chunkFile.seekg( CHUNK_BLOCK_SIZE * next );
				chunkFile.read( reinterpret_cast<char*>( &current ), sizeof( current ) );
				if( current == chunk )
				{
					uint32_t relative;
					chunkFile.read( reinterpret_cast<char*>( &relative ), sizeof( relative ) );
					offsets.push_back( baseOffset + relative );
				}
				else
				{
					break;
				}
				++next;
			}

			return true;
		}
	}
	return true;
}

bool ChunkIndex::FindChunkOffsets( uint32_t chunk, std::vector<size_t>& offsets )
{
	size_t baseOffset{ 0 };
	for( auto path : m_indexFiles )
	{
		if( !FindMatchingChunksInFile( chunk, path, baseOffset, offsets ) )
		{
			return false;
		}
		baseOffset += BLOCKS_PER_FILE;
	}
	return true;
}

bool ChunkIndex::FindMatchingChunk( const std::string& chunk, size_t& chunkOffset )
{
	std::string sourceMD5;
	bool sourceChecksumGenerated{ false };

	size_t baseOffset{ 0 };
	std::vector<size_t> offsets;

	auto end = static_cast<uint32_t>( chunk.size() );
	RollingChecksum rollingChecksum = ResourceTools::GenerateRollingAdlerChecksum( chunk, 0, end );

	for( auto path : m_indexFiles )
	{
		if( !FindMatchingChunksInFile( rollingChecksum.checksum, path, baseOffset, offsets ) )
		{
			return false;
		}
		if( !offsets.empty() )
		{
			std::ifstream chunkFile;
			chunkFile.open( m_fileToIndex, std::ifstream::binary );
			if( !chunkFile )
			{
				return false;
			}

			for( size_t offset : offsets )
			{
				std::string fileData;
				fileData.resize( chunk.size() );
				chunkFile.seekg( static_cast<std::streamoff>( offset ) );
				chunkFile.read( fileData.data(), fileData.size() );
				if( !sourceChecksumGenerated )
				{
					if( !ResourceTools::GenerateMd5Checksum( fileData, sourceMD5 ) )
					{
						return false;
					}
					sourceChecksumGenerated = true;
				}

				std::string matchingChunkMD5;
				if( !ResourceTools::GenerateMd5Checksum( chunk, matchingChunkMD5 ) )
				{
					return false;
				}
				if( sourceMD5 == matchingChunkMD5 )
				{
					// It's legit!
					chunkOffset = offset;
					return true;
				}
			}
		}
	}
	return false;
}

}