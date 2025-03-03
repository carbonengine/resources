#include "BundleResource.h"

#include "BundleResourceImpl.h"

#include <yaml-cpp/yaml.h>

namespace CarbonResources
{
    BundleResourceParams::BundleResourceParams() :
	    ResourceParams()
    {
    }


    BundleResource::BundleResource( const BundleResourceParams& params ) :
		Resource( new BundleResourceImpl( params ) ),
		m_impl( reinterpret_cast<BundleResourceImpl*>( Resource::m_impl ) )
    {

    }

    BundleResource::~BundleResource( )
    {
		
    }
	
}