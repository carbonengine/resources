#pragma once

#ifndef Patching_H
#define Patching_H

#include <string>
#include <filesystem>


namespace fs = std::filesystem;

namespace ResourceTools
{

class BundleStreamOut;

bool ApplyPatch( const std::string& data, const std::string& patchData, std::string& out );

bool CreatePatch( const std::string& data1, const std::string& data2, std::string& patchData );

bool CreatePatchFile( fs::path before, fs::path after, fs::path patch );

bool ApplyPatchFile( fs::path target, fs::path patch );

bool ApplyPatchFileChunked( fs::path target, BundleStreamOut patch );

class PatchData
{
public:
	PatchData( BundleStreamOut* data, void* bufferStart, size_t outFileLength )
	{
		m_data = data;
		m_bufferStart = bufferStart;
		m_outFileLength = outFileLength;
	}
	BundleStreamOut* m_data;
	void* m_bufferStart;
	size_t m_outFileLength;
};
}
#endif //Patching_H
