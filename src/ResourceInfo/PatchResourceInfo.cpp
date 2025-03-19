#include "PatchResourceInfo.h"

#include <sstream>

#include <ResourceTools.h>

#include <ResourceGroup.h>

#include <yaml-cpp/yaml.h>

namespace CarbonResources
{

    PatchResourceInfo::PatchResourceInfo( const PatchResourceInfoParams& params ):
      ResourceInfo(params)
    {
		m_targetResourceRelativepath = params.targetResourceRelativePath;

        m_dataOffset = params.dataOffset;

		m_type = TypeId();
    }

    PatchResourceInfo::~PatchResourceInfo()
    {

    }

    Result PatchResourceInfo::GetTargetResourceRelativePath( std::filesystem::path& targetResourceRelativePath ) const
	{
		if( !m_targetResourceRelativepath.HasValue() )
		{
			return Result::RESOURCE_VALUE_NOT_SET;
		}
		else
		{
			targetResourceRelativePath = m_targetResourceRelativepath.GetValue();

			return Result::SUCCESS;
		}
	}

    Result PatchResourceInfo::GetDataOffset( unsigned long& dataoffset ) const
    {
		if( !m_dataOffset.HasValue() )
		{
			return Result::RESOURCE_VALUE_NOT_SET;
		}
		else
		{
			dataoffset = m_dataOffset.GetValue();

			return Result::SUCCESS;
		}
    }

	Result PatchResourceInfo::ImportFromYaml( YAML::Node& resource, const Version& documentVersion )
	{
		if( m_targetResourceRelativepath.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( YAML::Node parameter = resource[m_targetResourceRelativepath.GetTag()] )
			{
				m_targetResourceRelativepath = parameter.as<std::string>();
			}
			else
			{
				return Result::MALFORMED_RESOURCE_INPUT;
			}
		}

        if( m_dataOffset.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( YAML::Node parameter = resource[m_dataOffset.GetTag()] )
			{
				m_dataOffset = parameter.as<unsigned long>();
			}
			else
			{
				return Result::MALFORMED_RESOURCE_INPUT;
			}
		}

		return ResourceInfo::ImportFromYaml( resource, documentVersion );
	}

	Result PatchResourceInfo::ExportToYaml( YAML::Emitter& out, const Version& documentVersion )
	{
		Result resourceExportResult = ResourceInfo::ExportToYaml( out, documentVersion );

		if( resourceExportResult != Result::SUCCESS )
		{
			return resourceExportResult;
		}

		// Resource relative path
		if( m_targetResourceRelativepath.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( !m_targetResourceRelativepath.HasValue() )
			{
				return Result::REQUIRED_RESOURCE_PARAMETER_NOT_SET;
			}

			out << YAML::Key << m_targetResourceRelativepath.GetTag();
			out << YAML::Value << m_targetResourceRelativepath.GetValue().string();
		}

        // Data offset
        if( m_dataOffset.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( !m_dataOffset.HasValue() )
			{
				return Result::REQUIRED_RESOURCE_PARAMETER_NOT_SET;
			}

			out << YAML::Key << m_dataOffset.GetTag();
			out << YAML::Value << m_dataOffset.GetValue();
		}

		return Result::SUCCESS;
	}

    std::string PatchResourceInfo::TypeId( )
    {
		return "BinaryPatch";
    }


}