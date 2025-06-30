#include "ResourceInfo.h"

#include "Md5ChecksumStream.h"

#include <sstream>

#include <ResourceTools.h>

#include <FileDataStreamIn.h>

#include <FileDataStreamOut.h>

#include <yaml-cpp/yaml.h>

#include "../ResourceGroupImpl.h"

#include "CompressedFileDataStreamOut.h"

namespace CarbonResources
{

    std::string Location::CalculateLocationFromChecksums( const std::string& relativePathChecksum, const std::string& dataChecksum ) const
    {
		std::stringstream ss; 
		ss << relativePathChecksum.substr( 0, 2 );
		ss << "/";
		ss << relativePathChecksum;
		ss << "_";
		ss << dataChecksum;

        return ss.str();
    }

    Result Location::SetFromRelativePathAndDataChecksum(const std::filesystem::path& relativePath, const std::string& dataChecksum)
    {
		std::string relativePathChecksum = "";

		if( !ResourceTools::GenerateFowlerNollVoChecksum( relativePath.generic_string(), relativePathChecksum ) )
		{
			return Result{ ResultType::FAILED_TO_GENERATE_RELATIVE_PATH_CHECKSUM };
		}

		location = CalculateLocationFromChecksums( relativePathChecksum, dataChecksum );

		return Result{ ResultType::SUCCESS };
    }


    ResourceInfo::ResourceInfo( const ResourceInfoParams& params )
    {
		m_relativePath = params.relativePath;

		m_location = params.location;

        m_type = TypeId();
		
		m_checksum = params.checksum;

		m_compressedSize = params.compressedSize;

		m_uncompressedSize = params.uncompressedSize;

    	if( params.binaryOperation )
    	{
    		m_binaryOperation = params.binaryOperation;
    	}

    	if( !params.prefix.empty() )
    	{
    		m_prefix = params.prefix;
    	}
    	else
    	{
    		m_prefix.Reset();
    	}
    }

    ResourceInfo::~ResourceInfo()
    {

    }

	Result ResourceInfo::GetBinaryOperation( unsigned int& binaryOperation ) const
	{
		if( !m_binaryOperation.HasValue() )
		{
			return Result{ ResultType::RESOURCE_VALUE_NOT_SET };
		}
		else
		{
			binaryOperation = m_binaryOperation.GetValue();

			return Result{ ResultType::SUCCESS };
		}
	}

    Result ResourceInfo::GetRelativePath(std::filesystem::path& relativePath) const
    {
        if (!m_relativePath.HasValue())
        {
			return Result{ ResultType::RESOURCE_VALUE_NOT_SET };
        }
        else
        {
			relativePath = m_relativePath.GetValue();

            return Result{ ResultType::SUCCESS };
        }
    }

    void ResourceInfo::SetRelativePath( const std::filesystem::path& relativePath )
    {
		m_relativePath = relativePath;
    }

    Result ResourceInfo::GetLocation(std::string& location) const
    {
		if( !m_location.HasValue() )
		{
			return Result{ ResultType::RESOURCE_VALUE_NOT_SET };
		}
		else
		{
			location = m_location.GetValue().ToString();

			return Result{ ResultType::SUCCESS };
		}
    }

    Result ResourceInfo::GetType(std::string& type) const
    {
		if( !m_type.HasValue() )
		{
			return Result{ ResultType::RESOURCE_VALUE_NOT_SET };
		}
		else
		{
			type = m_type.GetValue();

			return Result{ ResultType::SUCCESS };
		}
    }

    Result ResourceInfo::GetChecksum(std::string& checksum) const
    {
		if( !m_checksum.HasValue() )
		{
			return Result{ ResultType::RESOURCE_VALUE_NOT_SET };
		}
		else
		{
			checksum = m_checksum.GetValue();

			return Result{ ResultType::SUCCESS };
		}
    }

    Result ResourceInfo::GetUncompressedSize( uintmax_t& uncompressedSize ) const
    {
		if( !m_uncompressedSize.HasValue() )
		{
			return Result{ ResultType::RESOURCE_VALUE_NOT_SET };
		}
		else
		{
			uncompressedSize = m_uncompressedSize.GetValue();

			return Result{ ResultType::SUCCESS };
		}
    }

    Result ResourceInfo::GetCompressedSize( uintmax_t& compressedSize ) const
    {
		if( !m_compressedSize.HasValue() )
		{
			return Result{ ResultType::RESOURCE_VALUE_NOT_SET };
		}
		else
		{
			compressedSize = m_compressedSize.GetValue();

			return Result{ ResultType::SUCCESS };
		}
    }

    Result ResourceInfo::PutDataStream( ResourcePutDataStreamParams& params ) const
    {
		if( !params.dataStream )
		{
			return Result{ ResultType::FAILED_TO_SAVE_FILE };
		}

        switch( params.resourceDestinationSettings.destinationType )
		{
		case ResourceDestinationType::LOCAL_RELATIVE:

			return PutDataStreamLocalRelative( params );

			break;

		case ResourceDestinationType::LOCAL_CDN:

			return PutDataStreamLocalCdn( params );

			break;

        case ResourceDestinationType::REMOTE_CDN:

            return PutDataStreamRemoteCdn( params );

		default:
			return Result{ ResultType::FAILED_TO_SAVE_FILE };
		}
    }
   
    Result ResourceInfo::PutData( ResourcePutDataParams& params ) const
    {
        if (!params.data)
        {
			return Result{ ResultType::FAILED_TO_SAVE_FILE };
        }

        switch (params.resourceDestinationSettings.destinationType)
        {
		case ResourceDestinationType::LOCAL_RELATIVE:

			return PutDataLocalRelative( params );

			break;

        case ResourceDestinationType::LOCAL_CDN:

			return PutDataLocalCdn( params );
		
			break;

        case ResourceDestinationType::REMOTE_CDN:

            return PutDataRemoteCdn( params );

			break;

		default:
			return Result{ ResultType::FAILED_TO_SAVE_FILE };

        }

    }

    Result ResourceInfo::PutDataLocalRelative( ResourcePutDataParams& params ) const
    {
		std::string& data = *params.data;

		std::filesystem::path path = params.resourceDestinationSettings.basePath / m_relativePath.GetValue();

        bool res = ResourceTools::SaveFile( path, data );

        if (res)
        {
			return Result{ ResultType::SUCCESS };
        }
        else
        {
			return Result{ ResultType::FAILED_TO_SAVE_FILE };
        }

    }

    Result ResourceInfo::PutDataRemoteCdn( ResourcePutDataParams& params ) const
    {
		std::string& data = *params.data;

		// Construct path
		std::filesystem::path dataPath = params.resourceDestinationSettings.basePath / m_location.GetValue().ToString();

        std::string compressedData;

        if (!ResourceTools::GZipCompressData(data, compressedData))
        {
			return Result{ ResultType::FAILED_TO_COMPRESS_DATA };
        }

        bool res = ResourceTools::SaveFile( dataPath, compressedData );

		if( res )
		{
			return Result{ ResultType::SUCCESS };
		}
		else
		{
			return Result{ ResultType::FAILED_TO_SAVE_FILE };
		}
    }

    Result ResourceInfo::PutDataLocalCdn( ResourcePutDataParams& params ) const
    {
		std::string& data = *params.data;

        // Construct path
		std::filesystem::path dataPath = params.resourceDestinationSettings.basePath / m_location.GetValue().ToString();

        bool res = ResourceTools::SaveFile( dataPath, data );

		if( res )
		{
			return Result{ ResultType::SUCCESS };
		}
		else
		{
			return Result{ ResultType::FAILED_TO_SAVE_FILE };
		}

    }

    Result ResourceInfo::PutDataStreamLocalRelative( ResourcePutDataStreamParams& params ) const
    {
		std::filesystem::path path = params.resourceDestinationSettings.basePath / m_relativePath.GetValue();

        bool res = params.dataStream->StartWrite( path );

		if( res )
		{
			return Result{ ResultType::SUCCESS };
		}
		else
		{
			return Result{ ResultType::FAILED_TO_SAVE_FILE };
		}
    }

    Result ResourceInfo::PutDataStreamLocalCdn( ResourcePutDataStreamParams& params ) const
    {
		// Construct path
		std::filesystem::path dataPath = params.resourceDestinationSettings.basePath / m_location.GetValue().ToString();

		bool res = params.dataStream->StartWrite( dataPath );

		if( res )
		{
			return Result{ ResultType::SUCCESS };
		}
		else
		{
			return Result{ ResultType::FAILED_TO_SAVE_FILE };
		}
    }

    Result ResourceInfo::PutDataStreamRemoteCdn( ResourcePutDataStreamParams& params ) const
	{
        // Ensure the correct type has been passed to compress
		ResourceTools::CompressedFileDataStreamOut* castTest = dynamic_cast<ResourceTools::CompressedFileDataStreamOut*>( params.dataStream );

        if (!castTest)
        {
			return Result{ ResultType::FAILED_TO_WRITE_TO_STREAM };
        }

        return PutDataStreamLocalCdn( params );
	}


    Result ResourceInfo::GetDataStream( ResourceGetDataStreamParams& params ) const
    {
		if( !params.dataStream )
		{
			return Result{ ResultType::FAILED_TO_OPEN_FILE, "Data Stream not provided." };
		}

        // GetDataStream attempts to retrieve resources from arbitary size number of base paths.
        // Each failure adds to the error string
        // If function ultimately ends in failure then all information of attempted locations are displayed
        std::string errorString = "";

        for( int i = 0; i < params.resourceSourceSettings.basePaths.size(); i++ )
		{

			switch( params.resourceSourceSettings.sourceType )
			{
			case ResourceSourceType::LOCAL_RELATIVE: 
            {
				Result getDataStreamLocalRelativeResult = GetDataStreamLocalRelative( params, i );

                if( getDataStreamLocalRelativeResult.type == ResultType::SUCCESS )
				{
					return getDataStreamLocalRelativeResult;
				}

                errorString += getDataStreamLocalRelativeResult.info;
				errorString += "\n";

				break;
			}
			case ResourceSourceType::LOCAL_CDN: 
            {
				Result getDataStreamLocalCdnResult = GetDataStreamLocalCdn( params, i );

                if( getDataStreamLocalCdnResult.type == ResultType::SUCCESS )
				{
					return getDataStreamLocalCdnResult;
				}

                errorString += getDataStreamLocalCdnResult.info;
				errorString += "\n";

				break;
			}

			case ResourceSourceType::REMOTE_CDN:
            {
				Result getDataStreamRemoteCdnResult = GetDataStreamRemoteCdn( params, i );

                if( getDataStreamRemoteCdnResult.type == ResultType::SUCCESS )
				{
					return getDataStreamRemoteCdnResult;
				}

                errorString += getDataStreamRemoteCdnResult.info;
				errorString += "\n";

				break;
            }
			default:

				return Result{ ResultType::FAILED_TO_OPEN_FILE };
			}
		}

        return Result{ ResultType::FAILED_TO_OPEN_FILE, errorString };
    }

	Result ResourceInfo::GetData( ResourceGetDataParams& params ) const
    {
		if( params.data == nullptr )
		{
			return Result{ ResultType::FAILED_TO_OPEN_FILE };
		}

        // GetDataStream attempts to retrieve resources from arbitary size number of base paths.
		// Each failure adds to the error string
		// If function ultimately ends in failure then all information of attempted locations are displayed
		std::string errorString = "";

        for( int i = 0; i < params.resourceSourceSettings.basePaths.size(); i++ )
        {
			switch( params.resourceSourceSettings.sourceType )
			{
			case ResourceSourceType::LOCAL_RELATIVE:
            {
				Result getDataLocalRelativeResult = GetDataLocalRelative( params, i );

                if (getDataLocalRelativeResult.type == ResultType::SUCCESS)
                {
					return getDataLocalRelativeResult;
                }

                errorString += getDataLocalRelativeResult.info;
				errorString += "\n";

				break;
            }
			case ResourceSourceType::LOCAL_CDN:
            {
				Result getDataLocalCdnResult = GetDataLocalCdn( params, i );

                if( getDataLocalCdnResult.type == ResultType::SUCCESS )
				{
					return getDataLocalCdnResult;
				}

                errorString += getDataLocalCdnResult.info;
				errorString += "\n";

				break;
            }
			case ResourceSourceType::REMOTE_CDN:
            {
				Result getDataRemoteCdn = GetDataRemoteCdn( params, i );

                if( getDataRemoteCdn.type == ResultType::SUCCESS )
				{
					return getDataRemoteCdn;
				}

				errorString += getDataRemoteCdn.info;
				errorString += "\n";

				break;
			}
			default:
				return Result{ ResultType::FAILED_TO_OPEN_FILE };
			}
        }

        return Result{ ResultType::FAILED_TO_OPEN_FILE, errorString };

    }

    Result ResourceInfo::GetDataLocalRelative( ResourceGetDataParams& params, const int basePathId ) const
    {
        if (basePathId >= params.resourceSourceSettings.basePaths.size())
        {
			return Result{ ResultType::FAILED_TO_OPEN_FILE_STREAM, "Internal component error" };
        }

		std::string& data = *params.data;

        std::filesystem::path dataPath = params.resourceSourceSettings.basePaths.at( basePathId ) / m_relativePath.GetValue();

		bool res = ResourceTools::GetLocalFileData( dataPath, data );

		if( res )
		{
			return Result{ ResultType::SUCCESS };
		}
		else
		{
			std::stringstream ss;

			ss << "Failed to open file at: " << dataPath;

			return Result{ ResultType::FAILED_TO_OPEN_FILE_STREAM, ss.str() };
		}
    }

    Result ResourceInfo::GetDataLocalCdn( ResourceGetDataParams& params, const int basePathId ) const
    {
		if( basePathId >= params.resourceSourceSettings.basePaths.size() )
		{
			return Result{ ResultType::FAILED_TO_OPEN_FILE_STREAM, "Internal component error" };
		}

		std::string& data = *params.data;

        // Construct path
        std::filesystem::path path = params.resourceSourceSettings.basePaths.at( basePathId ) / m_location.GetValue().ToString();

		bool res = ResourceTools::GetLocalFileData( path, data );

        if (res)
        {
			return Result{ ResultType::SUCCESS };
        }
        else
        {
			std::stringstream ss;

			ss << "Failed to open file at: " << path << "\n Relative Path: " << m_relativePath.GetValue();

			return Result{ ResultType::FAILED_TO_OPEN_FILE_STREAM, ss.str() };

        }
    }

	Result ResourceInfo::GetDataRemoteCdn( ResourceGetDataParams& params, const int basePathId ) const
    {
		if( basePathId >= params.resourceSourceSettings.basePaths.size() )
		{
			return Result{ ResultType::FAILED_TO_OPEN_FILE_STREAM, "Internal component error" };
		}

		std::filesystem::path path = params.resourceSourceSettings.basePaths.at( basePathId ) / m_location.GetValue().ToString();

        std::filesystem::path tempPath = params.cacheBasePath / m_location.GetValue().ToString();

        std::string url = path.string();

		std::replace( url.begin(), url.end(), '\\', '/' );

		bool haveFileCached{false};

		if( std::filesystem::exists( tempPath ) )
		{
			if( ResourceTools::Md5ChecksumMatches( tempPath, params.expectedChecksum ) )
			{
				haveFileCached = true;
			}
			else
			{
				std::filesystem::remove( tempPath );
			}
		}

		if( !haveFileCached )
		{
			ResourceTools::Downloader downloader;

			bool downloadFileResult = downloader.DownloadFile( url, tempPath.string(), params.downloadRetrySeconds );

			if (!downloadFileResult)
			{
				std::stringstream ss;

				ss << "Failed to download file \nfrom remote url: " << url << "\nto local path: " << tempPath.string();

				return Result{ ResultType::FAILED_TO_DOWNLOAD_FILE, ss.str() };
			}

			if( !params.expectedChecksum.empty() && !ResourceTools::Md5ChecksumMatches( tempPath, params.expectedChecksum ) )
			{
				return Result{ ResultType::FAILED_TO_DOWNLOAD_FILE, "The downloaded file does not have the expected checksum" };
			}
		}

		ResourceGetDataParams localParams = params;

        localParams.resourceSourceSettings.sourceType = ResourceSourceType::LOCAL_CDN;

        localParams.resourceSourceSettings.basePaths.clear();

        localParams.resourceSourceSettings.basePaths.push_back( params.cacheBasePath );

        // Attempt locally now it has been downloaded
		return GetDataLocalCdn( localParams, 0 );
        

    }

    Result ResourceInfo::GetDataStreamLocalRelative( ResourceGetDataStreamParams& params, const int basePathId ) const
    {
		if( basePathId >= params.resourceSourceSettings.basePaths.size() )
		{
			return Result{ ResultType::FAILED_TO_OPEN_FILE_STREAM, "Internal component error" };
		}

		std::filesystem::path dataPath = params.resourceSourceSettings.basePaths.at( basePathId ) / m_relativePath.GetValue();

		bool res = params.dataStream->StartRead( dataPath );

		if( res )
		{
			return Result{ ResultType::SUCCESS };
		}
		else
		{
			std::stringstream ss;

			ss << "Failed to open file at: " << dataPath;

			return Result{ ResultType::FAILED_TO_OPEN_FILE_STREAM, ss.str() };
		}
    }

    Result ResourceInfo::GetDataStreamLocalCdn( ResourceGetDataStreamParams& params, const int basePathId ) const
    {
		if( basePathId >= params.resourceSourceSettings.basePaths.size() )
		{
			return Result{ ResultType::FAILED_TO_OPEN_FILE_STREAM, "Internal component error" };
		}

		// Construct path
		std::filesystem::path path = params.resourceSourceSettings.basePaths.at( basePathId ) / m_location.GetValue().ToString();

		bool res = params.dataStream->StartRead( path );

		if( res )
		{
			return Result{ ResultType::SUCCESS };
		}
		else
		{
			std::stringstream ss;

			ss << "Failed to open file at: " << path << "\n Relative Path: " << m_relativePath.GetValue();

			return Result{ ResultType::FAILED_TO_OPEN_FILE_STREAM, ss.str() };
		}
    }

    Result ResourceInfo::GetDataStreamRemoteCdn( ResourceGetDataStreamParams& params, const int basePathId ) const
    {
		if( basePathId >= params.resourceSourceSettings.basePaths.size() )
		{
			return Result{ ResultType::FAILED_TO_OPEN_FILE_STREAM, "Internal component error" };
		}

		std::filesystem::path path = params.resourceSourceSettings.basePaths.at( basePathId ) / m_location.GetValue().ToString();

		std::filesystem::path tempPath = params.cacheBasePath / m_location.GetValue().ToString();

        std::string url = path.string();

        std::replace( url.begin(), url.end(), '\\', '/' );
		bool haveFileCached{false};

		if( std::filesystem::exists( tempPath ) )
		{
			if( ResourceTools::Md5ChecksumMatches( tempPath, params.expectedChecksum ) )
			{
				haveFileCached = true;
			}
			else
			{
				std::filesystem::remove( tempPath );
			}
		}

		if( !haveFileCached )
		{
			ResourceTools::Downloader downloader;

			bool downloadFileResult = downloader.DownloadFile( url, tempPath.string(), params.downloadRetrySeconds );

			if (!downloadFileResult)
			{
				std::stringstream ss;

				ss << "Failed to download file \nfrom remote url: " << url << "\nto local path: " << tempPath.string();

				return Result{ ResultType::FAILED_TO_DOWNLOAD_FILE, ss.str() };
			}

			if( !params.expectedChecksum.empty() && !ResourceTools::Md5ChecksumMatches( tempPath, params.expectedChecksum ) )
			{
				return Result{ ResultType::FAILED_TO_DOWNLOAD_FILE, "The downloaded file does not have the expected checksum" };
			}
		}

        ResourceGetDataStreamParams localCdnParams = params;

        localCdnParams.resourceSourceSettings.sourceType = ResourceSourceType::LOCAL_CDN;

        localCdnParams.resourceSourceSettings.basePaths.clear();

        localCdnParams.resourceSourceSettings.basePaths.push_back( params.cacheBasePath );

        return GetDataStreamLocalCdn( localCdnParams, 0 );

    }

    Result ResourceInfo::ImportFromYaml( YAML::Node& resource, const VersionInternal& documentVersion )
	{
    	if( m_binaryOperation.IsParameterExpectedInDocumentVersion( documentVersion ) )
    	{
    		if( YAML::Node parameter = resource[m_binaryOperation.GetTag()] )
    		{
    			m_binaryOperation = parameter.as<unsigned int>();
    		}
    		else
    		{
    			m_binaryOperation.Reset();
    		}
    	}

		if( m_relativePath.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( YAML::Node parameter = resource[m_relativePath.GetTag()] )
            {
				m_relativePath = parameter.as<std::string>();
            }
            else
            {
				return Result{ ResultType::MALFORMED_RESOURCE_INPUT };
            }
		}

		if( m_location.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( YAML::Node parameter = resource[m_location.GetTag()] )
			{
				m_location = parameter.as<std::string>();
			}
			else
			{
				return Result{ ResultType::MALFORMED_RESOURCE_INPUT };
			}
		}

        if( m_type.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( YAML::Node parameter = resource[m_type.GetTag()] )
			{
				m_type = parameter.as<std::string>();
			}
			else
			{
				return Result{ ResultType::MALFORMED_RESOURCE_INPUT };
			}
		}

		if( m_checksum.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( YAML::Node parameter = resource[m_checksum.GetTag()] )
			{
				m_checksum = parameter.as<std::string>();
			}
			else
			{
				return Result{ ResultType::MALFORMED_RESOURCE_INPUT };
			}
		}

		if( m_uncompressedSize.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( YAML::Node parameter = resource[m_uncompressedSize.GetTag()] )
			{
				m_uncompressedSize = parameter.as<uintmax_t>();
			}
			else
			{
				return Result{ ResultType::MALFORMED_RESOURCE_INPUT };
			}
		}

		if( m_compressedSize.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( YAML::Node parameter = resource[m_compressedSize.GetTag()] )
			{
				m_compressedSize = parameter.as<uintmax_t>();
			}
			else
			{
				return Result{ ResultType::MALFORMED_RESOURCE_INPUT };
			}

		}

    	if( m_prefix.IsParameterExpectedInDocumentVersion( documentVersion ) )
    	{
    		YAML::Node parameter = resource[m_prefix.GetTag()];
    		if( parameter.IsDefined() )
    		{
    			m_prefix = parameter.as<std::string>();
    		}
    	}


		return Result{ ResultType::SUCCESS };
	}

    std::string ResourceInfo::TypeId()
    {
		return "Resource";
    }

    Result ResourceInfo::SetParametersFromResource( const ResourceInfo* other, const VersionInternal& documentVersion )
    {
        if (!other)
        {
			return Result{ ResultType::FAIL };
        }

        if (m_relativePath.IsParameterExpectedInDocumentVersion( documentVersion ))
        {
			std::filesystem::path relativePath;

			Result getRelativePathResult = other->GetRelativePath( relativePath );

			if( getRelativePathResult.type != ResultType::SUCCESS )
			{
				return getRelativePathResult;
			}

			m_relativePath = relativePath;
        }
        
        if (m_location.IsParameterExpectedInDocumentVersion( documentVersion ))
        {
			std::string location;

			Result getLocationResult = other->GetLocation( location );

			if( getLocationResult.type != ResultType::SUCCESS )
			{
				return getLocationResult;
			}

			m_location = location;
        }
		
        if (m_checksum.IsParameterExpectedInDocumentVersion( documentVersion ))
        {
			std::string checksum;

			Result getChecksumResult = other->GetChecksum( checksum );

			if( getChecksumResult.type != ResultType::SUCCESS )
			{
				return getChecksumResult;
			}

			m_checksum = checksum;
        }
        
        if (m_uncompressedSize.IsParameterExpectedInDocumentVersion( documentVersion ))
        {
			uintmax_t uncompressedSize;

			Result getUncompressedSizeResult = other->GetUncompressedSize( uncompressedSize );

			if( getUncompressedSizeResult.type != ResultType::SUCCESS )
			{
				return getUncompressedSizeResult;
			}

			m_uncompressedSize = uncompressedSize;
        }
        
        if (m_compressedSize.IsParameterExpectedInDocumentVersion( documentVersion ))
        {
			uintmax_t compressedSize;

			Result getCompressedSizeResult = other->GetCompressedSize( compressedSize );

			if( getCompressedSizeResult.type != ResultType::SUCCESS )
			{
				return getCompressedSizeResult;
			}

			m_compressedSize = compressedSize;
        }

    	if (m_binaryOperation.IsParameterExpectedInDocumentVersion( documentVersion ))
    	{
    		unsigned int binaryOperation;

    		Result getBinaryOperationResult = other->GetBinaryOperation( binaryOperation );

    		if( getBinaryOperationResult.type != ResultType::SUCCESS )
    		{
    			if( getBinaryOperationResult.type == ResultType::RESOURCE_VALUE_NOT_SET )
    			{
    				m_binaryOperation.Reset();
    				return Result{ ResultType::SUCCESS };
    			}
    			return getBinaryOperationResult;
    		}
			if( binaryOperation != 0 )
			{
				m_binaryOperation = binaryOperation;
			}
    		else
    		{
    			m_binaryOperation.Reset();
    		}
    	}

        return Result{ ResultType::SUCCESS };
    }

    bool ResourceInfo::operator == ( const ResourceInfo* other ) const
	{
		if( !m_relativePath.HasValue() )
		{
			return false;
		}
		if( !other->m_relativePath.HasValue() )
		{
			return false;
		}
		// Equality is defined as having the same relative path, not same data
		return ( m_relativePath.GetValue() == other->m_relativePath.GetValue() );
	}

	bool ResourceInfo::operator < ( const ResourceInfo& other ) const
	{
		if( !m_relativePath.HasValue() )
		{
			return other.m_relativePath.HasValue();
		}
		if( !other.m_relativePath.HasValue() )
		{
			return true;
		}
		// Equality is defined as having the same relative path, not same data
		return m_relativePath.GetValue() < other.m_relativePath.GetValue();
	}

	Result ResourceInfo::UpdateLocation()
	{
        std::filesystem::path relativePath;

        Result getRelativePathResult = GetRelativePath( relativePath );

        if( getRelativePathResult.type != ResultType::SUCCESS )
        {
			return getRelativePathResult;
        }

        Location l;

		Result setLocationResult = l.SetFromRelativePathAndDataChecksum( relativePath, m_checksum.GetValue() );

        if (setLocationResult.type != ResultType::SUCCESS)
        {
			return setLocationResult;
        }

		m_location = l;
    	return Result({ ResultType::SUCCESS } );
	}

    Result ResourceInfo::SetParametersFromData( const std::string& data )
    {
        std::string checksum;

		if( !ResourceTools::GenerateMd5Checksum( data, checksum ) )
		{
			return Result{ ResultType::FAILED_TO_GENERATE_CHECKSUM };
		}

    	SetDataChecksum( checksum );

        std::string type;

        Result getTypeResult = GetType( type );

        if( getTypeResult.type != ResultType::SUCCESS )
        {
			return getTypeResult;
        }

        std::string compressedData;

        if (!ResourceTools::GZipCompressData(data, compressedData))
        {
			return Result{ ResultType::FAILED_TO_COMPRESS_DATA };
        }

        m_compressedSize = compressedData.size();

        m_uncompressedSize = data.size();

        return Result{ ResultType::SUCCESS };
        
    }

	Result ResourceInfo::SetParametersFromSourceStream( ResourceTools::FileDataStreamIn& stream, size_t matchSize )
	{
		std::string chunk;
		std::string checksum;

    	m_uncompressedSize = matchSize;

    	auto start = stream.GetCurrentPosition();

    	ResourceTools::Md5ChecksumStream md5ChecksumStream;
    	while(matchSize)
    	{

    		stream >> chunk;
    		size_t chunkSize = chunk.size() > matchSize ? matchSize : chunk.size();
    		md5ChecksumStream << chunk.substr( chunkSize );
    		matchSize -= chunkSize;
    		if(!chunkSize)
    		{
				return Result{ ResultType::FAILED_TO_GENERATE_CHECKSUM };
    		}
    	}

    	if (!md5ChecksumStream.FinishAndRetrieve(checksum))
    	{
    		stream.Seek( start );
			return Result{ ResultType::FAILED_TO_GENERATE_CHECKSUM };
    	}

    	m_checksum = checksum;
    	stream.Seek( start );
		return Result{ ResultType::SUCCESS };
	}

    Result ResourceInfo::ExportToYaml( YAML::Emitter& out, const VersionInternal& documentVersion )
	{
		// Relative path
		if( m_relativePath.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( !m_relativePath.HasValue() )
			{
				return Result{ ResultType::REQUIRED_RESOURCE_PARAMETER_NOT_SET };
			}

            std::filesystem::path relativePath;

            Result getRelativePathResult = GetRelativePath( relativePath );

            if (getRelativePathResult.type != ResultType::SUCCESS)
            {
				return getRelativePathResult;
            }

            std::string relativePathStr = relativePath.string();
			std::replace( relativePathStr.begin(), relativePathStr.end(), '\\', '/' );

			out << YAML::Key << m_relativePath.GetTag();
			out << YAML::Value << relativePathStr;
		}

        // Type
		if( m_type.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( !m_type.HasValue() )
			{
				return Result{ ResultType::REQUIRED_RESOURCE_PARAMETER_NOT_SET };
			}

			out << YAML::Key << m_type.GetTag();
			out << YAML::Value << m_type.GetValue();
		}

        // Location
		if( m_location.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( !m_location.HasValue() )
			{
				return Result{ ResultType::REQUIRED_RESOURCE_PARAMETER_NOT_SET };
			}

			out << YAML::Key << m_location.GetTag();
			out << YAML::Value << m_location.GetValue().ToString();
		}

		// Checksum
		if( m_checksum.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( !m_checksum.HasValue() )
			{
				return Result{ ResultType::REQUIRED_RESOURCE_PARAMETER_NOT_SET };
			}

			out << YAML::Key << m_checksum.GetTag();
			out << YAML::Value << m_checksum.GetValue();
		}

		if( m_uncompressedSize.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( !m_uncompressedSize.HasValue() )
			{
				return Result{ ResultType::REQUIRED_RESOURCE_PARAMETER_NOT_SET };
			}

			out << YAML::Key << m_uncompressedSize.GetTag();
			out << YAML::Value << m_uncompressedSize.GetValue();
		}

        // Compressed Size
		if( m_compressedSize.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( !m_compressedSize.HasValue() )
			{
				return Result{ ResultType::REQUIRED_RESOURCE_PARAMETER_NOT_SET };
			}

			out << YAML::Key << m_compressedSize.GetTag();
			out << YAML::Value << m_compressedSize.GetValue();
		}

    	// Binary Operation
    	if( m_binaryOperation.IsParameterExpectedInDocumentVersion( documentVersion ) )
    	{
    		// This is an optional field
    		if( m_binaryOperation.HasValue() )
    		{
    			out << YAML::Key << m_binaryOperation.GetTag();
    			out << YAML::Value << m_binaryOperation.GetValue();
    		}
    	}

    	if( m_prefix.IsParameterExpectedInDocumentVersion( documentVersion ) )
    	{
    		// This is an optional field
    		if( m_prefix.HasValue() )
    		{
    			out << YAML::Key << m_prefix.GetTag();
    			out << YAML::Value << m_prefix.GetValue();
    		}
    	}

        /*
        // Clean this up after ABI thought exercises finished


		if( m_something.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( !m_something.HasValue() )
			{
				return Result::REQUIRED_RESOURCE_PARAMETER_NOT_SET;
			}

			out << YAML::Key << m_something.GetTag();
			out << YAML::Value << m_something.GetValue();
		}
        */

		return Result{ ResultType::SUCCESS };
	}

	void ResourceInfo::SetDataChecksum( const std::string& checksum )
    {
	    m_checksum = checksum;

    	UpdateLocation();
    }

	void ResourceInfo::SetCompressedSize( uintmax_t compressedSize )
    {
	    m_compressedSize = compressedSize;
    }

	void ResourceInfo::SetUncompressedSize( uintmax_t uncompressedSize )
	{
		m_uncompressedSize = uncompressedSize;
	}


    Result ResourceInfo::ExportToCsv( std::string& out, const VersionInternal& documentVersion )
    {
    	std::stringstream result;

    	if( m_prefix.HasValue() )
    	{
    		result << m_prefix.GetValue() << ":/";
    	}

    	// Relative path
		if( !m_relativePath.HasValue() )
		{
			return Result{ ResultType::REQUIRED_RESOURCE_PARAMETER_NOT_SET };
		}

        std::filesystem::path relativePath;

        Result getRelativePathResult = GetRelativePath( relativePath );

        if (getRelativePathResult.type != ResultType::SUCCESS)
        {
			return getRelativePathResult;
        }

        std::string relativePathStr = relativePath.string();
		std::replace( relativePathStr.begin(), relativePathStr.end(), '\\', '/' );

		result << relativePathStr << ",";


        // Location
		if( !m_location.HasValue() )
		{
			return Result{ ResultType::REQUIRED_RESOURCE_PARAMETER_NOT_SET };
		}

		result << m_location.GetValue().ToString() << ",";

		// Checksum
		if( !m_checksum.HasValue() )
		{
			return Result{ ResultType::REQUIRED_RESOURCE_PARAMETER_NOT_SET };
		}

		result << m_checksum.GetValue() << ",";

		if( !m_uncompressedSize.HasValue() )
		{
			return Result{ ResultType::REQUIRED_RESOURCE_PARAMETER_NOT_SET };
		}

		result << m_uncompressedSize.GetValue() << ",";

        // Compressed Size
		if( !m_compressedSize.HasValue() )
		{
			return Result{ ResultType::REQUIRED_RESOURCE_PARAMETER_NOT_SET };
		}

		result << m_compressedSize.GetValue(); // End of required fields

    	// This is an optional field
    	if( m_binaryOperation.HasValue() )
    	{
    		result << "," << m_binaryOperation.GetValue();
    	}

    	out = result.str();

		return Result{ ResultType::SUCCESS };
    }

}