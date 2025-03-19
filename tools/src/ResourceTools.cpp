
#include "ResourceTools.h"

#include <curl/curl.h>
#include <zlib.h>

#include "Md5ChecksumStream.h"
#include "FileDataStreamIn.h"
#include "FileDataStreamOut.h"

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
	  Md5ChecksumStream md5ChecksumStream;

      md5ChecksumStream << data;

      if (md5ChecksumStream.FinishAndRetrieve(checksum))
      {
		  return true;
      }
      else
      {
		  return false;
      }

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

      FileDataStreamIn fileDataStreamIn;

      if (!fileDataStreamIn.StartRead(filepath))
      {
		  return false;
      }

      if (!(fileDataStreamIn >> data))
      {
		  return false;
      }
      else
      {
		  return true;
      }

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
		  uncompressedData.append( std::string( reinterpret_cast<const char*>( out ), strm.total_out - uncompressedData.size() ) );
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
	  FileDataStreamOut fileDataStreamOut;

      if (!fileDataStreamOut.StartWrite(path))
      {
		  return false;
      }

      if (!(fileDataStreamOut << data))
      {
		  return false;
      }
      else
      {
		  return true;
      }

  }


}