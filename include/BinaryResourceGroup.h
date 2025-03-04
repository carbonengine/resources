/* 
	*************************************************************************

	BinaryResourceGroup.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/

#pragma once
#ifndef BinaryResourceGroup_H
#define BinaryResourceGroup_H

#include "Exports.h"
#include "ResourceGroup.h"
#include "Enums.h"
#include <memory>
#include <string>

namespace CarbonResources
{
    class BinaryResourceGroupImpl;

    class API BinaryResourceGroup final: public ResourceGroup
    {
    public:

	    BinaryResourceGroup( const std::string& relativePath );

	    ~BinaryResourceGroup();

    private:

		BinaryResourceGroupImpl* m_impl;

    };

}

#endif // BinaryResourceGroup_H