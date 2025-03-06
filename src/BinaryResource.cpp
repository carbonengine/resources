#include "BinaryResource.h"

#include <sstream>

#include <ResourceTools.h>

#include <yaml-cpp/yaml.h>

namespace CarbonResources
{

    BinaryResource::BinaryResource( const BinaryResourceParams& params ):
        Resource(params)
    {
		m_binaryOperation = params.binaryOperation;

        m_type = TypeId();
    }

    BinaryResource::~BinaryResource()
    {

    }

    DocumentParameter<unsigned int> BinaryResource::GetBinaryOperation() const
	{
		return m_binaryOperation;
	}

    Result BinaryResource::ImportFromYaml( YAML::Node& resource, const Version& documentVersion )
    {
		if( m_binaryOperation.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			//TODO handle failure
			m_binaryOperation = resource[m_binaryOperation.GetTag()].as<unsigned long>();
		}

		return Resource::ImportFromYaml( resource, documentVersion );
    }

    Result BinaryResource::ExportToYaml( YAML::Emitter& out, const Version& documentVersion )
    {
		Result resourceExportResult = Resource::ExportToYaml( out, documentVersion );

		if( resourceExportResult != Result::SUCCESS )
		{
			return resourceExportResult;
		}

        // Binary Operation
		if( m_binaryOperation.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( !m_binaryOperation.HasValue() )
			{
				return Result::REQUIRED_RESOURCE_PARAMETER_NOT_SET;
			}

			out << YAML::Key << m_binaryOperation.GetTag();
			out << YAML::Value << m_binaryOperation.GetValue();
		}

		return Result::SUCCESS;
    }

    std::string BinaryResource::TypeId( )
    {
		return "Binary";
    }
    

}