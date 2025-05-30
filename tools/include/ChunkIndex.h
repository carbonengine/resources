#pragma once

#include <filesystem>
#include <map>
#include <vector>

#include "StatusCallback.h"

namespace ResourceTools
{
class ChunkIndex
{
public:
	ChunkIndex( std::filesystem::path fileToIndex, size_t chunkSize, StatusCallback statusCallback=nullptr );
	~ChunkIndex();
	bool Generate();
	bool FindChunkOffsets( uint32_t chunk, std::vector<size_t>& offsets );
	bool FindMatchingChunk( const std::string& chunk, size_t& chunkOffset );
private:
	std::filesystem::path GenerateIndexPath();
	bool Flush( std::vector<std::pair<uint32_t, uint32_t>>& index );

	std::filesystem::path m_fileToIndex;
	size_t m_chunkSize;
	std::vector<std::filesystem::path> m_indexFiles;
	size_t m_currentIndexFile;
	StatusCallback m_statusCallback;
};

}
