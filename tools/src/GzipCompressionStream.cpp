
#include "GzipCompressionStream.h"



namespace ResourceTools
{
  
  GzipCompressionStream::GzipCompressionStream( ):
	m_compressionInProgress( false )
  {

  }

  GzipCompressionStream ::~GzipCompressionStream()
  {

  }

  bool GzipCompressionStream::Start( )
  {
	  
	  m_stream.zalloc = Z_NULL;

	  m_stream.zfree = Z_NULL;

	  m_stream.opaque = Z_NULL;

	  int windowBits = MAX_WBITS | 16; // 16 is a magic bit flag to specify GZip compression format.

	  int ret = deflateInit2( &m_stream, Z_BEST_COMPRESSION, Z_DEFLATED, windowBits, 8, Z_DEFAULT_STRATEGY );

	  if( ret != Z_OK )
	  {
		  return false;
	  }

      m_compressionInProgress = true;

      return true;
  }

  bool GzipCompressionStream::operator<<( CompressionChunk& compressionChunk )
  {
      if (!m_compressionInProgress)
      {
		  return false;
      }

	  int ret = 0;
	  constexpr int CHUNK = 16384; // 16kb
	  unsigned char out[CHUNK];

      
	  m_stream.next_in = reinterpret_cast<Bytef*>( const_cast<char*>( compressionChunk.uncompressedData->c_str() ) );
	  m_stream.avail_in = static_cast<uInt>( compressionChunk.uncompressedData->size() );

      do
	  {
		  m_stream.next_out = out;
		  m_stream.avail_out = CHUNK;
		  int flush = m_stream.avail_in <= CHUNK ? Z_FINISH : Z_NO_FLUSH;
		  ret = deflate( &m_stream, flush );
		  compressionChunk.compressedData->append( std::string( reinterpret_cast<const char*>( out ), m_stream.total_out ) );
	  } while( ret == Z_OK );

      if( ret != Z_STREAM_END )
	  {
		  deflateEnd( &m_stream );
		  return false;
	  }

	  return true;
  }

  bool GzipCompressionStream::Finish()
  {
	  if( !m_compressionInProgress )
	  {
		  return false;
	  }

      m_compressionInProgress = false;

	  return deflateEnd( &m_stream ) == Z_OK;
  }


}