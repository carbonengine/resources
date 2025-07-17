// Copyright © 2025 CCP ehf.

#pragma once
#ifndef PatchResourceGroupImpl_H
#define PatchResourceGroupImpl_H

#include "PatchResourceGroup.h"

#include "ResourceGroupImpl.h"

#include "ResourceInfo/ResourceGroupInfo.h"

#include "ResourceInfo/PatchResourceInfo.h"

namespace CarbonResources
{

    class PatchResourceGroup::PatchResourceGroupImpl : public ResourceGroup::ResourceGroupImpl
    {
    public:

        PatchResourceGroupImpl( );

        ~PatchResourceGroupImpl();

        Result SetResourceGroup( const ResourceGroupInfo& resourceGroup );

        Result SetMaxInputChunkSize( uintmax_t maxInputChunkSize );

        Result Apply( const PatchApplyParams& params );

        virtual std::string GetType() const override;

        static std::string TypeId();

    	Result SetRemovedResourceRelativePaths( const std::vector<std::filesystem::path>& paths );

		virtual Result GetGroupSpecificResourcesToBundle(std::vector<ResourceInfo*>& toBundle) const final;

    private:

        virtual Result CreateResourceFromYaml( YAML::Node& resource, ResourceInfo*& resourceOut ) override;

        virtual Result ImportGroupSpecialisedYaml( YAML::Node& resourceGroupFile ) override;

        virtual Result ExportGroupSpecialisedYaml( YAML::Emitter& out, VersionInternal outputDocumentVersion ) const override;

        Result GetTargetResourcePatches( const ResourceInfo* targetResource, std::vector<const PatchResourceInfo*>& patches ) const;

    protected:

        DocumentParameter<uintmax_t> m_maxInputChunkSize = DocumentParameter<uintmax_t>( MAX_INPUT_CHUNK_SIZE, TypeId() );

        DocumentParameter<ResourceGroupInfo*> m_resourceGroupParameter = DocumentParameter<ResourceGroupInfo*>( RESOURCE_GROUP_RESOURCE, TypeId() );

    	DocumentParameterCollection<std::filesystem::path> m_removedResources = DocumentParameterCollection<std::filesystem::path>( REMOVED_RESOURCE_RELATIVE_PATHS, TypeId() );
    };

}

#endif // PatchResourceGroupImpl_H