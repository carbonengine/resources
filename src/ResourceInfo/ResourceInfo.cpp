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

    std::filesystem::path ResourceInfo::GetRelativePath() const
    {
		return m_relativePath.GetValue();
    }

    void ResourceInfo::SetRelativePath( const std::filesystem::path& relativePath )
    {
		m_relativePath = relativePath;
    }

    std::string ResourceInfo::GetLocation() const
    {
		return m_location.GetValue().ToString();
    }

    std::string ResourceInfo::GetType() const
    {
		return m_type.GetValue();
    }

    std::string ResourceInfo::GetChecksum() const
    {
		return m_checksum.GetValue();
    }

    unsigned long ResourceInfo::GetUncompressedSize() const
    {
        // TODO what if they don't have a value as they are optional
        // probably needs to be refactored to return Result as return like everything else
		return m_uncompressedSize.GetValue();
    }

    unsigned long ResourceInfo::GetCompressedSize() const
    {
		return m_compressedSize.GetValue();
    }

    unsigned long ResourceInfo::GetSomething() const
    {
		return m_something.GetValue();
    }
   
    Result ResourceInfo::PutData( ResourcePutDataParams& params ) const
    {
		bool dataWasSavedSomewhere = false;

		if( params.resourceDestinationSettings.developmentLocalBasePath != "" )
        {
			Result putDevelopmentLocalDataResult = PutDevelopmentLocalData( params );

			if( putDevelopmentLocalDataResult != Result::SUCCESS )
			{
				return putDevelopmentLocalDataResult;
			}

            dataWasSavedSomewhere = true;
        }
		
        
        if (params.resourceDestinationSettings.productionLocalBasePath != "")
        {
			Result putProductionLocalDataResult = PutProductionLocalData( params );

			if( putProductionLocalDataResult != Result::SUCCESS )
			{
				return putProductionLocalDataResult;
			}

            dataWasSavedSomewhere = true;
        }
        
        if (dataWasSavedSomewhere)
        {
			return Result::SUCCESS;
        }
        else
        {
			return Result::FAILED_TO_SAVE_FILE;
        }

    }

    Result ResourceInfo::PutDevelopmentLocalData( ResourcePutDataParams& params ) const
    {
		std::filesystem::path path = params.resourceDestinationSettings.developmentLocalBasePath / m_relativePath.GetValue();

        bool res = ResourceTools::SaveFile( path, params.data );

        if (res)
        {
			return Result::SUCCESS;
        }
        else
        {
			return Result::FAILED_TO_SAVE_FILE;
        }

    }

    Result ResourceInfo::PutProductionLocalData( ResourcePutDataParams& params ) const
    {
        // Construct path
		std::filesystem::path dataPath = params.resourceDestinationSettings.productionLocalBasePath / m_location.GetValue().ToString();

        bool res = ResourceTools::SaveFile( dataPath, params.data );

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
        // Get file from development local
		Result getDevelopmentLocalDataResult = GetDevelopmentLocalData( params );

		if( getDevelopmentLocalDataResult == Result::SUCCESS )
		{
			return Result::SUCCESS;
		}

		// Get file from production local
		Result getProductionLocalDataResult = GetProductionLocalData( params );

		if( getProductionLocalDataResult == Result::SUCCESS )
		{
			return Result::SUCCESS;
		}

		// Get file from production remote
		Result getProductionRemoteDataResult = GetProductionRemoteData( params );

		if( getProductionRemoteDataResult == Result::SUCCESS )
		{
			return Result::SUCCESS;
		}

		return Result::FAILED_TO_OPEN_FILE;
    }

    Result ResourceInfo::GetDevelopmentLocalData( ResourceGetDataParams& params ) const
    {
        // Early out based on which arguments were provided
        if (params.resourceSourceSettings.developmentLocalBasePath == "")
        {
			return Result::FAIL;
        }

        std::filesystem::path dataPath = params.resourceSourceSettings.developmentLocalBasePath / m_relativePath.GetValue();

		bool res = ResourceTools::GetLocalFileData( dataPath, params.data );

		if( res )
		{
			return Result::SUCCESS;
		}
		else
		{
			return Result::FAILED_TO_OPEN_LOCAL_FILE;
		}
    }

    Result ResourceInfo::GetProductionLocalData( ResourceGetDataParams& params ) const
    {
		// Early out based on which arguments were provided
		if( params.resourceSourceSettings.productionLocalBasePath == "" )
		{
			return Result::FAIL;
		}

        // Construct path
        std::filesystem::path path = params.resourceSourceSettings.productionLocalBasePath / m_location.GetValue().ToString();

		bool res = ResourceTools::GetLocalFileData( path, params.data );

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
	Result ResourceInfo::GetProductionRemoteData( ResourceGetDataParams& params ) const
    {
		// Early out based on which arguments were provided
		if( params.resourceSourceSettings.productionRemoteBaseUrl == "" )
		{
			return Result::FAIL;
		}

		bool downloadFileResult = ResourceTools::DownloadFile( "URL", "outputPath" );

        if (!downloadFileResult)
        {
			return Result::FAILED_TO_OPEN_REMOTE_FILE;
        }
        else
        {
            // Attempt locally now it has been downloaded
			return GetProductionLocalData( params );
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
		m_relativePath = other->GetRelativePath();

        m_location = other->GetLocation();

        m_checksum = other->GetChecksum();

        m_uncompressedSize = other->GetUncompressedSize();

        m_compressedSize = other->GetCompressedSize();

        m_something = other->GetSomething();

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

        Location l;

		l.SetFromRelativePathAndDataChecksum( GetType(), GetRelativePath(), checksum );

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

            std::string relativePathStr = GetRelativePath().string();
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
			out << YAML::Value << GetType();
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