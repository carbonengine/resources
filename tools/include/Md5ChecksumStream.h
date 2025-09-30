// Copyright © 2025 CCP ehf.

#pragma once
#ifndef Md5ChecksumStream_H
#define Md5ChecksumStream_H

#include <string>
#include <sstream>

namespace CryptoPP
{
class HexEncoder;

namespace Weak1
{
class MD5;
}
}

namespace ResourceTools
{

class Md5ChecksumStream
{
public:
	Md5ChecksumStream();

	~Md5ChecksumStream();

	bool FinishAndRetrieve( std::string& checksum );

	bool operator<<( const std::string& data );

private:
	void Finish();

private:
	std::stringstream m_ss;

	CryptoPP::HexEncoder* m_encoder;

	CryptoPP::Weak1::MD5* m_hash;
};


}

#endif // Md5ChecksumStream_H