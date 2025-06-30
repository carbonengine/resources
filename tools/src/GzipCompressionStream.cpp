
#include "GzipCompressionStream.h"



namespace ResourceTools
{
  
  GzipCompressionStream::GzipCompressionStream( std::string* out ):
	m_compressionInProgress( false ),
	m_out( out )
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

  bool GzipCompressionStream::ProcessBuffer( bool finish )
  {
  	constexpr size_t CHUNK = 16384; // 16kb
  	int ret = Z_OK;
  	unsigned char outbuffer[CHUNK];
  	int flush{Z_NO_FLUSH};
  	while( ret == Z_OK && ( finish || !m_buffer.empty() ) )
  	{
  		size_t available = m_buffer.size();
  		m_stream.next_in = reinterpret_cast<Bytef*>( const_cast<char*>( m_buffer.c_str() ) );
  		m_stream.avail_in = static_cast<uInt>( available );
  		m_stream.next_out = outbuffer;
  		m_stream.avail_out = CHUNK;
  		if( finish && m_buffer.size() <= CHUNK )
  		{
  			flush = Z_FINISH;
  		}
  		uLong alreadyIn = m_stream.total_in;
  		uLong alreadyOut = m_stream.total_out;
  		ret = deflate( &m_stream, flush );
  		uLong outBytes = m_stream.total_out - alreadyOut;
  		uLong inBytes = m_stream.total_in - alreadyIn;
  		m_out->append( std::string( reinterpret_cast<const char*>( outbuffer ), outBytes ) );
  		m_buffer = m_buffer.substr( inBytes );
  	}

  	if( ret != Z_OK && ret != Z_STREAM_END )
  	{
  		deflateEnd( &m_stream );
  		return false;
  	}
  	return true;
  }

  bool GzipCompressionStream::operator<<( const std::string* toCompress )
  {
      if (!m_compressionInProgress)
      {
		  return false;
      }

  	  m_buffer.append( *toCompress );

      return ProcessBuffer( false );
  }

  bool GzipCompressionStream::Finish()
  {
	  if( !m_compressionInProgress )
	  {
		  return false;
	  }

  	  ProcessBuffer( true );

      m_compressionInProgress = false;

	  return deflateEnd( &m_stream ) == Z_OK;
  }
}