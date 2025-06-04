
#include "BundleStreamOut.h"
#include "ResourceTools.h"

namespace ResourceTools
{
  

  BundleStreamOut::BundleStreamOut( uintmax_t chunkSize ) :
	  m_chunkSize(chunkSize),
	  m_compressionStream( nullptr )
  {

  }

  BundleStreamOut::~BundleStreamOut()
  {

  }

  bool BundleStreamOut::operator<<( const std::string& data )
  {
      m_cache.append( data );

	  return true;
  }

  bool BundleStreamOut::operator>>( GetChunk& data )
  {
	  size_t cacheSize = m_cache.size();

      if (cacheSize == 0)
      {
          // No data in cache
		  return false;
      }

      if (data.clearCache)
      {
          // Clear the cache to destination
		  std::string& dataRef = *data.data;

          dataRef = m_uncompressedData;
	    
          dataRef.append( m_cache );

          m_cache.clear();

          return true;
      }

      if (m_cache.size() < m_chunkSize)
      {
          // Not enough data to create chunk
		  return false;
      }
      else
      {

          if (!m_compressionStream)
          {
			  m_compressionStream = new GzipCompressionStream( &m_compressedData );

              m_compressedData.clear();

              m_uncompressedData.clear();

              if (!m_compressionStream->Start())
              {
                  // TODO: Return value is confused, sometimes it means failure sometimes it means not enough data
                  // this needs refactoring along with changes to make procedure more stream friendly
                  // Full chunk indication should be passed back as part of GetChunk struct
                  // For streaming the uncompressed data should be outputted as chunk is being made
                  // and streamed to file a bit at a time to keep memory usage down with large files
				  return false;
              }
          }

          bool chunkSizeAchieved = false;

          while (m_compressedData.size() < m_chunkSize)
          {
              // Get chunk from cache
			  std::string chunk = m_cache.substr( 0, m_chunkSize );

              if (!m_compressionStream->operator<<(&chunk))
              {
				  return false;
              }

              m_uncompressedData.append( chunk );

              m_cache.erase( 0, m_chunkSize );

              if( m_compressedData.size() >= m_chunkSize )
              {
                  // Chunk size achieved after compression
				  chunkSizeAchieved = true;

				  break;
              }

              if( m_cache.size() < m_chunkSize )
              {
                  // Chunk not achieved, not enough data in cache to create chunk of requested size
				  break;
              }
          }
          if (chunkSizeAchieved)
          {
              // Remove the data from the cache and return the resulting uncompressed data

              std::string& dataRef = *data.data;

              if (!m_compressionStream->Finish())
              {
				  return false;
              }

			  dataRef = m_uncompressedData;

              delete m_compressionStream;

              m_compressedData.clear();

              m_uncompressedData.clear();

              return true;
          }
		  
      }

      return false;

  }

  uintmax_t BundleStreamOut::GetChunkSize() const
  {
	  return m_chunkSize;
  }

  bool BundleStreamOut::ReadBytes( size_t n, std::string& out )
  {
	  if( m_cache.size() < n )
	  {
		  return false;
	  }
  	  out = m_cache.substr( 0, n );
  	  m_cache.erase( 0, n );
	  return true;
  }

}