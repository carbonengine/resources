#include "PatchResource.h"

#include <sstream>

#include <ResourceTools.h>

#include <ResourceGroup.h>

namespace CarbonResources
{

    PatchResource::PatchResource( const PatchResourceParams& params ):
      Resource(params)
    {

    }

    PatchResource::~PatchResource()
    {

    }

    Result PatchResource::GetPathPrefix( std::string& prefix ) const
    {
		prefix = "diff";

		return Result::SUCCESS;
    }


}