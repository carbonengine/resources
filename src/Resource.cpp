#include "Resource.h"

#include "ResourceImpl.h"

namespace CarbonResources
{

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

	Result Resource::Export( const Version& documentVersion )
    {
		/*
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
        */

	    return Result::SUCCESS;

    }

    /// @brief Returns relative path of a resource
    /// @param data data which the checksum will be based on
    /// @param data_size size of data passed in
    /// @param checksum will contain the resulting checksum on success
    /// @return true on success, false on failure
	std::string Resource::GetRelativePath() const
    {
		return m_impl->GetRelativePath().GetValue().ToString();
    }

    /// @brief Returns cdn location of a resource
    /// @param data data which the checksum will be based on
    /// @param data_size size of data passed in
    /// @param checksum will contain the resulting checksum on success
    /// @return true on success, false on failure
    std::string Resource::GetLocation() const
    {
		return m_impl->GetLocation().GetValue();
    }

    /// @brief Returns data checksum of a resource
    /// @param data data which the checksum will be based on
    /// @param data_size size of data passed in
    /// @param checksum will contain the resulting checksum on success
    /// @return true on success, false on failure
    std::string Resource::GetChecksum() const
    {
		return m_impl->GetChecksum().GetValue();
    }

    /// @brief Returns uncompressed size of resource
    /// @param data data which the checksum will be based on
    /// @param data_size size of data passed in
    /// @param checksum will contain the resulting checksum on success
    /// @return true on success, false on failure
    unsigned long Resource::GetUncompressedSize() const
    {
		return m_impl->GetUncompressedSize().GetValue();
    }

    /// @brief Returns compressed size of resource
    /// @param data data which the checksum will be based on
    /// @param data_size size of data passed in
    /// @param checksum will contain the resulting checksum on success
    /// @return true on success, false on failure
    unsigned long Resource::GetCompressedSize() const
    {
		return m_impl->GetCompressedSize().GetValue();
    }


    // TODO remove this something tag, it is just a thought exercise
    unsigned long Resource::GetSomething() const
    {
		return m_impl->GetSomething().GetValue();
    }

    
    Result Resource::GetData( ResourceGetDataParams& params ) const
    {
		return m_impl->GetData( params );
    }
    

}