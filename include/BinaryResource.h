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

    struct API BinaryResourceParams : public ResourceParams
    {

        unsigned int binaryOperation = 0;

    };

    class BinaryResourceImpl;
	class BinaryResourceGroupImpl;

    class API BinaryResource final : public Resource
    {
    public:
        BinaryResource( const BinaryResourceParams& params );

	    ~BinaryResource();

        unsigned int GetBinaryOperation() const;

    private:
		BinaryResourceImpl* m_impl;

        friend class BinaryResourceGroupImpl;
    };

}

#endif // BinaryResource_H