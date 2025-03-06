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

namespace CarbonResources
{
    class BinaryResourceGroupImpl : public ResourceGroupImpl
    {
    public:
		BinaryResourceGroupImpl( const std::string& relativePath );

	    ~BinaryResourceGroupImpl();

    private:

	    static std::string TypeId();

	    virtual Resource* CreateResourceFromYaml( YAML::Node& resource ) override;

	    virtual Result ImportGroupSpecialisedYaml( YAML::Node& resourceGroupFile ) override;

	    virtual Result ExportGroupSpecialisedYaml( YAML::Emitter& out, Version outputDocumentVersion ) const override;

	    virtual Result [[deprecated( "Prfer yaml" )]] ImportFromCSVFile( ResourceGroupImportFromFileParams& params ) override;
    };

}

#endif // BinaryResourceGroupImpl_H