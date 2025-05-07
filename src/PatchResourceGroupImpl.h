/* 
	*************************************************************************

	PatchResourceGroupImpl.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/
#pragma once
#ifndef PatchResourceGroupImpl_H
#define PatchResourceGroupImpl_H

#include "PatchResourceGroup.h"

#include "ResourceGroupImpl.h"

#include "ResourceInfo/ResourceGroupInfo.h"

#include "ResourceInfo/PatchResourceInfo.h"

namespace CarbonResources
{

    class PatchResourceGroupImpl : public ResourceGroupImpl
    {
    public:

        PatchResourceGroupImpl( );

        ~PatchResourceGroupImpl();

        Result SetResourceGroup( const ResourceGroupInfo& resourceGroup );

        void SetMaxInputChunkSize( uintmax_t maxInputChunkSize );

        Result Apply( const PatchApplyParams& params );

        virtual std::string GetType() const override;

        static std::string TypeId();

    private:

        virtual Result CreateResourceFromYaml( YAML::Node& resource, ResourceInfo*& resourceOut ) override;

        virtual Result ImportGroupSpecialisedYaml( YAML::Node& resourceGroupFile ) override;

        virtual Result ExportGroupSpecialisedYaml( YAML::Emitter& out, VersionInternal outputDocumentVersion ) const override;

        Result GetTargetResourcePatches( const ResourceInfo* targetResource, std::vector<const PatchResourceInfo*>& patches ) const;

    protected:

        DocumentParameter<uintmax_t> m_maxInputChunkSize = DocumentParameter<uintmax_t>( MAX_INPUT_CHUNK_SIZE, TypeId() );

        DocumentParameter<ResourceGroupInfo*> m_resourceGroupParameter = DocumentParameter<ResourceGroupInfo*>( RESOURCE_GROUP_RESOURCE, TypeId() );
    };

}

#endif // PatchResourceGroupImpl_H