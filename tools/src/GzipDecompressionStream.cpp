// Copyright © 2025 CCP ehf.

#include "GzipDecompressionStream.h"

#include <algorithm>

constexpr int CHUNK = 16384; // 16kb

namespace ResourceTools
{
  
  GzipDecompressionStream::GzipDecompressionStream( std::string* out ):
	m_decompressionInProgress( false ),
	m_out( out )
  {

  }

  GzipDecompressionStream ::~GzipDecompressionStream()
  {

  }

  bool GzipDecompressionStream::Start( )
  {
  	m_stream.zalloc = Z_NULL;
  	m_stream.zfree = Z_NULL;
  	m_stream.opaque = Z_NULL;
  	m_stream.avail_in = 0;
  	m_stream.next_in = Z_NULL;
  	int ret = inflateInit2( &m_stream, 16 | MAX_WBITS );
  	if( ret != Z_OK )
  	{
  		return false;
  	}
	m_decompressionInProgress = true;
  	return true;
  }

  bool GzipDecompressionStream::ProcessBuffer( bool finish )
  {
  	constexpr size_t CHUNK = 16384; // 16kb
  	int ret = Z_OK;
  	unsigned char outbuffer[CHUNK];
  	int flush{Z_NO_FLUSH};
	size_t uIntMax = std::numeric_limits<uInt>::max();
	size_t index{0};

  	while( ret == Z_OK && ( finish || index < m_buffer.size() ) )
  	{
  		m_stream.next_in = reinterpret_cast<Bytef*>( const_cast<char*>( m_buffer.c_str() + index ) );
  		m_stream.avail_in = static_cast<uInt>( std::min( m_buffer.size() - index, uIntMax ) );
  		m_stream.next_out = outbuffer;
  		m_stream.avail_out = CHUNK;
		auto alreadyOut = m_stream.total_out;
  		auto alreadyIn = m_stream.total_in;
  		ret = inflate( &m_stream, Z_NO_FLUSH );
  		m_out->append( std::string( reinterpret_cast<const char*>( outbuffer ), m_stream.total_out - alreadyOut ) );
  		index += m_stream.total_in - alreadyIn;
  	}
	m_buffer.clear();

  	if( ret != Z_OK && ret != Z_STREAM_END )
  	{
  		inflateEnd( &m_stream );
  		return false;
  	}
  	return true;
  }

  bool GzipDecompressionStream::operator<<( std::string* toDecompress )
  {
      if (!m_decompressionInProgress)
      {
		  return false;
      }

  	  m_buffer.append( *toDecompress );

      return ProcessBuffer( false );
  }

  bool GzipDecompressionStream::Finish()
  {
	  if( !m_decompressionInProgress )
	  {
		  return false;
	  }

  	  ProcessBuffer( true );

      m_decompressionInProgress = false;

	  return inflateEnd( &m_stream ) == Z_OK;;
  }
}
