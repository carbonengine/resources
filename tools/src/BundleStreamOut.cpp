
#include "BundleStreamOut.h"
#include "ResourceTools.h"


namespace ResourceTools
{
  

  BundleStreamOut::BundleStreamOut( uintmax_t chunkSize ) :
	  m_chunkSize(chunkSize)
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

          dataRef.resize( m_cache.size() );

          dataRef = m_cache;

          m_cache = "";

          return true;
      }

      if (m_cache.size() < m_chunkSize)
      {
          // Not enough data to create chunk
		  return false;
      }
      else
      {
          // Copy chunk amount out of cache
          int numberOfChunksConsumed = 0;

          std::string uncompressedData;
          std::string compressedData;

          bool chunkSizeAchieved = false;

          while (compressedData.size() < m_chunkSize)
          {
			  uncompressedData.append(m_cache.substr( numberOfChunksConsumed * m_chunkSize, m_chunkSize ));

			  // Compress data
              if (!ResourceTools::GZipCompressData(uncompressedData, compressedData))
              {
				  return false;
              }

              numberOfChunksConsumed++;

              if( compressedData.size() >= m_chunkSize )
              {
                  // Chunk size achieved after compression
				  chunkSizeAchieved = true;

				  break;
              }

              if ((numberOfChunksConsumed + 1) * m_chunkSize > m_cache.size())
              {
                  // Chunk not achieved, not enough data in cache to create chunk of requested size
				  break;
              }
          }
          

          if (chunkSizeAchieved)
          {
              // Remove the data from the cache and return the resulting uncompressed data

              std::string& dataRef = *data.data;

			  dataRef = m_cache.substr( 0, m_chunkSize * numberOfChunksConsumed );

			  m_cache.erase( 0, m_chunkSize * numberOfChunksConsumed );

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