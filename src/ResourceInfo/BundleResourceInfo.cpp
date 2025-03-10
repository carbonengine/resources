#include "BundleResourceInfo.h"

#include <sstream>

#include <ResourceTools.h>

namespace CarbonResources
{

    BundleResourceInfo::BundleResourceInfo( const BundleResourceInfoParams& params ):
      ResourceInfo(params)
    {
		m_type = TypeId();
    }

    BundleResourceInfo::~BundleResourceInfo()
    {

    }

    std::string BundleResourceInfo::TypeId( )
    {
		return "BinaryChunk";
    }

}