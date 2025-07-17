// Copyright © 2025 CCP ehf.

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

    Result BundleResourceInfo::SetParametersFromResource( const ResourceInfo* other, const VersionInternal& documentVersion )
	{
		return ResourceInfo::SetParametersFromResource( other, documentVersion );
	}

}