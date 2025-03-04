
#include "ResourceTools.h"

#include <sstream>
#include <fstream>

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/hex.h>
#include <cryptopp/md5.h>
#include <cryptopp/files.h>

namespace ResourceTools
{

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

  bool GetLocalFileData( const std::string& filepath, std::string& data )
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

	  return true;
  }

  bool DownloadFile( const std::string& url, const std::string& outputPath )
  {
	  return false;
  }

  bool GZipCompressData(const unsigned char* dataToCompress, unsigned long dataToCompressSize, unsigned char* compressedData, unsigned long& compressedDataSize)
  {
	  return false;
  }

  bool GZipUncompressData(const unsigned char* dataToUncompress, unsigned long dataToUncompressSize, unsigned char* uncompressedData, unsigned long& uncompressedDataSize)
  {
	  return false;
  }

  bool CreatePatch(const std::string& previousData, const std::string& latestData, std::string& outputPath)
  {
	  return false;
  }

  bool SaveFile(const std::string path, const std::string& data)
  {
	  std::ofstream out( path );

      if (!out)
      {
		  return false;
      }

      out << data;

      out.close();

	  return true;
  }

}