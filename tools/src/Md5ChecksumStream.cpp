
#include "Md5ChecksumStream.h"

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/hex.h>
#include <cryptopp/md5.h>
#include <cryptopp/files.h>

namespace ResourceTools
{
 
  Md5ChecksumStream::Md5ChecksumStream()
  {
      m_encoder = new CryptoPP::HexEncoder( new CryptoPP::FileSink( m_ss ), false );

      m_hash = new CryptoPP::Weak1::MD5();
  }

  Md5ChecksumStream::~Md5ChecksumStream()
  {
	  Finish();
  }

  void Md5ChecksumStream::Finish()
  {
      if (m_encoder)
      {
		  delete m_encoder;

          m_encoder = nullptr;
      }
	  
      if (m_hash)
      {
		  delete m_hash;

          m_hash = nullptr;
      }

      return;
  }


  bool Md5ChecksumStream::operator<<( const std::string& data )
  {
      if (!m_hash)
      {
		  return false;
      }

	  m_hash->Update( (const CryptoPP::byte*)data.data(), data.size() );

	  return true;
  }

  bool Md5ChecksumStream::FinishAndRetrieve( std::string& checksum )
  {
      if (!m_hash || !m_encoder)
      {
		  return false;
      }

	  std::string digest;

	  digest.resize( m_hash->DigestSize() );

	  m_hash->Final( (CryptoPP::byte*)&digest[0] );

	  CryptoPP::StringSource stringSource( digest, true, new CryptoPP::Redirector( *m_encoder ) );

	  checksum = m_ss.str();

	  while( checksum.size() < 32 )
	  {
		  checksum = "0" + checksum;
	  }

      Finish();
  	  return true;
  }

}