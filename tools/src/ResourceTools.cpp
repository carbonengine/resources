
#include "ResourceTools.h"

#include <sstream>
#include <fstream>

#include <curl/curl.h>
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/hex.h>
#include <cryptopp/md5.h>
#include <cryptopp/files.h>
#include <zlib.h>

static CURL* s_curlHandle{nullptr};

namespace ResourceTools
{
  bool Initialize()
  {
  	if(!s_curlHandle)
  	{
  		// According to the docs, `curl_global_init` must be called at least once within a program
  		// before the program calls any other function in libcurl, and multiple calls
  		// should have the same effect as one call. (https://curl.se/libcurl/c/curl_global_init.html)
  		curl_global_init( CURL_GLOBAL_DEFAULT );
  		s_curlHandle = curl_easy_init();
  	}
  	return s_curlHandle != nullptr;
  }

  bool ShutDown()
  {
	if( s_curlHandle )
	{
		curl_easy_cleanup( s_curlHandle );
		curl_global_cleanup();
		s_curlHandle = nullptr;
	}
  	return true;
  }

  bool GenerateMd5Checksum( const std::string& data, std::string& checksum )
  {
	  // Generate an md5 checksum
	  std::stringstream ss;

	  CryptoPP::HexEncoder encoder( new CryptoPP::FileSink( ss ), false );

	  std::string digest;

	  CryptoPP::Weak1::MD5 hash;

	  hash.Update( (const CryptoPP::byte*)data.data(), data.size() );

	  digest.resize( hash.DigestSize() );

	  hash.Final( (CryptoPP::byte*)&digest[0] );

	  CryptoPP::StringSource( digest, true, new CryptoPP::Redirector( encoder ) );

	  checksum = ss.str();

	  while( checksum.size() < 32 )
	  {
		  checksum = "0" + checksum;
	  }

	  return true;
  }

  bool GenerateFowlerNollVoChecksum(const std::string& input, std::string& checksum)
  {
	  unsigned long long offset_bias = 14695981039346656037;

	  unsigned long long prime = 1099511628211;

	  unsigned long long hash = offset_bias;

      const char* data = input.data();

	  for( int i = 0; i < input.size(); i++ )
	  {
		  hash = ( hash * prime ) & 0xffffffffffffffff;

		  hash = ( hash ^ data[i] ) & 0xffffffffffffffff;
	  }

	  std::ostringstream ss;

	  ss << std::hex << hash;

	  checksum = ss.str();

	  //Pad with zeros to get to correct size
	  while( checksum.size() < 16 )// TODO constant
	  {
		  checksum = "0" + checksum;
	  }

	  return true;
  }

  bool GetLocalFileData( const std::filesystem::path& filepath, std::string& data )
  {
	  std::ifstream inputStream;

	  inputStream.open( filepath, std::ios::in | std::ios::binary );

	  if( !inputStream )
	  {
		  return false;
	  }

      inputStream.seekg( 0, std::ios::end );

      size_t fileSize = inputStream.tellg();

      inputStream.seekg( 0, std::ios::beg );

      data.clear();

      data.resize( fileSize );

      inputStream.read( data.data(), fileSize );

      inputStream.close();

	  return true;
  }

size_t WriteToFileStreamCallback( void* contents, size_t size, size_t nmemb, void* context )
  {
  	const char* charString = static_cast<const char*>(contents);
  	const size_t realSize = size * nmemb;
  	std::ofstream* out = static_cast<std::ofstream*>(context);
  	std::string str( charString, realSize );
  	*out << str;
  	return realSize;
  }

  bool DownloadFile( const std::string& url, const std::filesystem::path& outputPath )
  {
	  if( std::filesystem::exists( outputPath ) )
	  {
		  // Let's not overwrite an existing file.
		  return false;
	  }
	  std::filesystem::path parent = outputPath.parent_path();
	  if( !std::filesystem::exists( parent ) && !std::filesystem::create_directories( parent ) )
	  {
		  // Failed to create directory to place the downloaded file in.
		  return false;
	  }
	  if( !s_curlHandle )
	  {
		  // Initialize() must be called before downloading.
		  return false;
	  }
	  std::ofstream out( outputPath, std::ios_base::app | std::ios::binary );
	  curl_easy_setopt( s_curlHandle, CURLOPT_URL, url.c_str() );
	  curl_easy_setopt( s_curlHandle, CURLOPT_FAILONERROR, 1 );
	  curl_easy_setopt( s_curlHandle, CURLOPT_WRITEDATA, &out );
	  curl_easy_setopt( s_curlHandle, CURLOPT_WRITEFUNCTION, WriteToFileStreamCallback );
  	  curl_easy_setopt( s_curlHandle, CURLOPT_ACCEPT_ENCODING, "gzip" );
	  CURLcode cc = curl_easy_perform( s_curlHandle );
	  return cc == CURLE_OK;
  }

  bool GZipCompressData( const std::string& dataToCompress, std::string& compressedData )
  {
	  z_stream strm;
	  constexpr int CHUNK = 16384; // 16kb
	  unsigned char out[CHUNK];

	  strm.zalloc = Z_NULL;
	  strm.zfree = Z_NULL;
	  strm.opaque = Z_NULL;
	  int windowBits = MAX_WBITS | 16; // 16 is a magic bit flag to specify GZip compression format.
	  int ret = deflateInit2( &strm, Z_BEST_COMPRESSION, Z_DEFLATED, windowBits, 8, Z_DEFAULT_STRATEGY );
	  if( ret != Z_OK )
	  {
		  return false;
	  }

	  strm.next_in = reinterpret_cast<Bytef*>( const_cast<char*>( dataToCompress.c_str() ) );
	  strm.avail_in = static_cast<uInt>( dataToCompress.size() );

	  do
	  {
		  strm.next_out = out;
		  strm.avail_out = CHUNK;
		  int flush = strm.avail_in <= CHUNK ? Z_FINISH : Z_NO_FLUSH;
		  ret = deflate( &strm, flush );
		  compressedData.append( std::string( reinterpret_cast<const char*>( out ), strm.total_out ) );
	  } while( ret == Z_OK );

	  if( ret != Z_STREAM_END )
	  {
		  deflateEnd( &strm );
		  return false;
	  }
	  return deflateEnd( &strm ) == Z_OK;
  }

  bool GZipUncompressData( const std::string& dataToUncompress, std::string& uncompressedData )
  {
	  z_stream strm;
	  constexpr int CHUNK = 16384; // 16kb
	  unsigned char out[CHUNK];

	  strm.zalloc = Z_NULL;
	  strm.zfree = Z_NULL;
	  strm.opaque = Z_NULL;
	  strm.avail_in = 0;
	  strm.next_in = Z_NULL;
	  int ret = inflateInit2( &strm, 16 | MAX_WBITS );
	  if( ret != Z_OK )
	  {
		  return false;
	  }

	  strm.next_in = reinterpret_cast<Bytef*>( const_cast<char*>( dataToUncompress.c_str() ) );
	  strm.avail_in = static_cast<uInt>( dataToUncompress.size() );

	  do
	  {
		  strm.next_out = out;
		  strm.avail_out = CHUNK;
		  ret = inflate( &strm, Z_NO_FLUSH );
		  uncompressedData.append( std::string( reinterpret_cast<const char*>( out ), strm.total_out ) );
	  } while( ret == Z_OK );

	  if( ret != Z_STREAM_END )
	  {
		  inflateEnd( &strm );
		  return false;
	  }

	  return inflateEnd( &strm ) == Z_OK;
  }

  bool ApplyPatch(const std::string& data, const std::string& patchData, std::string& out)
  {
	// TODO implement patch application
	  out = patchData;

      return true;
  }

  bool CreatePatch(const std::string& previousData, const std::string& latestData, std::string& patchData)
  {
      // TODO implement patching
	  patchData = latestData;

	  return true;
  }

  bool SaveFile( const std::filesystem::path& path, const std::string& data )
  {
	  // Creates directories if required
      std::filesystem::path directory = path.parent_path();

      if (!std::filesystem::exists(directory))
      {
		  std::filesystem::create_directories( directory );
      }

      if( !std::filesystem::exists( directory ) )
	  {
		  return false;
	  }

	  std::ofstream out( path, std::ios::binary );

      if (!out)
      {
		  return false;
      }

      out << data;

      out.close();

	  return true;
  }







  ChunkStream::ChunkStream( unsigned long chunkSize ) :
	  m_chunkSize(chunkSize)
  {

  }

  ChunkStream ::~ChunkStream()
  {

  }

  bool ChunkStream::operator<<( const std::string& data )
  {
      m_cache.append( data );

	  return true;
  }

  bool ChunkStream::operator>>( GetChunk& data )
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

  bool ChunkStream::operator >> ( GetFile& file )
  {
	  size_t cacheSize = m_cache.size();

      if (cacheSize == 0)
      {
          // No data in cache
		  return false;
      }

      if (cacheSize < file.fileSize)
      {
          // Not enough data to create file
		  return false;
      }
      else
      {
          // Copy resource amount of cache
		  std::string& dataRef = *file.data;

		  dataRef = m_cache.substr( 0, file.fileSize );

		  m_cache.erase( 0, file.fileSize );
      }


  }



}