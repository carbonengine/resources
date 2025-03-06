#include "BundleResource.h"

#include <sstream>

#include <ResourceTools.h>

namespace CarbonResources
{

    BundleResource::BundleResource( const BundleResourceParams& params ):
      Resource(params)
    {
		m_type = TypeId();
    }

    BundleResource::~BundleResource()
    {

    }

    std::string BundleResource::TypeId( )
    {
		return "BinaryChunk";
    }

}