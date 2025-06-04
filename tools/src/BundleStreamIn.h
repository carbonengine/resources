/* 
	*************************************************************************

	BundleStreamIn.h

	Author:    James Hawk
	Created:   March. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/

#pragma once
#ifndef BundleStreamIn_H
#define BundleStreamIn_H

#include <string>

namespace ResourceTools
{

    struct GetFile
    {
		uintmax_t fileSize = 0;

        std::string* data = nullptr;
    };

    class BundleStreamIn
	{
	public:
		BundleStreamIn( uintmax_t chunkSize );

		~BundleStreamIn();

        uintmax_t GetCacheSize();

		bool operator<<( const std::string& chunkData );

		bool operator>>( GetFile& fileData );


	private:

        uintmax_t m_chunkSize;

		std::string m_cache;

        uintmax_t m_dataReadOfCurrentFile;
	};





    
}

#endif // BundleStreamIn_H