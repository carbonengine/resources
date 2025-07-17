// Copyright © 2025 CCP ehf.

#include "ResourceTools.h"

#include "BundleStreamOut.h"

#include <sstream>
#if __APPLE__
#include <sys/stat.h> // for lstat
#endif
#include <filesystem>
#include <fstream>

#include <curl/curl.h>
#include <zlib.h>

#include "Md5ChecksumStream.h"
#include "FileDataStreamIn.h"
#include "FileDataStreamOut.h"
#include "RollingChecksum.h"


namespace ResourceTools
{

  bool GenerateMd5Checksum( const std::filesystem::path& path, std::string& checksum )
  {
	ResourceTools::Md5ChecksumStream md5Stream;
	ResourceTools::FileDataStreamIn fileDataIn;
	if( !fileDataIn.StartRead( path ) )
	{
		return false;
	}
	std::string temp;
	while( fileDataIn >> temp )
	{
		md5Stream << temp;
	}
	md5Stream.FinishAndRetrieve( checksum );
	return true;
  }

  bool Md5ChecksumMatches( const std::filesystem::path& path, std::string& checksum )
  {
  	std::string otherChecksum;
  	if( !GenerateMd5Checksum( path, otherChecksum ) )
  	{
  		return false;
  	}
  	return otherChecksum == checksum;
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
	  unsigned long long offset_bias = 14695981039346656037U;

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
	  while( checksum.size() < 16 )
	  {
		  checksum = "0" + checksum;
	  }

	  return true;
  }

 std::list<ChunkMatch> FindMatchingChunks( const std::string& source, std::string& destination )
  {
	  std::list<ChunkMatch> result;

	  uint32_t window = 4; // The amount of bytes to use for the checksum.

	  if( destination.size() < window || source.size() < window )
	  {
		  // Not much to find here. Let's not bother.
		  return result;
	  }

	  uint32_t start = 0;
	  uint32_t end = window;

	  RollingChecksum sourceStartChecksum = GenerateRollingAdlerChecksum( source, 0, window );
	  while( end < destination.size() )
	  {
		  ChunkMatch match;
		  match.sourceOffset = 0;
		  match.destinationOffset = start;
		  match.length = 0;

		  RollingChecksum destinationChecksum = GenerateRollingAdlerChecksum( destination, start, end );

		  uint32_t length = 0;
		  uint32_t maxLength = 0;
		  match.length = 0;
		  uint32_t sourceStart = 0;
		  uint32_t sourceEnd = sourceStart + window;
		  RollingChecksum rollingDestinationChecksum = destinationChecksum;
		  RollingChecksum rollingSourceChecksum = sourceStartChecksum;
	  	  uint32_t sourceMatchStart = sourceStart;
		  while( sourceEnd <= source.size() && end + length <= destination.size() )
		  {
			  if( length )
			  {
				  rollingDestinationChecksum = GenerateRollingAdlerChecksum( destination, start + length, end + length, rollingDestinationChecksum );
			  }
			  else
			  {
				  rollingDestinationChecksum = destinationChecksum;
			  }
		  	  if(sourceStart)
		  	  {
			  	  rollingSourceChecksum = GenerateRollingAdlerChecksum( source, sourceStart, sourceEnd, rollingSourceChecksum );
		  	  }
			  else
			  {
			  	rollingSourceChecksum = sourceStartChecksum;
			  }
			  if( rollingDestinationChecksum.checksum == rollingSourceChecksum.checksum )
			  {
			  	  if(!length)
			  	  {
			  	  	sourceMatchStart = sourceStart;
			  	  }
				  ++length;
				  if( length > maxLength )
				  {
					  maxLength = length;
					  match.length = length + window - 1;
					  match.sourceOffset = sourceMatchStart;
					  match.destinationOffset = start;
				  }
			  }
		  	  else
		  	  {
			  	  length = 0;
		  	  }
			  ++sourceStart;
			  ++sourceEnd;
		  }
		  if( maxLength )
		  {
			  bool subsumed = false;
			  for( auto it = result.rbegin(); it != result.rend(); it++ )
			  {
				  if( match.destinationOffset - it->destinationOffset > match.length  )
				  {
					  break;
				  }
				  int64_t delta = match.destinationOffset - it->destinationOffset;
				  if( it->length >= delta + match.length )
				  {
					  subsumed = true;
					  break;
				  }
			  }
			  if( !subsumed )
			  {
				  result.push_back( match );
			  }
		  }
		  ++start;
		  ++end;
	  }
	  return result;
  }

  bool FindMatchingChunk( const std::string& chunk, std::filesystem::path filePath, size_t& chunkOffset )
  {
  	  if( chunk.size() > std::numeric_limits<uint32_t>::max() )
  	  {
	  	  return false;
  	  }
	  uint32_t chunkSize = static_cast<uint32_t>( chunk.size() );
	  FileDataStreamIn stream( chunkSize );
	  stream.StartRead( filePath );
	  std::string backlog;
	  std::string fileData;
	  RollingChecksum chunkChecksum = GenerateRollingAdlerChecksum( chunk, 0, chunkSize );
	  RollingChecksum lastChecksum;
	  uint64_t fileOffset{ 0 };
	  uint32_t backlogOffset{ 0 };
	  while( stream >> fileData )
	  {
		  backlog += fileData;
		  while( backlogOffset + chunkSize <= backlog.size() )
		  {
			  if( backlogOffset == 0 )
			  {
				  lastChecksum = GenerateRollingAdlerChecksum( backlog, 0, chunkSize );
			  }
			  else
			  {
				  lastChecksum = GenerateRollingAdlerChecksum( backlog, backlogOffset, backlogOffset + chunkSize, lastChecksum );
			  }
			  if( lastChecksum.checksum == chunkChecksum.checksum )
			  {
				  // We have a potential match. Time to verify
				  std::string sourceMD5;
				  if( !ResourceTools::GenerateMd5Checksum( chunk, sourceMD5 ) )
				  {
					  ++backlogOffset;
					  continue;
				  }
				  std::string matchingChunkMD5;
				  std::string matchStr = backlog.substr( backlogOffset, chunkSize );
				  if( !ResourceTools::GenerateMd5Checksum( matchStr, matchingChunkMD5 ) )
				  {
					  ++backlogOffset;
					  continue;
				  }
				  if( sourceMD5 == matchingChunkMD5 )
				  {
					  // It's legit!
					  chunkOffset = backlogOffset + fileOffset;
					  return true;
				  }
			  }
			  ++backlogOffset;
		  }

		  // Shrink fileData every now and then, but beware, we need to access start-1 for the rolling checksum.
		  if( backlogOffset > chunkSize + 1 )
		  {
			  backlogOffset -= chunkSize;
			  fileOffset += chunkSize;
			  backlog = backlog.substr( chunkSize );
		  }
	  }
	  return false;
  }

  size_t CountMatchingChunks( const std::filesystem::path& fileA, size_t offsetA, std::filesystem::path fileB, size_t offsetB, size_t chunkSize )
  {
  	size_t result{ 0 };
  	FileDataStreamIn streamA( chunkSize );
  	if(!streamA.StartRead( fileA ))
  	{
  		return result;
  	}
  	streamA.Seek( offsetA );

  	FileDataStreamIn streamB( chunkSize );
  	if(!streamB.StartRead( fileB ))
  	{
  		return result;
  	}
  	streamB.Seek( offsetB );

  	std::string chunkA;
  	std::string chunkB;

  	std::string checksumA;
  	std::string checksumB;

  	while( true )
  	{
  		if( !(streamA >> chunkA) || !(streamB >> chunkB) )
  		{
  			return result;
  		}
  		if( !ResourceTools::GenerateMd5Checksum( chunkA, checksumA ) )
  		{
  			return result;
  		}
  		if( !ResourceTools::GenerateMd5Checksum( chunkB, checksumB ) )
  		{
  			return result;
  		}
  		if( checksumA != checksumB )
  		{
  			break;
  		}
  		++result;
  	}

	return result;
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

  bool GZipCompressData( const std::string& dataToCompress, std::string& compressedData )
  {
      // Ensure the input is cleared prior to calculating compression
	  compressedData.clear();

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
		  compressedData.append( std::string( reinterpret_cast<const char*>( out ), strm.total_out - compressedData.size() ) );
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

#if __APPLE__
  unsigned int CalculateBinaryOperation( const std::filesystem::path& path )
  {
  	  struct stat s;
	  int err = lstat( path.c_str(), &s );
  	  if(err)
  	  {
	  	  return 0;
  	  }
  	  return s.st_mode;
  }
#elif WIN32
int FileAttributesToMode(DWORD attr)
  {
  	int m = 0x8000; // Regular file, not a symlink or directory or anything.
  	if (attr & FILE_ATTRIBUTE_READONLY) {
  		m |= 0444;
  	}
  	else
  	{
  		m |= 0666;
  	}
  	return m;
  }

void SetExecutableFlagByFileExtension(const wchar_t *path, BY_HANDLE_FILE_INFORMATION fileInfo, unsigned int &result) {
  	const wchar_t *fileExtension = wcsrchr(path, '.');
  	if (fileExtension) {
  		if (_wcsicmp(fileExtension, L".exe") == 0 ||
			  _wcsicmp(fileExtension, L".bat") == 0 ||
			  _wcsicmp(fileExtension, L".cmd") == 0 ||
			  _wcsicmp(fileExtension, L".com") == 0) {
  			result |= 0111;
			  }
  	}
  }

unsigned int CalculateBinaryOperation( const std::filesystem::path& path )
{
  	HANDLE hFile;
  	BY_HANDLE_FILE_INFORMATION fileInfo;
  	DWORD access = FILE_READ_ATTRIBUTES;
  	DWORD flags{0};
  	unsigned int result{0};
  	hFile = CreateFileW(path.wstring().c_str(), access, 0, nullptr, OPEN_EXISTING, flags, nullptr);

  	if (hFile == INVALID_HANDLE_VALUE)
  	{
  		return 0;
  	}

  	FILE_BASIC_INFO basicInfo;
  	if (!GetFileInformationByHandle(hFile, &fileInfo) || !GetFileInformationByHandleEx(hFile, FileBasicInfo, &basicInfo, sizeof(basicInfo)))
  	{
  		return 0;
  	}

  	auto fileType = GetFileType(hFile);
  	if (fileType != FILE_TYPE_DISK) {
  		return 0;
  	}

  	result = FileAttributesToMode(fileInfo.dwFileAttributes);
  	SetExecutableFlagByFileExtension(path.wstring().c_str(), fileInfo, result);
  	CloseHandle(hFile);
  	return result;
  }
#endif

}