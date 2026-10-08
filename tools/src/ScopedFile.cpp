// Copyright © 2025 CCP ehf.

#include "ScopedFile.h"


namespace ResourceTools
{

ScopedFile::ScopedFile( std::filesystem::path location ) :
	m_location( location )
{
}

ScopedFile::~ScopedFile()
{
	if( std::filesystem::exists( m_location ) )
	{
		// Attempt to delete the temporary file
		std::error_code ec;
		std::filesystem::remove_all( m_location, ec );
	}
}


}