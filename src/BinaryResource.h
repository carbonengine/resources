/* 
	*************************************************************************

	BinaryResource.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/
#pragma once
#ifndef BinaryResource_H
#define BinaryResource_H

#include "Resource.h"

namespace CarbonResources
{
    struct BinaryResourceParams : public ResourceParams
    {

	    unsigned int binaryOperation = 0;

    };

    class BinaryResource : public Resource
    {
    public:
	    BinaryResource( const BinaryResourceParams& params );

	    ~BinaryResource();

	    DocumentParameter<unsigned int> GetBinaryOperation() const;

	    virtual Result ImportFromYaml( YAML::Node& resource, const Version& documentVersion ) override;

	    virtual Result ExportToYaml( YAML::Emitter& out, const Version& documentVersion ) override;

        static std::string TypeId();

    private:
	    DocumentParameter<unsigned int> m_binaryOperation = DocumentParameter<unsigned int>( { 0, 0, 0 }, "BinaryOperation" );
    };

}

#endif // BinaryResource_H