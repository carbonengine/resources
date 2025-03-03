#include "Resource.h"

#include "ResourceImpl.h"

#include <yaml-cpp/yaml.h>

namespace CarbonResources
{



    ResourceParams::ResourceParams()
    {
    }

    Result ResourceParams::ImportFromYaml(YAML::Node& resource, const Version& documentVersion)
    {
        
        if (relativePath.IsParameterExpectedInDocumentVersion(documentVersion))
        {
			//TODO handle failure
			relativePath = resource[relativePath.GetTag()].as<std::string>();
        }

        if( location.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			//TODO handle failure
			location = resource[location.GetTag()].as<std::string>();
		}

        if( checksum.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			//TODO handle failure
			checksum = resource[checksum.GetTag()].as<std::string>();
		}

        if( uncompressedSize.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			//TODO handle failure
			uncompressedSize = resource[uncompressedSize.GetTag()].as<unsigned long>();
		}

        if( compressedSize.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			//TODO handle failure
			compressedSize = resource[compressedSize.GetTag()].as<unsigned long>();
		}

        // TODO this is an example of how this could be managed, nothing yet setup to formally test
		BINARY_GUARD_RETURN( 1,1,0 );

	    if( something.IsParameterExpectedInDocumentVersion( documentVersion ) )
	    {
			//TODO handle failure
		    something = resource[something.GetTag()].as<unsigned long>();
	    }

	    return Result::SUCCESS;
    }

    Resource::Resource( const ResourceParams& params ) :
	    m_impl( new ResourceImpl(params) )
    {
    }

    Resource::Resource( ResourceImpl* impl ) :
		m_impl( impl )
	{
	}

    Resource::~Resource()
    {
		delete m_impl;
    }

	Result Resource::ExportToYaml( YAML::Emitter& out, const Version& documentVersion )
    {
		DocumentParameter<std::string> relativePath = GetRelativePath();

        if (relativePath.IsParameterExpectedInDocumentVersion(documentVersion))
        {
            if (!relativePath.HasValue())
            {
				return Result::REQUIRED_RESOURCE_PARAMETER_NOT_SET;
            }

			out << YAML::Key << relativePath.GetTag();
			out << YAML::Value << relativePath.GetValue();
        }

        DocumentParameter<std::string> location = GetLocation();

		if( location.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( !location.HasValue() )
			{
				return Result::REQUIRED_RESOURCE_PARAMETER_NOT_SET;
			}

			out << YAML::Key << location.GetTag();
			out << YAML::Value << location.GetValue();
		}

        DocumentParameter<std::string> checksum = GetChecksum();

		if( checksum.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( !checksum.HasValue() )
			{
				return Result::REQUIRED_RESOURCE_PARAMETER_NOT_SET;
			}

			out << YAML::Key << checksum.GetTag();
			out << YAML::Value << checksum.GetValue();
		}

        DocumentParameter<unsigned long> uncompressedSize = GetUncompressedSize();

		if( uncompressedSize.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( !uncompressedSize.HasValue() )
			{
				return Result::REQUIRED_RESOURCE_PARAMETER_NOT_SET;
			}

			out << YAML::Key << uncompressedSize.GetTag();
			out << YAML::Value << uncompressedSize.GetValue();
		}

        DocumentParameter<unsigned long> compressedSize = GetCompressedSize();

		if( compressedSize.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( !compressedSize.HasValue() )
			{
				return Result::REQUIRED_RESOURCE_PARAMETER_NOT_SET;
			}

			out << YAML::Key << compressedSize.GetTag();
			out << YAML::Value << compressedSize.GetValue();
		}

        BINARY_GUARD_RETURN( 1, 1, 0 );

        DocumentParameter<unsigned long> something = GetSomething();

		if( something.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( !something.HasValue() )
			{
				return Result::REQUIRED_RESOURCE_PARAMETER_NOT_SET;
			}

			out << YAML::Key << something.GetTag();
			out << YAML::Value << something.GetValue();
		}

	    return Result::SUCCESS;
    }

    /// @brief Returns relative path of a resource
    /// @param data data which the checksum will be based on
    /// @param data_size size of data passed in
    /// @param checksum will contain the resulting checksum on success
    /// @return true on success, false on failure
    DocumentParameter<std::string> Resource::GetRelativePath() const
    {
		return m_impl->GetResourceParams().relativePath;
    }

    /// @brief Returns cdn location of a resource
    /// @param data data which the checksum will be based on
    /// @param data_size size of data passed in
    /// @param checksum will contain the resulting checksum on success
    /// @return true on success, false on failure
    DocumentParameter<std::string> Resource::GetLocation() const
    {
		return m_impl->GetResourceParams().location;
    }

    /// @brief Returns data checksum of a resource
    /// @param data data which the checksum will be based on
    /// @param data_size size of data passed in
    /// @param checksum will contain the resulting checksum on success
    /// @return true on success, false on failure
    DocumentParameter<std::string> Resource::GetChecksum() const
    {
		return m_impl->GetResourceParams().checksum;
    }

    /// @brief Returns uncompressed size of resource
    /// @param data data which the checksum will be based on
    /// @param data_size size of data passed in
    /// @param checksum will contain the resulting checksum on success
    /// @return true on success, false on failure
    DocumentParameter<unsigned long> Resource::GetUncompressedSize() const
    {
		return m_impl->GetResourceParams().uncompressedSize;
    }

    /// @brief Returns compressed size of resource
    /// @param data data which the checksum will be based on
    /// @param data_size size of data passed in
    /// @param checksum will contain the resulting checksum on success
    /// @return true on success, false on failure
    DocumentParameter<unsigned long> Resource::GetCompressedSize() const
    {
		return m_impl->GetResourceParams().compressedSize;
    }



    DocumentParameter<unsigned long> Resource::GetSomething() const
    {
		return m_impl->GetResourceParams().something;
    }

    Result Resource::GetData( const ResourceGetDataParams& params ) const
    {
		return m_impl->GetData( params );
    }

}