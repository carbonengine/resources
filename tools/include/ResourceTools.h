/* 
	*************************************************************************

	ResourceTools.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/

#pragma once
#ifndef ResourceTools_H
#define ResourceTools_H

#include <memory>
#include <string>
#include <filesystem>

namespace ResourceTools
{

    // TODO expect all these signatures to move around, just made off the top of my head so far
    // Not settled on correct approach to manage data etc

    bool GenerateMd5Checksum( const std::string& data, std::string& checksum );

    bool GenerateFowlerNollVoChecksum( const std::string& input, std::string& checksum );

    bool GetLocalFileData( const std::filesystem::path& filepath, std::string& data );

    bool DownloadFile( const std::string& url, const std::string& outputPath );

    bool GZipCompressData( const std::string& dataToCompress, std::string& compressedData );

    bool GZipUncompressData( const std::string& dataToUncompress, std::string& uncompressedData );

    bool CreatePatch(const std::string& data1, const std::string& data2, std::string& patchData);

    bool SaveFile( const std::filesystem::path& path, const std::string& data);

}

#endif // ResourceTools_H