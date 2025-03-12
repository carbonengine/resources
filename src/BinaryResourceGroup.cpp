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
		m_impl->SomethingThatUsesTestStruct( Internal::ThisIsAnExampleTodoRemove( args ) );
    }

}