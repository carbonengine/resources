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

class CarbonResources::PatchResourceGroup::PatchResourceGroupImpl : public CarbonResources::PatchResourceGroup::ResourceGroupImpl
{
public:

	PatchResourceGroupImpl( );

    ~PatchResourceGroupImpl();

	Result CreatePatch( const PatchCreateParams& params ) const;

private:

	virtual std::string Type() const override;

    virtual Resource* CreateResourceFromYaml( YAML::Node& resource ) override;

    virtual Result ImportGroupSpecialisedYaml( YAML::Node& resourceGroupFile ) override;

    virtual Result ExportGroupSpecialisedYaml( YAML::Emitter& out, Version outputDocumentVersion ) const override;

    

protected:

    DocumentParameter<std::string> m_resourceGroupPathParameter = DocumentParameter<std::string>( { 0, 1, 0 }, "ResourceGroup" );
};

#endif // PatchResourceGroupImpl_H