/* 
	*************************************************************************

	BinaryResourceImpl.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/
#pragma once
#ifndef BinaryResourceImpl_H
#define BinaryResourceImpl_H

#include "BinaryResource.h"

#include "ResourceImpl.h"

namespace CarbonResources
{

    class BinaryResourceImpl : public ResourceImpl
    {
    public:
	    BinaryResourceImpl( const BinaryResourceParams& params );

	    ~BinaryResourceImpl();

	    DocumentParameter<unsigned int> GetBinaryOperation() const;

	    virtual Result ImportFromYaml( YAML::Node& resource, const Version& documentVersion ) override;

	    virtual Result ExportToYaml( YAML::Emitter& out, const Version& documentVersion ) override;

    private:
	    DocumentParameter<unsigned int> m_binaryOperation = DocumentParameter<unsigned int>( { 0, 0, 0 }, "BinaryOperation" );
    };

}

#endif // BinaryResourceImpl_H