
#include "BundleStreamOut.h"


namespace ResourceTools
{
  

  BundleStreamOut::BundleStreamOut( unsigned long chunkSize ) :
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
		  std::string& dataRef = *data.data;

          dataRef = m_cache.substr( 0, m_chunkSize );

		  m_cache.erase( 0, m_chunkSize );
      }

  }

  unsigned long BundleStreamOut::GetChunkSize() const
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
  }

}