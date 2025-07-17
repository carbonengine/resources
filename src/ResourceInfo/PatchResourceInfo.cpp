// Copyright © 2025 CCP ehf.

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

    	m_sourceOffset = params.sourceOffset;

		m_type = TypeId();
    }

    PatchResourceInfo::~PatchResourceInfo()
    {

    }

    Result PatchResourceInfo::GetTargetResourceRelativePath( std::filesystem::path& targetResourceRelativePath ) const
	{
		if( !m_targetResourceRelativepath.HasValue() )
		{
			return Result{ ResultType::RESOURCE_VALUE_NOT_SET };
		}
		else
		{
			targetResourceRelativePath = m_targetResourceRelativepath.GetValue();

			return Result{ ResultType::SUCCESS };
		}
	}

    Result PatchResourceInfo::GetDataOffset( uintmax_t& dataoffset ) const
    {
		if( !m_dataOffset.HasValue() )
		{
			return Result{ ResultType::RESOURCE_VALUE_NOT_SET };
		}
		else
		{
			dataoffset = m_dataOffset.GetValue();

			return Result{ ResultType::SUCCESS };
		}
    }

	Result PatchResourceInfo::GetSourceOffset( uintmax_t& sourceOffset ) const
	{
		if( !m_sourceOffset.HasValue() )
		{
			return Result{ ResultType::RESOURCE_VALUE_NOT_SET };
		}
		else
		{
			sourceOffset = m_sourceOffset.GetValue();
			return Result{ ResultType::SUCCESS };
		}
	}

	Result PatchResourceInfo::ImportFromYaml( YAML::Node& resource, const VersionInternal& documentVersion )
	{
    	Result result = SetParameterFromYamlNode( resource, m_targetResourceRelativepath, TypeId(), documentVersion );
    	if( result.type != ResultType::SUCCESS )
    	{
    		return result;
    	}

    	result = SetParameterFromYamlNode( resource, m_dataOffset, TypeId(), documentVersion );
		if( result.type != ResultType::SUCCESS )
    	{
    		return result;
    	}

    	result = SetParameterFromYamlNode( resource, m_sourceOffset, TypeId(), documentVersion );
    	if( result.type != ResultType::SUCCESS )
    	{
    		return result;
    	}

		return ResourceInfo::ImportFromYaml( resource, documentVersion );
	}

	Result PatchResourceInfo::ExportToYaml( YAML::Emitter& out, const VersionInternal& documentVersion )
	{
		Result resourceExportResult = ResourceInfo::ExportToYaml( out, documentVersion );

		if( resourceExportResult.type != ResultType::SUCCESS )
		{
			return resourceExportResult;
		}

		// Resource relative path
		if( m_targetResourceRelativepath.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( !m_targetResourceRelativepath.HasValue() )
			{
				return Result{ ResultType::REQUIRED_RESOURCE_PARAMETER_NOT_SET };
			}

			out << YAML::Key << m_targetResourceRelativepath.GetTag();
			out << YAML::Value << m_targetResourceRelativepath.GetValue().string();
		}

        // Data offset
        if( m_dataOffset.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			if( !m_dataOffset.HasValue() )
			{
				return Result{ ResultType::REQUIRED_RESOURCE_PARAMETER_NOT_SET };
			}

			out << YAML::Key << m_dataOffset.GetTag();
			out << YAML::Value << m_dataOffset.GetValue();
		}

    	// Source offset
    	if( m_sourceOffset.IsParameterExpectedInDocumentVersion( documentVersion ) )
    	{
    		if( !m_sourceOffset.HasValue() )
    		{
				return Result{ ResultType::REQUIRED_RESOURCE_PARAMETER_NOT_SET };
    		}

    		out << YAML::Key << m_sourceOffset.GetTag();
    		out << YAML::Value << m_sourceOffset.GetValue();
    	}

		return Result{ ResultType::SUCCESS };
	}

    std::string PatchResourceInfo::TypeId( )
    {
		return "BinaryPatch";
    }

    Result PatchResourceInfo::SetParametersFromResource( const ResourceInfo* other, const VersionInternal& documentVersion )
	{
		if( other == nullptr )
        {
			return Result{ ResultType::FAIL };
        }

        if (other->TypeId() != TypeId())
        {
			return Result{ ResultType::RESOURCE_TYPE_MISSMATCH };
        }

        const PatchResourceInfo* otherAsPatch = reinterpret_cast<const PatchResourceInfo*>( other );

        if (m_dataOffset.IsParameterExpectedInDocumentVersion(documentVersion))
        {
			uintmax_t dataOffset;

			Result getOffsetResult = otherAsPatch->GetDataOffset( dataOffset );

			if( getOffsetResult.type != ResultType::SUCCESS )
			{
				return getOffsetResult;
			}

			m_dataOffset = dataOffset;
        }
		
        if (m_targetResourceRelativepath.IsParameterExpectedInDocumentVersion(documentVersion))
        {
			std::filesystem::path targetResourceRelativePath;

			Result getTargetResourceRelativePathResult = otherAsPatch->GetTargetResourceRelativePath( targetResourceRelativePath );

			if( getTargetResourceRelativePathResult.type != ResultType::SUCCESS )
			{
				return getTargetResourceRelativePathResult;
			}

			m_targetResourceRelativepath = targetResourceRelativePath;
        }

		return ResourceInfo::SetParametersFromResource( other, documentVersion );
	}


}