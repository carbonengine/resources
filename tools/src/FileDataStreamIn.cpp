
#include "FileDataStreamIn.h"

namespace ResourceTools
{
  
  FileDataStreamIn::FileDataStreamIn( unsigned long chunkSize /*= -1*/ ) :
	  m_chunkSize( chunkSize ),
	  m_fileSize(0),
	  m_currentPosition(0)
  {
	  
  }

  FileDataStreamIn::~FileDataStreamIn()
  {
      if (m_readInProgress)
      {
		  Finish();
      }
  }

  void FileDataStreamIn::Finish()
  {
	  m_readInProgress = false;

	  m_inputStream.close();
  }

  bool FileDataStreamIn::IsFinished()
  {
	  return !m_readInProgress;
  }

  size_t FileDataStreamIn::GetCurrentPosition()
  {
	  return m_currentPosition;
  }

  bool FileDataStreamIn::StartRead( std::filesystem::path filepath )
  {
      m_inputStream.open( filepath, std::ios::in | std::ios::binary );

      if( !m_inputStream )
	  {
		  return false;
	  }

      m_inputStream.seekg( 0, std::ios::end );

      m_fileSize = m_inputStream.tellg();

      m_inputStream.seekg( 0, std::ios::beg );

      m_readInProgress = true;

	  return true;
      
  }

  bool FileDataStreamIn::operator>>( std::string& data )
  {
      if (!m_readInProgress)
      {
		  return false;
      }

      // If -1 is passed as chunk size then read entire file
      if (m_chunkSize == -1)
      {
		  m_chunkSize = m_fileSize;
      }

	  unsigned long readSize = m_chunkSize;

      if( ( m_currentPosition + readSize ) > m_fileSize )
      {
		  readSize = m_fileSize - m_currentPosition;
      }

	  data.resize( readSize );

      if( !m_inputStream.read( data.data(), readSize ) )
      {
		  return false;
      }

      m_currentPosition += readSize;

      if (m_currentPosition == m_fileSize)
      {
		  Finish();
      }

      return true;
  }

}