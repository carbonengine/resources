/* 
	*************************************************************************

	BinaryResourceGroupImpl.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/
#pragma once
#ifndef BinaryResourceGroupImpl_H
#define BinaryResourceGroupImpl_H

#include "BinaryResourceGroup.h"

#include "ResourceGroupImpl.h"

#include "ResourceInfo/BinaryResourceInfo.h"

#include <optional>

#include <iostream>

namespace CarbonResources
{
    
    class BinaryResourceInfo;

    class BinaryResourceGroupImpl : public ResourceGroupImpl
    {
    public:
		BinaryResourceGroupImpl( );

	    ~BinaryResourceGroupImpl();

        virtual std::string GetType() const override;

        static std::string TypeId();

    private:

	    virtual Result CreateResourceFromYaml( YAML::Node& resource, ResourceInfo*& resourceOut ) override;

	    virtual Result ImportGroupSpecialisedYaml( YAML::Node& resourceGroupFile ) override;

	    virtual Result ExportGroupSpecialisedYaml( YAML::Emitter& out, VersionInternal outputDocumentVersion ) const override;

	    virtual Result [[deprecated( "Prfer yaml" )]] ImportFromCSV( const std::string& data ) override;
    };

}

#endif // BinaryResourceGroupImpl_H