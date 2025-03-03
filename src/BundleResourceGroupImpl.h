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

class CarbonResources::BundleResourceGroup::BundleResourceGroupImpl : public CarbonResources::BundleResourceGroup::ResourceGroupImpl
{
public:

	BundleResourceGroupImpl( );

    ~BundleResourceGroupImpl();

private:

    virtual std::string Type() const override;

    virtual Resource* CreateResourceFromYaml( YAML::Node& resource ) override;

    virtual Result ImportGroupSpecialisedYaml( YAML::Node& resourceGroupFile ) override;

    virtual Result ExportGroupSpecialisedYaml( YAML::Emitter& out, Version outputDocumentVersion ) const override;

};

#endif // BundleResourceGroupImpl_H