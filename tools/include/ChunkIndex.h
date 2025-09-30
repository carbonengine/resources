// Copyright © 2025 CCP ehf.

#pragma once

#include <filesystem>
#include <map>
#include <unordered_set>
#include <vector>

#include "StatusCallback.h"

namespace ResourceTools
{
class ChunkIndex
{
public:
	ChunkIndex( std::filesystem::path fileToIndex, uint32_t chunkSize, const std::filesystem::path& indexFolder, StatusCallback statusCallback = nullptr );
	~ChunkIndex();
	bool Generate();
	bool FindChunkOffsets( uint32_t chunk, std::vector<size_t>& offsets );
	bool FindMatchingChunk( const std::string& chunk, size_t& chunkOffset );
	bool GenerateChecksumFilter( const std::filesystem::path& targetFile );

private:
	std::filesystem::path GenerateIndexPath();
	bool Flush( std::vector<std::pair<uint32_t, uint32_t>>& index );
	bool IsRelevant( uint32_t checksum );

	std::filesystem::path m_fileToIndex;
	uint32_t m_chunkSize;
	std::vector<std::filesystem::path> m_indexFiles;
	size_t m_currentIndexFile;
	StatusCallback m_statusCallback;
	std::filesystem::path m_indexFolder;
	std::unordered_set<uint32_t> m_checksumFilter;
};

}
