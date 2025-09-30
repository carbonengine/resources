// Copyright © 2025 CCP ehf.

#pragma once
#ifndef BundleResourceGroupImpl_H
#define BundleResourceGroupImpl_H

#include "BundleResourceGroup.h"

#include "ParameterVersion.h"

#include "ResourceGroupImpl.h"

#include "ResourceInfo/BundleResourceInfo.h"

#include "ResourceInfo/PatchResourceInfo.h"

namespace CarbonResources
{

class BundleResourceGroup::BundleResourceGroupImpl : public ResourceGroup::ResourceGroupImpl
{
public:
	BundleResourceGroupImpl();

	~BundleResourceGroupImpl();

	Result SetResourceGroup( const ResourceGroupInfo& resourceGroup );

	Result Unpack( const BundleUnpackParams& params );

	virtual std::string GetType() const override;

	static std::string TypeId();

	Result SetChunkSize( uintmax_t size );

private:
	virtual Result CreateResourceFromYaml( YAML::Node& resource, ResourceInfo*& resourceOut ) override;

	virtual Result ImportGroupSpecialisedYaml( YAML::Node& resourceGroupFile ) override;

	virtual Result ExportGroupSpecialisedYaml( YAML::Emitter& out, VersionInternal outputDocumentVersion ) const override;

protected:
	DocumentParameter<uintmax_t> m_chunkSize = DocumentParameter<uintmax_t>( CHUNK_SIZE, TypeId() );

	DocumentParameter<ResourceGroupInfo*> m_resourceGroupParameter = DocumentParameter<ResourceGroupInfo*>( RESOURCE_GROUP_RESOURCE, TypeId() );
};

}

#endif // BundleResourceGroupImpl_H