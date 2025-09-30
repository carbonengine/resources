// Copyright © 2025 CCP ehf.

#pragma once
#ifndef PatchResourceInfo_H
#define PatchResourceInfo_H

#include "ResourceGroupInfo.h"

namespace CarbonResources
{
struct PatchResourceInfoParams : public ResourceGroupInfoParams
{
	std::filesystem::path targetResourceRelativePath = "NOT_SET";

	uintmax_t dataOffset = 0;

	uintmax_t sourceOffset = 0;
};

class ResourceGroup;

class PatchResourceInfo : public ResourceInfo
{
public:
	PatchResourceInfo( const PatchResourceInfoParams& params );

	~PatchResourceInfo();

	static std::string TypeId();

	Result GetTargetResourceRelativePath( std::filesystem::path& targetResourceRelativePath ) const;

	Result GetDataOffset( uintmax_t& dataoffset ) const;

	Result GetSourceOffset( uintmax_t& sourceOffset ) const;

	virtual Result ImportFromYaml( YAML::Node& resource, const VersionInternal& documentVersion ) override;

	virtual Result ExportToYaml( YAML::Emitter& out, const VersionInternal& documentVersion ) override;

	virtual Result SetParametersFromResource( const ResourceInfo* other, const VersionInternal& documentVersion ) override;

private:
	DocumentParameter<uintmax_t> m_dataOffset = DocumentParameter<uintmax_t>( DATA_OFFSET, TypeId() );
	DocumentParameter<uintmax_t> m_sourceOffset = DocumentParameter<uintmax_t>( SOURCE_OFFSET, TypeId() );

	DocumentParameter<std::filesystem::path> m_targetResourceRelativepath = DocumentParameter<std::filesystem::path>( TARGET_RESOURCE_RELATIVE_PATH, TypeId() );
};


}

#endif // PatchResourceInfo_H