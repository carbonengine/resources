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
		unsigned long fileSize = 0;

        std::string* data = nullptr;
    };

    class BundleStreamIn
	{
	public:
		BundleStreamIn( unsigned long chunkSize );

		~BundleStreamIn();

        unsigned long GetCacheSize();

		bool operator<<( const std::string& chunkData );

		bool operator>>( GetFile& fileData );


	private:

        unsigned long m_chunkSize;

		std::string m_cache;

        unsigned long m_dataReadOfCurrentFile;
	};





    
}

#endif // BundleStreamIn_H