
#include "CompressedFileDataStreamOut.h"

namespace ResourceTools
{

  CompressedFileDataStreamOut::CompressedFileDataStreamOut( )
  {
  }

  CompressedFileDataStreamOut::~CompressedFileDataStreamOut()
  {
  }

  bool CompressedFileDataStreamOut::StartWrite( std::filesystem::path filepath )
  {
      if (m_compressionStream)
      {
		  return false;
      }

	  m_compressionStream = std::make_unique<GzipCompressionStream>( &m_compressionBuffer );
      
      if (!m_compressionStream->Start())
      {
		  return false;
      }

      return FileDataStreamOut::StartWrite( filepath );
  }

  bool CompressedFileDataStreamOut::Finish()
  {
      if (!m_compressionStream)
      {
		  return false;
      }

      if (!m_compressionStream->Finish())
      {
		  return false;
      }

      if( m_compressionBuffer.size() > 0 )
	  {
          if (!FileDataStreamOut::operator<<(m_compressionBuffer))
          {
			  return false;
          }
	  }


      m_compressionStream.reset();

      m_compressionBuffer.clear();

      return FileDataStreamOut::Finish();
  }

  bool CompressedFileDataStreamOut::operator << ( const std::string& data )
  {
	  if( !m_compressionStream )
	  {
		  return false;
	  }

      if (!m_compressionStream->operator<<(&data))
      {
		  return false;
      }

      if (m_compressionBuffer.size() > 0)
      {
          if (!FileDataStreamOut::operator<<(m_compressionBuffer))
          {
			  return false;
          }

          m_compressionBuffer.clear();
      }

      return true;
  }


}