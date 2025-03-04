#include "ResourceImpl.h"

#include <sstream>

#include <ResourceTools.h>

#include <yaml-cpp/yaml.h>

namespace CarbonResources
{

    ResourceImpl::ResourceImpl( const ResourceParams& params )
    {
		m_relativePath = params.relativePath;

		m_location = params.location;
		
		m_checksum = params.checksum;

		m_compressedSize = params.compressedSize;

		m_uncompressedSize = params.uncompressedSize;

        // TODO this is an example of how this could be managed, nothing yet setup to formally test
        // This would need to happen here reinstate when things work again
		//BINARY_GUARD_RETURN( 1, 1, 0 );

		m_something = params.something;
    }

    ResourceImpl::~ResourceImpl()
    {

    }

    DocumentParameter<RelativePath> ResourceImpl::GetRelativePath() const
    {
		return m_relativePath;
    }

    void ResourceImpl::SetRelativePath( const RelativePath& relativePath )
    {
		m_relativePath = relativePath;
    }

    DocumentParameter<std::string> ResourceImpl::GetLocation() const
    {
		return m_location;
    }

    DocumentParameter<std::string> ResourceImpl::GetChecksum() const
    {
		return m_checksum;
    }

    DocumentParameter<unsigned long> ResourceImpl::GetUncompressedSize() const
    {
		return m_uncompressedSize;
    }

    DocumentParameter<unsigned long> ResourceImpl::GetCompressedSize() const
    {
		return m_compressedSize;
    }

    DocumentParameter<unsigned long> ResourceImpl::GetSomething() const
    {
		return m_something;
    }

    Result ResourceImpl::GetData( ResourceGetDataParams& params ) const
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

    Result ResourceImpl::GetDevelopmentLocalData( ResourceGetDataParams& params ) const
    {
        // Early out based on which arguments were provided
        if (params.resourceSourceSettings.developmentLocalBasePath == "")
        {
			return Result::FAIL;
        }

        // TODO break out as to reduce duplication from production local
		// Construct path
		std::stringstream ss;

		ss << params.resourceSourceSettings.developmentLocalBasePath;

		ss << m_relativePath.GetValue().filename;

		std::string path = ss.str();

		bool res = ResourceTools::GetLocalFileData( path, params.data );

		if( res )
		{
			return Result::SUCCESS;
		}
		else
		{
			return Result::FAILED_TO_OPEN_LOCAL_FILE;
		}
    }

    Result ResourceImpl::GetProductionLocalData( ResourceGetDataParams& params ) const
    {
		// Early out based on which arguments were provided
		if( params.resourceSourceSettings.productionLocalBasePath == "" )
		{
			return Result::FAIL;
		}

		// Early out based on which arguments were provided
		if( params.resourceSourceSettings.productionLocalBasePath == "" )
		{
			return Result::FAIL;
		}

        // Construct path
		std::stringstream ss;

		ss << params.resourceSourceSettings.productionLocalBasePath;

		ss << m_relativePath.GetValue().filename;

		std::string path = ss.str();

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

    Result ResourceImpl::GetProductionRemoteData( ResourceGetDataParams& params ) const
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

    Result ResourceImpl::ImportFromYaml( YAML::Node& resource, const Version& documentVersion )
	{

		if( m_relativePath.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			//TODO handle failure
			m_relativePath = resource[m_relativePath.GetTag()].as<std::string>();
		}

		if( m_location.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			//TODO handle failure
			m_location = resource[m_location.GetTag()].as<std::string>();
		}

		if( m_checksum.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			//TODO handle failure
			m_checksum = resource[m_checksum.GetTag()].as<std::string>();
		}

		if( m_uncompressedSize.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			//TODO handle failure
			m_uncompressedSize = resource[m_uncompressedSize.GetTag()].as<unsigned long>();
		}

		if( m_compressedSize.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			//TODO handle failure
			m_compressedSize = resource[m_compressedSize.GetTag()].as<unsigned long>();
		}

		// TODO this is an example of how this could be managed, nothing yet setup to formally test
		BINARY_GUARD_RETURN( 1, 1, 0 );

		if( m_something.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			//TODO handle failure
			m_something = resource[m_something.GetTag()].as<unsigned long>();
		}

		return Result::SUCCESS;
	}


    Result ResourceImpl::ExportToYaml( YAML::Emitter& out, const Version& documentVersion )
	{
		// Relative path
		if( m_relativePath.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( !m_relativePath.HasValue() )
			{
				return Result::REQUIRED_RESOURCE_PARAMETER_NOT_SET;
			}

			out << YAML::Key << m_relativePath.GetTag();
			out << YAML::Value << m_relativePath.GetValue().ToString();
		}

        // Location
		if( m_location.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( !m_location.HasValue() )
			{
				return Result::REQUIRED_RESOURCE_PARAMETER_NOT_SET;
			}

			out << YAML::Key << m_location.GetTag();
			out << YAML::Value << m_location.GetValue();
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

        // TODO this is an experiment with detecting binary missmatches and handling gracefully, WIP
		BINARY_GUARD_RETURN( 1, 1, 0 );


		if( m_something.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( !m_something.HasValue() )
			{
				return Result::REQUIRED_RESOURCE_PARAMETER_NOT_SET;
			}

			out << YAML::Key << m_something.GetTag();
			out << YAML::Value << m_something.GetValue();
		}

		return Result::SUCCESS;
	}

}