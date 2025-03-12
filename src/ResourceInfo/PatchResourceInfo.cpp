#include "PatchResourceInfo.h"

#include <sstream>

#include <ResourceTools.h>

#include <ResourceGroup.h>

namespace CarbonResources
{

    PatchResourceInfo::PatchResourceInfo( const PatchResourceInfoParams& params ):
      ResourceInfo(params)
    {
		m_type = TypeId();
    }

    PatchResourceInfo::~PatchResourceInfo()
    {

    }

    std::string PatchResourceInfo::TypeId( )
    {
		return "BinaryPatch";
    }


}