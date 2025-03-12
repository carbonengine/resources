#include "ResourceInfo.h"

#include <sstream>

#include <ResourceTools.h>

#include <yaml-cpp/yaml.h>

#include "..\ResourceGroupImpl.h"

namespace CarbonResources
{
    // TODO split this into own file
    Result Location::SetFromRelativePathAndDataChecksum(const std::string& resourceType, const std::filesystem::path& relativePath, const std::string& dataChecksum)
    {
		std::string relativePathChecksum = "";

        std::stringstream ss;

        // By concatenating the type with the path files with the same relativePath can exist on the CDN
        ss << resourceType;

        ss << ":/";

        ss << relativePath;

		std::string relativePathWithPrefix = ss.str();

		if( !ResourceTools::GenerateFowlerNollVoChecksum( relativePathWithPrefix, relativePathChecksum ) )
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

		m_something = params.something;
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

    Result ResourceInfo::GetSomething(unsigned long& something) const
    {
		if( !m_something.HasValue() )
		{
			return Result::RESOURCE_VALUE_NOT_SET;
		}
		else
		{
			something = m_something.GetValue();

			return Result::SUCCESS;
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

    // TODO what if many options are filled out for source?
    // TODO optionally run a validation on the data against checksum
	Result ResourceInfo::GetData( ResourceGetDataParams& params ) const
    {
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
			return Result::FAILED_TO_OPEN_LOCAL_FILE;
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
			return Result::FAILED_TO_OPEN_LOCAL_FILE;
        }
    }

    // TODO this is where retry logic should reside
	Result ResourceInfo::GetDataRemoteCdn( ResourceGetDataParams& params ) const
    {

		bool downloadFileResult = ResourceTools::DownloadFile( "URL", "outputPath" );

        if (!downloadFileResult)
        {
			return Result::FAILED_TO_OPEN_REMOTE_FILE;
        }
        else
        {
            // Attempt locally now it has been downloaded
			return GetDataLocalCdn( params );
        }

    }

    Result ResourceInfo::ImportFromYaml( YAML::Node& resource, const Version& documentVersion )
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

        /*
        TODO finish ABI experiments and clean this all up when document version tests are done

		if( m_something.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			//TODO handle failure
			m_something = resource[m_something.GetTag()].as<unsigned long>();
		}
        */

		return Result::SUCCESS;
	}

    std::string ResourceInfo::TypeId()
    {
		return "Resource";
    }

    Result ResourceInfo::SetParametersFromResource( const ResourceInfo* other )
    {
		std::filesystem::path relativePath;

        if (other->GetRelativePath(relativePath) == Result::SUCCESS)
        {
			m_relativePath = relativePath;
        }

		std::string location;

        if (other->GetLocation(location) == Result::SUCCESS)
        {
			m_location = location;
        }

        std::string checksum;

        if (other->GetChecksum(checksum) == Result::SUCCESS)
        {
			m_checksum = checksum;
        }

        unsigned long uncompressedSize;

        if (other->GetUncompressedSize(uncompressedSize) == Result::SUCCESS)
        {
			m_uncompressedSize = uncompressedSize;
        }

        unsigned long compressedSize;

        if( other->GetCompressedSize(compressedSize) == Result::SUCCESS )
        {
			m_compressedSize = compressedSize;
        }

        unsigned long something;

        if (other->GetSomething(something) == Result::SUCCESS)
        {
			m_something = something;
        }

        return Result::SUCCESS;
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

		l.SetFromRelativePathAndDataChecksum( type, relativePath, checksum );

		m_location = l;

        std::string compressedData = "";

        if (!ResourceTools::GZipCompressData(data, compressedData))
        {
			return Result::FAILED_TO_COMPRESS_DATA;
        }

        m_compressedSize = compressedData.size();

        m_uncompressedSize = data.size();

        // TODO guard thought exercise
        m_something = 101;
        
    }

    Result ResourceInfo::ExportToYaml( YAML::Emitter& out, const Version& documentVersion )
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