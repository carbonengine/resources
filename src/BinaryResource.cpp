#include "BinaryResource.h"

#include "BinaryResourceImpl.h"

#include <yaml-cpp/yaml.h>

namespace CarbonResources
{
    BinaryResourceParams::BinaryResourceParams()
    {
    }

    Result BinaryResourceParams::ImportFromYaml( YAML::Node& resource, const Version& documentVersion )
	{
		if( binaryOperation.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			//TODO handle failure
			binaryOperation = resource[binaryOperation.GetTag()].as<unsigned long>();
		}

		return ResourceParams::ImportFromYaml( resource, documentVersion);
	}

    BinaryResource::BinaryResource( const BinaryResourceParams& params ) :
		Resource( new BinaryResourceImpl(params) ),
		m_impl( reinterpret_cast<BinaryResourceImpl*>( Resource::m_impl ) )
    {

    }

    BinaryResource::~BinaryResource( )
    {
		
    }

    Result BinaryResource::ExportToYaml( YAML::Emitter& out, const Version& documentVersion )
	{
		Result resourceExportResult = Resource::ExportToYaml( out, documentVersion );

        if( resourceExportResult != Result::SUCCESS )
        {
			return resourceExportResult;
        }

        DocumentParameter<unsigned int> binaryOperation = GetBinaryOperation();

		if( binaryOperation.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( !binaryOperation.HasValue() )
			{
				return Result::REQUIRED_RESOURCE_PARAMETER_NOT_SET;
			}

			out << YAML::Key << binaryOperation.GetTag();
			out << YAML::Value << binaryOperation.GetValue();
		}

        return Result::SUCCESS;

	}

    /// @brief Returns binary operation a resource
	/// @param data data which the checksum will be based on
	/// @param data_size size of data passed in
	/// @param checksum will contain the resulting checksum on success
	/// @return true on success, false on failure
	DocumentParameter<unsigned int> BinaryResource::GetBinaryOperation() const
	{
		return m_impl->GetBinaryOperation();
	}
	
}