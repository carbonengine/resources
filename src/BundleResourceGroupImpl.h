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

namespace CarbonResources
{

    class BundleResourceGroupImpl : public ResourceGroupImpl
    {
    public:

	    BundleResourceGroupImpl( );

        ~BundleResourceGroupImpl();

        virtual std::string GetType() const override;

        static std::string TypeId();

    private:

        virtual Result CreateResourceFromYaml( YAML::Node& resource, ResourceInfo*& resourceOut ) override;

        virtual Result ImportGroupSpecialisedYaml( YAML::Node& resourceGroupFile ) override;

        virtual Result ExportGroupSpecialisedYaml( YAML::Emitter& out, Version outputDocumentVersion ) const override;

    };

}

#endif // BundleResourceGroupImpl_H