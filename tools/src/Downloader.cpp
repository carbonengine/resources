// Copyright © 2025 CCP ehf.

#include "Downloader.h"

#include <fstream>
#include <set>
#include <thread>
#include <sstream>
#include <iostream>

int s_activeDownloaders{ 0 };
std::set<int> s_curl_retry_errors{
	CURLE_AGAIN,
	CURLE_COULDNT_RESOLVE_PROXY,
	CURLE_COULDNT_RESOLVE_HOST,
	CURLE_COULDNT_CONNECT,
	CURLE_RECV_ERROR,
	CURLE_SEND_ERROR,
	CURLE_OPERATION_TIMEDOUT
};

// Retry curl operation for up to 2 minutes.
std::chrono::seconds RETRY_PERIOD_SECONDS{ 120 };

bool InitializeCurl()
{
	// According to the docs, `curl_global_init` must be called at least once within a program
	// before the program calls any other function in libcurl, and multiple calls
	// should have the same effect as one call. (https://curl.se/libcurl/c/curl_global_init.html)
	return curl_global_init( CURL_GLOBAL_DEFAULT ) == CURLE_OK;
}

void ShutDownCurl()
{
	curl_global_cleanup();
}

struct WriteToFileStreamCallWrapper
{
	std::string url;

    std::filesystem::path relativePath;

	std::ofstream out;

    ResourceTools::DownloadFileCallback callback = nullptr;

	size_t totalSizeBytes = 0;

	size_t downloadedSizeBytes = 0;

	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;

	void* context = nullptr;
};

size_t WriteToFileStreamCallback( void* contents, size_t size, size_t nmemb, void* context )
{
	const char* charString = static_cast<const char*>( contents );

	const size_t realSize = size * nmemb;

	WriteToFileStreamCallWrapper* wrapper = static_cast<WriteToFileStreamCallWrapper*>( context );

	std::string str( charString, realSize );

	wrapper->out << str;

	wrapper->downloadedSizeBytes += nmemb;

    if (wrapper->callback)
    {
        // Calculate bytes per second
		auto end = std::chrono::high_resolution_clock::now();

		long long durationSecondsSinceSeconds = std::chrono::duration_cast<std::chrono::seconds>( end - wrapper->startTime ).count();
		
        double bytesPerSecond = (double)wrapper->downloadedSizeBytes;

        if (durationSecondsSinceSeconds > 0)
        {
			bytesPerSecond = (double)wrapper->downloadedSizeBytes / durationSecondsSinceSeconds;
        }

		wrapper->callback( wrapper->url, wrapper->relativePath, wrapper->totalSizeBytes, wrapper->downloadedSizeBytes, bytesPerSecond, wrapper->context );
    }

	return realSize;
}

size_t WriteToFileStringCallback( void* contents, size_t size, size_t nmemb, void* context )
{
	const char* charString = static_cast<const char*>( contents );
	const size_t realSize = size * nmemb;
	std::stringstream* out = static_cast<std::stringstream*>( context );
	std::string str( charString, realSize );
	*out << str;
	return realSize;
}

namespace ResourceTools
{
Downloader::Downloader()
{
	if( !s_activeDownloaders++ )
	{
		InitializeCurl();
	}
	m_curlHandle = curl_easy_init();
}

Downloader::~Downloader()
{
	curl_easy_cleanup( m_curlHandle );
	m_curlHandle = nullptr;
	if( !--s_activeDownloaders )
	{
		ShutDownCurl();
	}
}

Response Downloader::GetHeader( const std::string& url, std::string& response )
{
	std::stringstream out;
	curl_easy_setopt( m_curlHandle, CURLOPT_URL, url.c_str() );
	curl_easy_setopt( m_curlHandle, CURLOPT_NOBODY, 1L );
	curl_easy_setopt( m_curlHandle, CURLOPT_HEADER, 1L );
	curl_easy_setopt( m_curlHandle, CURLOPT_WRITEDATA, &out );
	curl_easy_setopt( m_curlHandle, CURLOPT_WRITEFUNCTION, WriteToFileStringCallback );
	CURLcode cc = curl_easy_perform( m_curlHandle );

	if( cc == CURLE_OK )
	{
		// File exists
		response = out.str();

		return Response::SUCCESS;
	}
	else if( cc == CURLE_REMOTE_FILE_NOT_FOUND )
	{
		// File doesn't exist
		response = out.str();

		return Response::FILE_NOT_FOUND;
	}
    else
    {
		curl_easy_reset( m_curlHandle );
		return Response::DOWNLOAD_ERROR;
    }
	
}

bool Downloader::DownloadFile( const std::string& url, const std::filesystem::path& relativePath, const std::filesystem::path& outputPath, const std::chrono::seconds& retrySeconds, uintmax_t retryCount, size_t expectedTotalSize /* = 0*/, DownloadFileCallback callback /* = nullptr */, void* callbackContext /* = nullptr */ )
{
	if( std::filesystem::exists( outputPath ) )
	{
		// Let's not overwrite an existing file.
		return false;
	}
	std::filesystem::path parent = outputPath.parent_path();
	if( !std::filesystem::exists( parent ) && !std::filesystem::create_directories( parent ) )
	{
		// Failed to create directory to place the downloaded file in.
		return false;
	}

	WriteToFileStreamCallWrapper callWrapper;
	callWrapper.out = std::ofstream( outputPath, std::ios_base::app | std::ios::binary );
	callWrapper.callback = callback;
	callWrapper.totalSizeBytes = expectedTotalSize;
	callWrapper.context = callbackContext;
	callWrapper.url = url;
	callWrapper.relativePath = relativePath;

	curl_easy_setopt( m_curlHandle, CURLOPT_URL, url.c_str() );
	curl_easy_setopt( m_curlHandle, CURLOPT_FAILONERROR, 1 );
	curl_easy_setopt( m_curlHandle, CURLOPT_WRITEDATA, &callWrapper );
	curl_easy_setopt( m_curlHandle, CURLOPT_WRITEFUNCTION, WriteToFileStreamCallback );
	curl_easy_setopt( m_curlHandle, CURLOPT_ACCEPT_ENCODING, "gzip" );
	CURLcode cc{ CURLE_OK };

    for (auto i = 0; i < retryCount; i++)
    {
		callWrapper.startTime = std::chrono::high_resolution_clock::now();
		cc = curl_easy_perform( m_curlHandle );

        if( cc == CURLE_OK )
        {
			return true;
        }
        else if (s_curl_retry_errors.find(cc) == s_curl_retry_errors.end())
        {
            // Error is something that a retry wont help with
			return false;
        }
        else
        {
            // Sleep with a simple backoff
			std::this_thread::sleep_for( (i+1) * retrySeconds );
        }

    }

    return false;

}

bool Downloader::GetAttributeValueFromHeader( const std::string& header, const std::string& attributeName, std::string& value )
{
	const std::string DELIMITER = ":";

	std::string attributeNameMatch = attributeName + DELIMITER;

	std::stringstream ss( header );

	std::string line;
	while( std::getline( ss, line ) )
	{
		auto findResult = line.find( attributeNameMatch );

		if( findResult != std::string::npos )
		{
			// Attribute found
			auto findResult = line.find( DELIMITER );

			if( findResult != std::string::npos )
			{
				// Delimiter found
				value = line.substr( findResult + 2 );
				return true;
			}
		}
	}

	return false;
}

}
