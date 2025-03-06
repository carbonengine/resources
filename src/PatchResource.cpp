#include "PatchResource.h"

#include <sstream>

#include <ResourceTools.h>

#include <ResourceGroup.h>

namespace CarbonResources
{

    PatchResource::PatchResource( const PatchResourceParams& params ):
      Resource(params)
    {
		m_type = TypeId();
    }

    PatchResource::~PatchResource()
    {

    }

    std::string PatchResource::TypeId( )
    {
		return "BinaryPatch";
    }


}