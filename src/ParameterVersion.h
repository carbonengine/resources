// Copyright © 2025 CCP ehf.

#pragma once
#ifndef PARAMETERVERSION_H
#define PARAMETERVERSION_H

#include "VersionInternal.h"

#include <vector>

namespace CarbonResources
{

extern VersionInternal VERSION_MAX;
extern VersionInternal VERSION_0_0_0;
extern VersionInternal VERSION_0_1_0;
extern VersionInternal VERSION_1_0_0;


// Uniquely identifies a parameter. In the case that a parameter gets added in one version and removed in a later version, then added again for some other purpose.
enum Parameter
{
	CHUNK_SIZE,
	RESOURCE_GROUP_RESOURCE,
	MAX_INPUT_CHUNK_SIZE,
	VERSION,
	TYPE,
	NUMBER_OF_RESOURCES,
	TOTAL_RESOURCE_SIZE_COMPRESSED,
	TOTAL_RESOURCE_SIZE_UNCOMPRESSED,
	RESOURCE,
	DATA_OFFSET,
	SOURCE_OFFSET,
	TARGET_RESOURCE_RELATIVE_PATH,
	RELATIVE_PATH,
	LOCATION,
	CHECKSUM,
	COMPRESSED_SIZE,
	UNCOMPRESSED_SIZE,
	BINARY_OPERATION,
	PREFIX,
	REMOVED_RESOURCE_RELATIVE_PATHS
};

class ParameterContext
{
public:
	ParameterContext( std::string context, VersionInternal introducedInVersion, VersionInternal deprecatedInVersion );
	std::string m_context;
	VersionInternal m_introducedInVersion;
	VersionInternal m_deprecatedInVersion;
};

class ParameterInfo
{
public:
	ParameterInfo( CarbonResources::Parameter id, std::string tag, std::vector<ParameterContext> context, bool isOptional = false );
	Parameter m_id;
	std::string m_tag;
	std::vector<ParameterContext> m_context;
	bool m_isOptional;
};

const ParameterInfo* GetParameterInfo( Parameter param );
bool IsParameterExpected( Parameter parameter, const std::string& context, VersionInternal version );
bool IsParameterRequired( Parameter parameter, const std::string& context, VersionInternal version );

}
#endif //PARAMETERVERSION_H
