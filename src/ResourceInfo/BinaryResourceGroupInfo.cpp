#include "BinaryResourceGroupInfo.h"

#include <sstream>

#include <ResourceTools.h>

namespace CarbonResources
{

    BinaryResourceGroupInfo::BinaryResourceGroupInfo( const BinaryResourceGroupInfoParams& params ) :
      ResourceInfo(params)
    {
		m_type = TypeId();
    }

    BinaryResourceGroupInfo::~BinaryResourceGroupInfo()
    {

    }

    std::string BinaryResourceGroupInfo::TypeId()
    {
		return "BinaryResourceGroup";
    }

}