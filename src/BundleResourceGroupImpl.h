/* 
	*************************************************************************

	BundleResourceGroupImpl.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/
#pragma once
#ifndef BundleResourceGroupImpl_H
#define BundleResourceGroupImpl_H

#include "BundleResourceGroup.h"

#include "ResourceGroupImpl.h"

#include "ResourceInfo/BundleResourceInfo.h"

#include "ResourceInfo/PatchResourceInfo.h"

namespace CarbonResources
{

    class BundleResourceGroupImpl : public ResourceGroupImpl
    {
    public:

	    BundleResourceGroupImpl();

        ~BundleResourceGroupImpl();

        Result SetResourceGroup( const ResourceGroupInfo& resourceGroup );

        Result Unpack( const BundleUnpackParams& params );

        virtual std::string GetType() const override;

        static std::string TypeId();

        void SetChunkSize( unsigned long size );

    private:

        virtual Result CreateResourceFromYaml( YAML::Node& resource, ResourceInfo*& resourceOut ) override;

        virtual Result ImportGroupSpecialisedYaml( YAML::Node& resourceGroupFile ) override;

        virtual Result ExportGroupSpecialisedYaml( YAML::Emitter& out, Version outputDocumentVersion ) const override;

    protected:

        DocumentParameter<unsigned long> m_chunkSize = DocumentParameter<unsigned long>( { 0, 1, 0 }, "ChunkSize" );

		DocumentParameter<ResourceGroupInfo*> m_resourceGroupParameter = DocumentParameter<ResourceGroupInfo*>( { 0, 1, 0 }, "ResourceGroupResource" );

    };

}

#endif // BundleResourceGroupImpl_H