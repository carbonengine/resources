#include "BinaryResourceGroup.h"
#include "BinaryResourceGroupImpl.h"

namespace CarbonResources
{

    BinaryResourceGroup::BinaryResourceGroup():
	    ResourceGroup( new BinaryResourceGroupImpl() ),
	    m_impl(reinterpret_cast<BinaryResourceGroupImpl*>(ResourceGroup::m_impl))
    {

    }

    BinaryResourceGroup::~BinaryResourceGroup()
    {

    }


}