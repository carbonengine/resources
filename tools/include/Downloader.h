// Copyright © 2025 CCP ehf.

#pragma once

#include <filesystem>
#include <string>

#include <curl/curl.h>

namespace ResourceTools
{
	// Utility class for downloading files.
	// Reuse is encouraged for multiple downloads, but do not share across threads.
	class Downloader
	{
	public:
		Downloader();
		~Downloader();
		bool DownloadFile( const std::string& url, const std::filesystem::path& outputPath, const std::chrono::seconds& retrySeconds );
	private:
		CURL* m_curlHandle{ nullptr };
	};
}