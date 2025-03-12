#include "BinaryResourceInfo.h"

#include <sstream>

#include <ResourceTools.h>

#include <yaml-cpp/yaml.h>

namespace CarbonResources
{

    BinaryResourceInfo::BinaryResourceInfo( const BinaryResourceInfoParams& params ) :
        ResourceInfo(params)
    {
		m_binaryOperation = params.binaryOperation;

        m_type = TypeId();
    }

    BinaryResourceInfo::~BinaryResourceInfo()
    {

    }

    Result BinaryResourceInfo::GetBinaryOperation(unsigned int& binaryOperation) const
	{
		if( !m_binaryOperation.HasValue() )
		{
			return Result::RESOURCE_VALUE_NOT_SET;
		}
		else
		{
			binaryOperation = m_binaryOperation.GetValue();

			return Result::SUCCESS;
		}
	}

    Result BinaryResourceInfo::ImportFromYaml( YAML::Node& resource, const Version& documentVersion )
    {
		if( m_binaryOperation.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( YAML::Node parameter = resource[m_binaryOperation.GetTag()] )
			{
				m_binaryOperation = parameter.as<unsigned long>();
			}
			else
			{
				return Result::MALFORMED_RESOURCE_INPUT;
			}
		}

		return ResourceInfo::ImportFromYaml( resource, documentVersion );
    }

    Result BinaryResourceInfo::ExportToYaml( YAML::Emitter& out, const Version& documentVersion )
    {
		Result resourceExportResult = ResourceInfo::ExportToYaml( out, documentVersion );

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

    std::string BinaryResourceInfo::TypeId()
    {
		return "Binary";
    }
    

}