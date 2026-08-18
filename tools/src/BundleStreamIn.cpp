// Copyright © 2025 CCP ehf.

#include "BundleStreamIn.h"


namespace ResourceTools
{

BundleStreamIn::BundleStreamIn( uintmax_t chunkSize ) :
	m_chunkSize( chunkSize ),
	m_dataReadOfCurrentFile( 0 ),
	m_cacheOffset(0),
	m_cacheSize(0)
{
}

BundleStreamIn ::~BundleStreamIn()
{
}

uintmax_t BundleStreamIn::GetCacheSize()
{
	return m_cacheSize;
}

bool BundleStreamIn::operator<<( const std::string& dataData )
{
	m_cache.append( dataData );

    auto test = dataData.size();

    m_cacheSize += dataData.size();

	return true;
}

void BundleStreamIn::clearCache()
{
    // If value is over the total cache size in ram then remove used data
    // This adjusts the buffer size less often to speed up processing
	m_cache.erase( 0, m_cacheOffset );
	m_cacheOffset = 0; 
	
}

bool BundleStreamIn::operator>>( GetFile& fileData )
{
	if( m_cacheSize == 0 )
	{
		// No data in cache
		return false;
	}

	std::string& dataRef = *fileData.data;

	if( ( m_dataReadOfCurrentFile + m_chunkSize ) >= fileData.fileSize )
	{
		uintmax_t remainingDataSize = fileData.fileSize - m_dataReadOfCurrentFile;

		dataRef = m_cache.substr( m_cacheOffset, remainingDataSize );

        m_cacheOffset += remainingDataSize;
		m_cacheSize -= remainingDataSize;

		m_dataReadOfCurrentFile = 0;
	}
	else
	{
		dataRef = m_cache.substr( m_cacheOffset, m_chunkSize );

        m_cacheOffset += m_chunkSize;
		m_cacheSize -= m_chunkSize;

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
	if( m_cacheSize < n )
	{
		return false;
	}

	out = m_cache.substr( m_cacheOffset, n );

    m_cacheOffset += n;

    clearCache();

	return true;
}

}