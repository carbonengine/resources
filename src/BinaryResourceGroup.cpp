#include "BinaryResourceGroup.h"
#include "BinaryResourceGroupImpl.h"

namespace CarbonResources
{

    BinaryResourceGroup::BinaryResourceGroup( const std::string& relativePath ) :
	    ResourceGroup( new BinaryResourceGroupImpl( relativePath ) ),
	    m_impl(reinterpret_cast<BinaryResourceGroupImpl*>(ResourceGroup::m_impl))
    {

    }

    BinaryResourceGroup::~BinaryResourceGroup()
    {

    }


}