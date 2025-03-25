#include "ResourceInfo.h"

#include <sstream>

#include <ResourceTools.h>

#include <FileDataStreamIn.h>

#include <FileDataStreamOut.h>

#include <yaml-cpp/yaml.h>

#include "..\ResourceGroupImpl.h"

namespace CarbonResources
{
    // TODO split this into own file
    Result Location::SetFromRelativePathAndDataChecksum(const std::filesystem::path& relativePath, const std::string& dataChecksum)
    {
		std::string relativePathChecksum = "";

		if( !ResourceTools::GenerateFowlerNollVoChecksum( relativePath.string(), relativePathChecksum ) )
		{
			return Result::FAILED_TO_GENERATE_RELATIVE_PATH_CHECKSUM;
		}

		std::stringstream ss2;// TODO naming and pull out of here
		ss2 << relativePathChecksum.substr( 0, 2 );
		ss2 << "/";
		ss2 << relativePathChecksum;
		ss2 << "_";
		ss2 << dataChecksum;

		location = ss2.str();

		return Result::SUCCESS;
    }


    ResourceInfo::ResourceInfo( const ResourceInfoParams& params )
    {
		m_relativePath = params.relativePath;

		m_location = params.location;

        m_type = TypeId();
		
		m_checksum = params.checksum;

		m_compressedSize = params.compressedSize;

		m_uncompressedSize = params.uncompressedSize;
    }

    ResourceInfo::~ResourceInfo()
    {

    }

    Result ResourceInfo::GetRelativePath(std::filesystem::path& relativePath) const
    {
        if (!m_relativePath.HasValue())
        {
			return Result::RESOURCE_VALUE_NOT_SET;
        }
        else
        {
			relativePath = m_relativePath.GetValue();

            return Result::SUCCESS;
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
			return Result::RESOURCE_VALUE_NOT_SET;
		}
		else
		{
			location = m_location.GetValue().ToString();

			return Result::SUCCESS;
		}
    }

    Result ResourceInfo::GetType(std::string& type) const
    {
		if( !m_type.HasValue() )
		{
			return Result::RESOURCE_VALUE_NOT_SET;
		}
		else
		{
			type = m_type.GetValue();

			return Result::SUCCESS;
		}
    }

    Result ResourceInfo::GetChecksum(std::string& checksum) const
    {
		if( !m_checksum.HasValue() )
		{
			return Result::RESOURCE_VALUE_NOT_SET;
		}
		else
		{
			checksum = m_checksum.GetValue();

			return Result::SUCCESS;
		}
    }

    Result ResourceInfo::GetUncompressedSize(unsigned long& uncompressedSize) const
    {
		if( !m_uncompressedSize.HasValue() )
		{
			return Result::RESOURCE_VALUE_NOT_SET;
		}
		else
		{
			uncompressedSize = m_uncompressedSize.GetValue();

			return Result::SUCCESS;
		}
    }

    Result ResourceInfo::GetCompressedSize(unsigned long& compressedSize) const
    {
		if( !m_compressedSize.HasValue() )
		{
			return Result::RESOURCE_VALUE_NOT_SET;
		}
		else
		{
			compressedSize = m_compressedSize.GetValue();

			return Result::SUCCESS;
		}
    }

    Result ResourceInfo::PutDataStream( ResourcePutDataStreamParams& params ) const
    {
		if( !params.dataStream )
		{
			return Result::FAILED_TO_SAVE_FILE;
		}

        switch( params.resourceDestinationSettings.destinationType )
		{
		case ResourceDestinationType::LOCAL_RELATIVE:

			return PutDataStreamLocalRelative( params );

			break;

		case ResourceDestinationType::LOCAL_CDN:

			return PutDataStreamLocalCdn( params );

			break;

		default:
			return Result::FAILED_TO_SAVE_FILE;
		}
    }
   
    Result ResourceInfo::PutData( ResourcePutDataParams& params ) const
    {
        if (!params.data)
        {
			return Result::FAILED_TO_SAVE_FILE;
        }

        switch (params.resourceDestinationSettings.destinationType)
        {
		case ResourceDestinationType::LOCAL_RELATIVE:

			return PutDataLocalRelative( params );

			break;

        case ResourceDestinationType::LOCAL_CDN:

			return PutDataLocalCdn( params );
		
			break;

		default:
			return Result::FAILED_TO_SAVE_FILE;

        }

    }

    Result ResourceInfo::PutDataLocalRelative( ResourcePutDataParams& params ) const
    {
		std::string& data = *params.data;

		std::filesystem::path path = params.resourceDestinationSettings.basePath / m_relativePath.GetValue();

        bool res = ResourceTools::SaveFile( path, data );

        if (res)
        {
			return Result::SUCCESS;
        }
        else
        {
			return Result::FAILED_TO_SAVE_FILE;
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
			return Result::SUCCESS;
		}
		else
		{
			return Result::FAILED_TO_SAVE_FILE;
		}

    }

    Result ResourceInfo::PutDataStreamLocalRelative( ResourcePutDataStreamParams& params ) const
    {
		std::filesystem::path path = params.resourceDestinationSettings.basePath / m_relativePath.GetValue();

        bool res = params.dataStream->StartWrite( path );

		if( res )
		{
			return Result::SUCCESS;
		}
		else
		{
			return Result::FAILED_TO_SAVE_FILE;
		}
    }

    Result ResourceInfo::PutDataStreamLocalCdn( ResourcePutDataStreamParams& params ) const
    {
		// Construct path
		std::filesystem::path dataPath = params.resourceDestinationSettings.basePath / m_location.GetValue().ToString();

		bool res = params.dataStream->StartWrite( dataPath );

		if( res )
		{
			return Result::SUCCESS;
		}
		else
		{
			return Result::FAILED_TO_SAVE_FILE;
		}
    }


    Result ResourceInfo::GetDataStream( ResourceGetDataStreamParams& params ) const
    {
		if( !params.dataStream )
		{
			return Result::FAILED_TO_OPEN_FILE;
		}

		switch( params.resourceSourceSettings.sourceType )
		{
		case ResourceSourceType::LOCAL_RELATIVE:

			return GetDataStreamLocalRelative( params );

			break;

		case ResourceSourceType::LOCAL_CDN:

			return GetDataStreamLocalCdn( params );

			break;

		case ResourceSourceType::REMOTE_CDN:

			return GetDataStreamRemoteCdn( params );

			break;

		default:
			return Result::FAILED_TO_OPEN_FILE;
		}
    }

	Result ResourceInfo::GetData( ResourceGetDataParams& params ) const
    {
        if (!params.data)
        {
			return Result::FAILED_TO_OPEN_FILE;
        }

        switch (params.resourceSourceSettings.sourceType)
        {
		case ResourceSourceType::LOCAL_RELATIVE:

			return GetDataLocalRelative( params );

			break;

		case ResourceSourceType::LOCAL_CDN:

            return GetDataLocalCdn( params );

			break;

		case ResourceSourceType::REMOTE_CDN:

            return GetDataRemoteCdn( params );

			break;

		default:
			return Result::FAILED_TO_OPEN_FILE;
        }

    }

    Result ResourceInfo::GetDataLocalRelative( ResourceGetDataParams& params ) const
    {
		std::string& data = *params.data;

        std::filesystem::path dataPath = params.resourceSourceSettings.basePath / m_relativePath.GetValue();

		bool res = ResourceTools::GetLocalFileData( dataPath, data );

		if( res )
		{
			return Result::SUCCESS;
		}
		else
		{
			return Result::FAILED_TO_OPEN_FILE;
		}
    }

    Result ResourceInfo::GetDataLocalCdn( ResourceGetDataParams& params ) const
    {
		std::string& data = *params.data;

        // Construct path
        std::filesystem::path path = params.resourceSourceSettings.basePath / m_location.GetValue().ToString();

		bool res = ResourceTools::GetLocalFileData( path, data );

        if (res)
        {
			return Result::SUCCESS;
        }
        else
        {
			return Result::FAILED_TO_OPEN_FILE;
        }
    }

    // TODO this is where retry logic should reside
	Result ResourceInfo::GetDataRemoteCdn( ResourceGetDataParams& params ) const
    {

		bool downloadFileResult = ResourceTools::DownloadFile( "URL", "outputPath" );

        if (!downloadFileResult)
        {
			return Result::FAILED_TO_DOWNLOAD_FILE;
        }
        else
        {
            // Attempt locally now it has been downloaded
			return GetDataLocalCdn( params );
        }

    }

    Result ResourceInfo::GetDataStreamLocalRelative( ResourceGetDataStreamParams& params ) const
    {
		std::filesystem::path dataPath = params.resourceSourceSettings.basePath / m_relativePath.GetValue();

		bool res = params.dataStream->StartRead( dataPath );

		if( res )
		{
			return Result::SUCCESS;
		}
		else
		{
			return Result::FAILED_TO_OPEN_FILE_STREAM;
		}
    }

    Result ResourceInfo::GetDataStreamLocalCdn( ResourceGetDataStreamParams& params ) const
    {
		// Construct path
		std::filesystem::path path = params.resourceSourceSettings.basePath / m_location.GetValue().ToString();

		bool res = params.dataStream->StartRead( path );

		if( res )
		{
			return Result::SUCCESS;
		}
		else
		{
			return Result::FAILED_TO_OPEN_FILE_STREAM;
		}
    }

    Result ResourceInfo::GetDataStreamRemoteCdn( ResourceGetDataStreamParams& params ) const
    {
        // TODO streaming from download needs work
		return Result::FAIL;
    }

    Result ResourceInfo::ImportFromYaml( YAML::Node& resource, const VersionInternal& documentVersion )
	{

		if( m_relativePath.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( YAML::Node parameter = resource[m_relativePath.GetTag()] )
            {
				m_relativePath = parameter.as<std::string>();
            }
            else
            {
				return Result::MALFORMED_RESOURCE_INPUT;
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
				return Result::MALFORMED_RESOURCE_INPUT;
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
				return Result::MALFORMED_RESOURCE_INPUT;
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
				return Result::MALFORMED_RESOURCE_INPUT;
			}
		}

		if( m_uncompressedSize.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( YAML::Node parameter = resource[m_uncompressedSize.GetTag()] )
			{
				m_uncompressedSize = parameter.as<unsigned long>();
			}
			else
			{
				return Result::MALFORMED_RESOURCE_INPUT;
			}
		}

		if( m_compressedSize.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( YAML::Node parameter = resource[m_compressedSize.GetTag()] )
			{
				m_compressedSize = parameter.as<unsigned long>();
			}
			else
			{
				return Result::MALFORMED_RESOURCE_INPUT;
			}

		}


		return Result::SUCCESS;
	}

    std::string ResourceInfo::TypeId()
    {
		return "Resource";
    }

    Result ResourceInfo::SetParametersFromResource( const ResourceInfo* other, const VersionInternal& documentVersion )
    {
        // TODO this needs to be version aware
        if (!other)
        {
			return Result::FAIL;
        }

        if (m_relativePath.IsParameterExpectedInDocumentVersion(documentVersion))
        {
			std::filesystem::path relativePath;

			Result getRelativePathResult = other->GetRelativePath( relativePath );

			if( getRelativePathResult != Result::SUCCESS )
			{
				return getRelativePathResult;
			}

			m_relativePath = relativePath;
        }
        
        if (m_location.IsParameterExpectedInDocumentVersion(documentVersion))
        {
			std::string location;

			Result getLocationResult = other->GetLocation( location );

			if( getLocationResult != Result::SUCCESS )
			{
				return getLocationResult;
			}

			m_location = location;
        }
		
        if (m_checksum.IsParameterExpectedInDocumentVersion(documentVersion))
        {
			std::string checksum;

			Result getChecksumResult = other->GetChecksum( checksum );

			if( getChecksumResult != Result::SUCCESS )
			{
				return getChecksumResult;
			}

			m_checksum = checksum;
        }
        
        if (m_uncompressedSize.IsParameterExpectedInDocumentVersion(documentVersion))
        {
			unsigned long uncompressedSize;

			Result getUncompressedSizeResult = other->GetUncompressedSize( uncompressedSize );

			if( getUncompressedSizeResult != Result::SUCCESS )
			{
				return getUncompressedSizeResult;
			}

			m_uncompressedSize = uncompressedSize;
        }
        
        if (m_compressedSize.IsParameterExpectedInDocumentVersion(documentVersion))
        {
			unsigned long compressedSize;

			Result getCompressedSizeResult = other->GetCompressedSize( compressedSize );

			if( getCompressedSizeResult != Result::SUCCESS )
			{
				return getCompressedSizeResult;
			}

			m_compressedSize = compressedSize;
        }

        return Result::SUCCESS;
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

    Result ResourceInfo::SetParametersFromData( const std::string& data )
    {
        std::string checksum;

		if( !ResourceTools::GenerateMd5Checksum( data, checksum ) )
		{
			return Result::FAILED_TO_GENERATE_CHECKSUM;
		}

        m_checksum = checksum;

        std::string type;

        Result getTypeResult = GetType( type );

        if( getTypeResult != Result::SUCCESS )
        {
			return getTypeResult;
        }

        std::filesystem::path relativePath;

        Result getRelativePathResult = GetRelativePath( relativePath );

        if( getRelativePathResult != Result::SUCCESS )
        {
			return getRelativePathResult;
        }

        Location l;

		l.SetFromRelativePathAndDataChecksum( relativePath, checksum );

		m_location = l;

        std::string compressedData = "";

        /*
        * TODO reinstate
        if (!ResourceTools::GZipCompressData(data, compressedData))
        {
			return Result::FAILED_TO_COMPRESS_DATA;
        }
        */

        m_compressedSize = compressedData.size();

        m_uncompressedSize = data.size();

        return Result::SUCCESS;
        
    }

    Result ResourceInfo::ExportToYaml( YAML::Emitter& out, const VersionInternal& documentVersion )
	{
		// Relative path
		if( m_relativePath.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( !m_relativePath.HasValue() )
			{
				return Result::REQUIRED_RESOURCE_PARAMETER_NOT_SET;
			}

            std::filesystem::path relativePath;

            Result getRelativePathResult = GetRelativePath( relativePath );

            if (getRelativePathResult != Result::SUCCESS)
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
				return Result::REQUIRED_RESOURCE_PARAMETER_NOT_SET;
			}

			out << YAML::Key << m_type.GetTag();
			out << YAML::Value << m_type.GetValue();
		}

        // Location
		if( m_location.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( !m_location.HasValue() )
			{
				return Result::REQUIRED_RESOURCE_PARAMETER_NOT_SET;
			}

			out << YAML::Key << m_location.GetTag();
			out << YAML::Value << m_location.GetValue().ToString();
		}

		// Checksum
		if( m_checksum.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( !m_checksum.HasValue() )
			{
				return Result::REQUIRED_RESOURCE_PARAMETER_NOT_SET;
			}

			out << YAML::Key << m_checksum.GetTag();
			out << YAML::Value << m_checksum.GetValue();
		}

		if( m_uncompressedSize.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( !m_uncompressedSize.HasValue() )
			{
				return Result::REQUIRED_RESOURCE_PARAMETER_NOT_SET;
			}

			out << YAML::Key << m_uncompressedSize.GetTag();
			out << YAML::Value << m_uncompressedSize.GetValue();
		}

        // Compressed Size
		if( m_compressedSize.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( !m_compressedSize.HasValue() )
			{
				return Result::REQUIRED_RESOURCE_PARAMETER_NOT_SET;
			}

			out << YAML::Key << m_compressedSize.GetTag();
			out << YAML::Value << m_compressedSize.GetValue();
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

		return Result::SUCCESS;
	}

}