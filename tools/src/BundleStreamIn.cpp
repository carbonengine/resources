// Copyright © 2025 CCP ehf.

#include "BundleStreamIn.h"


namespace ResourceTools
{

BundleStreamIn::BundleStreamIn( uintmax_t chunkSize ) :
	m_chunkSize( chunkSize ),
	m_dataReadOfCurrentFile( 0 )
{
}

BundleStreamIn ::~BundleStreamIn()
{
}

uintmax_t BundleStreamIn::GetCacheSize()
{
	return m_cache.size();
}

bool BundleStreamIn::operator<<( const std::string& dataData )
{
	m_cache.append( dataData );

	return true;
}

bool BundleStreamIn::operator>>( GetFile& fileData )
{
	size_t cacheSize = m_cache.size();

	if( cacheSize == 0 )
	{
		// No data in cache
		return false;
	}


	std::string& dataRef = *fileData.data;

	if( ( m_dataReadOfCurrentFile + m_chunkSize ) >= fileData.fileSize )
	{
		uintmax_t remainingDataSize = fileData.fileSize - m_dataReadOfCurrentFile;

		dataRef = m_cache.substr( 0, remainingDataSize );

		m_cache.erase( 0, remainingDataSize );

		m_dataReadOfCurrentFile = 0;
	}
	else
	{
		dataRef = m_cache.substr( 0, m_chunkSize );

		m_cache.erase( 0, m_chunkSize );

		m_dataReadOfCurrentFile += m_chunkSize;
	}

	return true;
}

uintmax_t BundleStreamIn::GetChunkSize() const
{
	return m_chunkSize;
}

bool BundleStreamIn::ReadBytes( size_t n, std::string& out )
{
	if( m_cache.size() < n )
	{
		return false;
	}

	out = m_cache.substr( 0, n );

	m_cache.erase( 0, n );

	return true;
}

}