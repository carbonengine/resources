#include "BinaryResourceGroup.h"
#include "BinaryResourceGroupImpl.h"

#include <iostream>

namespace CarbonResources
{

    BinaryResourceGroup::BinaryResourceGroup( ) :
	    ResourceGroup( new BinaryResourceGroupImpl( ) ),
	    m_impl(reinterpret_cast<BinaryResourceGroupImpl*>(ResourceGroup::m_impl))
    {

    }

    BinaryResourceGroup::~BinaryResourceGroup()
    {

    }

    void BinaryResourceGroup::SomethingThatUsesTestStruct( const ThisIsAnExampleTodoRemove& args )
    {
		/*
		Internal::ThisIsAnExampleTodoRemove abiSafeStruct;

        abiSafeStruct.a = args.a;

        abiSafeStruct.b = args.b;

        std::cout << args.revision << std::endl;
        // C was added in a later version so skip it
		if( args.revision >= 2 )
        {
			std::cout << "Set argument C" << std::endl;
			abiSafeStruct.c = args.c;
        }
        */
		std::cout << args.size << std::endl;
		Internal::ThisIsAnExampleTodoRemove internalArgs = Internal::ThisIsAnExampleTodoRemove( args );

		m_impl->SomethingThatUsesTestStruct( internalArgs );
    }

}