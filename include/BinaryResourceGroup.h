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
#include <filesystem>

namespace CarbonResources
{

    struct API ThisIsAnExampleTodoRemove
    {
		const unsigned int size = sizeof( ThisIsAnExampleTodoRemove );

	    int a = 0;

	    int b = 0;

        int c = 0;
    };


    class BinaryResourceGroupImpl;

    class API BinaryResourceGroup final: public ResourceGroup
    {
    public:

	    BinaryResourceGroup( );

	    ~BinaryResourceGroup();

        void SomethingThatUsesTestStruct( const ThisIsAnExampleTodoRemove& args );

    private:

		BinaryResourceGroupImpl* m_impl;

    };

}

#endif // BinaryResourceGroup_H