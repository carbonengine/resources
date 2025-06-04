#include "Patching.h"

#include <bsdiff.h>
#include <bspatch.h>

#include "BundleStreamOut.h"
#include "ResourceTools.h"

const char* BSDIFF_HEADER_STR = "ENDSLEY/BSDIFF43";
constexpr uint8_t BSDIFF_HEADER_TEXT_SIZE = 16;
constexpr uint8_t BSDIFF_SIZE_ENCODING_SIZE = 8;
constexpr uint8_t BSDIFF_HEADER_SIZE = BSDIFF_HEADER_TEXT_SIZE + BSDIFF_SIZE_ENCODING_SIZE;

void* bs_alloc( size_t size )
{
	return new int8_t[size];
}

void bs_free( void* ptr )
{
	delete[] reinterpret_cast<int8_t*>( ptr );
}

int bs_write( struct bsdiff_stream* stream, const void* buffer, size_t size, enum bsdiff_stream_type type )
{
	std::string* strp = reinterpret_cast<std::string*>( stream->opaque );
	strp->append( reinterpret_cast<const char*>( buffer ), size );
	return 0;
}

int bs_read_chunked(const struct bspatch_stream * stream, void * buffer,
				 size_t length, enum bspatch_stream_type type)
{
	ResourceTools::PatchData* spd = reinterpret_cast<ResourceTools::PatchData*>(stream->opaque);
	ResourceTools::BundleStreamOut * cs = spd->m_data;

	size_t remaining = length;
	std::string out;
	std::string data;
	size_t chunkSize = cs->GetChunkSize();
	ResourceTools::GetChunk gc;
	gc.data = &data;
	while( remaining )
	{
		size_t toRead = std::min( remaining, chunkSize );
		if( !cs->ReadBytes( toRead, data ) )
		{
			return -1;
		}
		out += data;
		remaining -= toRead;
	}

	if(out.size() < length)
	{
		return -1;
	}
	memcpy( buffer, out.c_str(), length );
	return 0;
}

int bs_read(const struct bspatch_stream * stream, void * buffer,
				 size_t length, enum bspatch_stream_type type)
{
	std::string* sp = reinterpret_cast<std::string*>( stream->opaque );
	if(sp->size() < length)
	{
		return -1;
	}
	memcpy( buffer, sp->c_str(), length );
	sp->erase( 0, length  );
	return 0;
}

namespace ResourceTools
{
bool ApplyPatch(const std::string& data, const std::string& patchData, std::string& out)
  {
  	if( patchData.size() < BSDIFF_HEADER_SIZE )
  	{
  		return false;
  	}
  	if( memcmp( patchData.data(), BSDIFF_HEADER_STR, BSDIFF_HEADER_TEXT_SIZE ) != 0 )
  	{
  		return false;
  	}
  	bspatch_stream stream;
  	uint64_t targetLength = *reinterpret_cast<uint64_t*>(const_cast<char*>(patchData.c_str() + BSDIFF_HEADER_TEXT_SIZE));
  	out.resize( targetLength );

  	auto readString = std::string(patchData.c_str() + BSDIFF_HEADER_SIZE, patchData.size() - BSDIFF_HEADER_SIZE);
  	stream.opaque = &readString;
  	stream.read = bs_read;

  	int result = bspatch( reinterpret_cast<const uint8_t*>( data.c_str() ), data.size(), reinterpret_cast<uint8_t*>( const_cast<char*>( out.c_str() ) ), targetLength, &stream );
  	if( result != 0 )
  	{
  		return false;
  	}
  	return true;
  }

bool ApplyPatchChunked(const std::string& data, ResourceTools::BundleStreamOut& patchData, std::string& out)
  {
  	bspatch_stream stream;
  	std::string headerBytes;
  	if(!patchData.ReadBytes( BSDIFF_HEADER_TEXT_SIZE, headerBytes ))
  	{
  		return false;
  	}
  	if( memcmp( headerBytes.c_str(), BSDIFF_HEADER_STR, BSDIFF_HEADER_TEXT_SIZE ) != 0 )
  	{
  		return false;
  	}
  	std::string targetLengthBytes;
  	if(!patchData.ReadBytes( BSDIFF_SIZE_ENCODING_SIZE, targetLengthBytes ) )
  	{
  		return false;
  	}

  	uint64_t targetLength = *reinterpret_cast<uint64_t*>(const_cast<char*>(targetLengthBytes.c_str()));
  	out.resize( targetLength );

  	PatchData spd(&patchData, out.data(), targetLength);
  	stream.opaque = &spd;
  	stream.read = bs_read_chunked;

  	int result = bspatch( (uint8_t*)data.c_str(), data.size(), (uint8_t*)out.c_str(), targetLength, &stream );
  	if( result != 0 )
  	{
  		return false;
  	}
  	return true;
  }

  bool CreatePatch(const std::string& previousData, const std::string& latestData, std::string& patchData)
  {
  	bsdiff_stream stream;
  	stream.opaque = &patchData;
  	stream.malloc = bs_alloc;
  	stream.free = bs_free;
  	stream.write = bs_write;
  	int result = bsdiff( reinterpret_cast<const uint8_t*>(previousData.c_str()), previousData.size(), reinterpret_cast<const uint8_t*>( latestData.c_str() ), latestData.size(), &stream );
  	if( result != 0 )
  	{
  		return false;
  	}
  	char sizeEncodedInHeader[8];
  	uint64_t size = latestData.size();
  	memcpy( sizeEncodedInHeader, &size, sizeof size );
  	patchData = std::string(BSDIFF_HEADER_STR, 16) + std::string( sizeEncodedInHeader, 8 ) + patchData;

  	return true;
  }

  bool ApplyPatchFile( std::filesystem::path target, std::filesystem::path patch )
  {
  	  std::string targetData;
  	  if( !GetLocalFileData( target, targetData ) )
  	  {
	  	  return false;
  	  }
	  std::string patchData;
	  if( !GetLocalFileData( patch, patchData ) )
	  {
		  return false;
	  }
  	  std::string outData;
  	  if( !ApplyPatch( targetData, patchData, outData ) )
  	  {
	  	  return false;
  	  }
  	  if( !SaveFile( target, outData ) )
  	  {
	  	  return false;
  	  }
	  return true;
  }

  bool ApplyPatchFileChunked( std::filesystem::path target, BundleStreamOut patch )
  {
	  std::string targetData;
	  if( !GetLocalFileData( target, targetData ) )
	  {
		  return false;
	  }
	  std::string patchData;
	  std::string outData;
	  if( !ApplyPatchChunked( targetData, patch, outData ) )
	  {
		  return false;
	  }
	  if( !SaveFile( target, outData ) )
	  {
		  return false;
	  }
	  return true;
  }


bool CreatePatchFile(std::filesystem::path before, std::filesystem::path after, std::filesystem::path patch)
  {
  	std::string beforeData;
  	std::string afterData;
  	std::string patchData;

  	if( !GetLocalFileData(before, beforeData) )
  	{
  		return false;
  	}
  	if( !GetLocalFileData(after, afterData) )
  	{
  		return false;
  	}

  	if(!CreatePatch( beforeData, afterData, patchData ))
  	{
  		return false;
  	}
	if(!SaveFile( patch, patchData ))
	{
		return false;
	}

  	return true;
  }
}