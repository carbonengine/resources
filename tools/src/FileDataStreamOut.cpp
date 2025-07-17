// Copyright © 2025 CCP ehf.

#include "FileDataStreamOut.h"

namespace ResourceTools
{

  FileDataStreamOut::FileDataStreamOut( ) :
	  m_fileSize( 0 ),
      m_writeInProgress(false)
  {
  }

  FileDataStreamOut::~FileDataStreamOut()
  {
	  if( m_writeInProgress )
	  {
		  Finish();
	  }
  }

  bool FileDataStreamOut::Finish()
  {
	  m_writeInProgress = false;

	  m_outputStream.close();

      return true;
  }

  bool FileDataStreamOut::IsFinished()
  {
	  return !m_writeInProgress;
  }

  bool FileDataStreamOut::StartWrite( std::filesystem::path filepath )
  {
	  // Creates directories if required
	  std::filesystem::path directory = filepath.parent_path();

	  if( directory != "" )
	  {
		  try
		  {
			  if( !std::filesystem::exists( directory ) )
			  {
				  std::filesystem::create_directories( directory );
			  }

			  if( !std::filesystem::exists( directory ) )
			  {
				  return false;
			  }
		  }
		  catch( std::filesystem::filesystem_error& )
		  {
			  return false;
		  }
	  }

	  m_outputStream.open( filepath, std::ios::out | std::ios::binary );

	  if( !m_outputStream )
	  {
		  return false;
	  }

	  m_fileSize = 0;

      m_writeInProgress = true;

	  return true;
  }

  bool FileDataStreamOut::operator<<( const std::string& data )
  {
	  if( !m_writeInProgress )
	  {
		  return false;
	  }

	  if( !m_outputStream.write( data.data(), data.size() ) )
	  {
		  return false;
	  }

	  m_fileSize += data.size();

	  return true;
  }

  size_t FileDataStreamOut::GetFileSize()
  {
	  return m_fileSize;
  }

}