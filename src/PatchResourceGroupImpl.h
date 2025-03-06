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

namespace CarbonResources
{

    class PatchResourceGroupImpl : public ResourceGroupImpl
    {
    public:

        PatchResourceGroupImpl( const std::filesystem::path& relativePath );

        Result SetResourceGroup( ResourceGroupImpl* resourceGroup);

        ~PatchResourceGroupImpl();

    private:

	    static std::string TypeId();

        virtual Resource* CreateResourceFromYaml( YAML::Node& resource ) override;

        virtual Result ImportGroupSpecialisedYaml( YAML::Node& resourceGroupFile ) override;

        virtual Result ExportGroupSpecialisedYaml( YAML::Emitter& out, Version outputDocumentVersion ) const override;

    protected:

        DocumentParameter<Resource*> m_resourceGroupParameter = DocumentParameter<Resource*>( { 0, 1, 0 }, "ResourceGroupResource" );
    };

}

#endif // PatchResourceGroupImpl_H