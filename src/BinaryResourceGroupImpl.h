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
    namespace Internal
    {
#define OPTIONAL_LOAD( structName, fromStruct, member )           \
		if( offsetof( struct structName, member ) < fromStruct.size ) \
		{                                                             \
			member = fromStruct.member;                                                    \
		}

	    struct ThisIsAnExampleTodoRemove
	    {
            ThisIsAnExampleTodoRemove(const CarbonResources::ThisIsAnExampleTodoRemove& apiStruct)
            {
				a = apiStruct.a;

                b = apiStruct.b;

                OPTIONAL_LOAD( ThisIsAnExampleTodoRemove, apiStruct, c );
                
                /*
                if (offsetof(struct ThisIsAnExampleTodoRemove, c) < apiStruct.size)
                {
					c = apiStruct.c;
                }
                */
				
            }

            unsigned int unused=0;

		    int a;

		    int b;

		    int c = 404;
	    };
    }
    
    class BinaryResourceInfo;

    class BinaryResourceGroupImpl : public ResourceGroupImpl
    {
    public:
		BinaryResourceGroupImpl( );

	    ~BinaryResourceGroupImpl();

        void SomethingThatUsesTestStruct( const Internal::ThisIsAnExampleTodoRemove& args );

        virtual std::string GetType() const override;

        static std::string TypeId();

    private:

	    virtual Result CreateResourceFromYaml( YAML::Node& resource, ResourceInfo*& resourceOut ) override;

	    virtual Result ImportGroupSpecialisedYaml( YAML::Node& resourceGroupFile ) override;

	    virtual Result ExportGroupSpecialisedYaml( YAML::Emitter& out, Version outputDocumentVersion ) const override;

	    virtual Result [[deprecated( "Prfer yaml" )]] ImportFromCSV( const std::string& data ) override;
    };

}

#endif // BinaryResourceGroupImpl_H