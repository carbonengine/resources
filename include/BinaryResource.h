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

#include <string>

#include "Exports.h"
#include "Resource.h"

namespace YAML
{
    class Emitter;
    class Node;
}

namespace CarbonResources
{

    class API BinaryResourceParams : public ResourceParams
    {
	public:
        BinaryResourceParams();

        virtual Result ImportFromYaml( YAML::Node& resource, const Version& documentVersion ) override; // TODO out of public API

		DocumentParameter<unsigned int> binaryOperation = DocumentParameter<unsigned int>( { 0, 0, 0 }, "BinaryOperation" );

    };

    class API BinaryResource final : public Resource
    {

    private:
	    class BinaryResourceImpl;

    public:
        BinaryResource( const BinaryResourceParams& params );

	    ~BinaryResource();

        virtual Result ExportToYaml( YAML::Emitter& out, const Version& documentVersion ) override; //TODO out of API

        DocumentParameter<unsigned int> GetBinaryOperation() const;

    private:
		BinaryResourceImpl* m_impl;

    };

}

#endif // BinaryResource_H