#include "PatchResource.h"

#include "PatchResourceImpl.h"

#include <yaml-cpp/yaml.h>

namespace CarbonResources
{
    PatchResourceParams::PatchResourceParams() :
		  ResourceParams()
    {
    }


    PatchResource::PatchResource( const PatchResourceParams& params ) :
		Resource( new PatchResourceImpl( params ) ),
		m_impl( reinterpret_cast<PatchResourceImpl*>( Resource::m_impl ) )
    {

    }

    PatchResource::~PatchResource( )
    {
		
    }
	
}