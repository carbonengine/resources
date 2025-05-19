#include "ParameterVersion.h"

#include <map>

namespace CarbonResources
{
VersionInternal VERSION_0_0_0{ 0, 0, 0 };
VersionInternal VERSION_0_1_0{ 0, 1, 0 };
VersionInternal VERSION_1_0_0{ 1, 0, 0 };
VersionInternal VERSION_MAX{ std::numeric_limits<unsigned int>::max(), std::numeric_limits<unsigned int>::max(), std::numeric_limits<unsigned int>::max() };

// These contexts specify in which constructs we should expect to find the parameter
const std::string CONTEXT_RESOURCE_GROUP( "ResourceGroup" );
const std::string CONTEXT_BUNDLE_GROUP( "BundleGroup" );
const std::string CONTEXT_PATCH_GROUP( "PatchGroup" );
const std::string CONTEXT_BINARY_PATCH( "BinaryPatch" );
const std::string CONTEXT_RESOURCE( "Resource" );

std::map<Parameter, ParameterInfo*> s_paramToInfo;

ParameterInfo PARAMETER_CHUNK_SIZE( CHUNK_SIZE, "ChunkSize", { { CONTEXT_BUNDLE_GROUP, VERSION_0_1_0, VERSION_MAX } } );
ParameterInfo PARAMETER_RESOURCE_GROUP_RESOURCE( Parameter::RESOURCE_GROUP_RESOURCE, "ResourceGroupResource", { { CONTEXT_BUNDLE_GROUP, VERSION_0_1_0, VERSION_MAX }, { CONTEXT_PATCH_GROUP, VERSION_0_1_0, VERSION_MAX } } );
ParameterInfo PARAMETER_MAX_INPUT_CHUNK_SIZE( Parameter::MAX_INPUT_CHUNK_SIZE, "MaxInputChunkSize", { { CONTEXT_PATCH_GROUP, VERSION_0_1_0, VERSION_MAX } } );
ParameterInfo PARAMETER_VERSION( Parameter::VERSION, "Version", { { CONTEXT_RESOURCE_GROUP, VERSION_0_1_0, VERSION_MAX } } );
ParameterInfo PARAMETER_TYPE( Parameter::TYPE, "Type", { { CONTEXT_RESOURCE_GROUP, VERSION_0_1_0, VERSION_MAX }, { CONTEXT_RESOURCE, VERSION_0_1_0, VERSION_MAX } } );
ParameterInfo PARAMETER_NUMBER_OF_RESOURCES( Parameter::NUMBER_OF_RESOURCES, "NumberOfResources", { { CONTEXT_RESOURCE_GROUP, VERSION_0_1_0, VERSION_MAX } } );
ParameterInfo PARAMETER_TOTAL_RESOURCE_SIZE_COMPRESSED( Parameter::TOTAL_RESOURCE_SIZE_COMPRESSED, "TotalResourcesSizeCompressed", { { CONTEXT_RESOURCE_GROUP, VERSION_0_1_0, VERSION_MAX } } );
ParameterInfo PARAMETER_TOTAL_RESOURCE_SIZE_UNCOMPRESSED( Parameter::TOTAL_RESOURCE_SIZE_UNCOMPRESSED, "TotalResourcesSizeUnCompressed", { { CONTEXT_RESOURCE_GROUP, VERSION_0_1_0, VERSION_MAX } } );
ParameterInfo PARAMETER_RESOURCES( Parameter::RESOURCE, "Resources", { { CONTEXT_RESOURCE_GROUP, VERSION_0_0_0, VERSION_MAX } } );
ParameterInfo PARAMETER_DATA_OFFSET( Parameter::DATA_OFFSET, "DataOffset", { { CONTEXT_BINARY_PATCH, VERSION_0_0_0, VERSION_MAX } } );
ParameterInfo PARAMETER_SOURCE_OFFSET( Parameter::SOURCE_OFFSET, "SourceOffset", { { CONTEXT_BINARY_PATCH, VERSION_0_0_0, VERSION_MAX } } );
ParameterInfo PARAMETER_TARGET_RESOURCE_RELATIVE_PATH( Parameter::TARGET_RESOURCE_RELATIVE_PATH, "TargetResourceRelativePath", { { CONTEXT_BINARY_PATCH, VERSION_0_0_0, VERSION_MAX } } );
ParameterInfo PARAMETER_RELATIVE_PATH( Parameter::RELATIVE_PATH, "RelativePath", { { CONTEXT_RESOURCE, VERSION_0_0_0, VERSION_MAX } } );
ParameterInfo PARAMETER_LOCATION( Parameter::LOCATION, "Location", { { CONTEXT_RESOURCE, VERSION_0_0_0, VERSION_MAX } } );
ParameterInfo PARAMETER_CHECKSUM( Parameter::CHECKSUM, "Checksum", { { CONTEXT_RESOURCE, VERSION_0_0_0, VERSION_MAX } } );
ParameterInfo PARAMETER_COMPRESSED_SIZE( Parameter::COMPRESSED_SIZE, "CompressedSize", { { CONTEXT_RESOURCE, VERSION_0_0_0, VERSION_MAX } } );
ParameterInfo PARAMETER_UNCOMPRESSED_SIZE( Parameter::UNCOMPRESSED_SIZE, "UncompressedSize", { { CONTEXT_RESOURCE, VERSION_0_0_0, VERSION_MAX } } );
ParameterInfo PARAMETER_BINARY_OPERATION( Parameter::BINARY_OPERATION, "BinaryOperation", { { CONTEXT_RESOURCE, VERSION_0_0_0, VERSION_MAX } }, true );
ParameterInfo PARAMETER_PREFIX( Parameter::PREFIX, "Prefix", { { CONTEXT_RESOURCE, VERSION_0_0_0, VERSION_MAX } }, true );

ParameterInfo::ParameterInfo( CarbonResources::Parameter id, std::string tag, std::vector<ParameterContext> context, bool isOptional ) :
	m_id( id ),
	m_tag( tag ),
	m_context( context ),
	m_isOptional( isOptional )
{
	s_paramToInfo[id] = this;
}
const ParameterInfo* GetParameterInfo( Parameter param )
{
	return s_paramToInfo[param];
}

bool IsParameterExpected( Parameter parameter, const std::string& context, VersionInternal version )
{
	const ParameterInfo* info = GetParameterInfo( parameter );
	if( !info )
	{
		return false;
	}
	for( auto entry : info->m_context )
	{
		if( entry.m_context == context && entry.m_introducedInVersion <= version && entry.m_deprecatedInVersion > version )
		{
			return true;
		}
	}
	return false;
}

bool IsParameterRequired( Parameter parameter, const std::string& context, VersionInternal version )
{
	const ParameterInfo* info = GetParameterInfo( parameter );
	if( !info )
	{
		return false;
	}
	if( IsParameterExpected( parameter, context, version ) )
	{
		return !info->m_isOptional;
	}

	// If the parameter was added in a newer minor version, then treat is as optional,
	// since minor version bumps should signal non-breaking changes.
	unsigned int maxMinorVersion{ 0 };
	for( auto context : info->m_context )
	{
		if( version.getMajor() == context.m_introducedInVersion.getMajor() )
		{
			if( context.m_introducedInVersion.getMinor() > maxMinorVersion )
			{
				maxMinorVersion = context.m_introducedInVersion.getMinor();
			}
		}
	}
	return maxMinorVersion <= version.getMinor();
}

ParameterContext::ParameterContext( std::string context, VersionInternal introducedInVersion, VersionInternal deprecatedInVersion ) :
	m_context( context ),
	m_introducedInVersion( introducedInVersion ),
	m_deprecatedInVersion( deprecatedInVersion )
{
}
}