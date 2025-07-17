// Copyright © 2025 CCP ehf.

#include "PatchResourceGroup.h"
#include "PatchResourceGroupImpl.h"

namespace CarbonResources
{

    PatchResourceGroup::PatchResourceGroup( ):
	    ResourceGroup(new PatchResourceGroupImpl()),
	    m_impl( reinterpret_cast<PatchResourceGroupImpl*>( ResourceGroup::m_impl ) )
    {
        
    }

    PatchResourceGroup::~PatchResourceGroup()
    {
    }

    Result PatchResourceGroup::Apply( const PatchApplyParams& params )
    {
		return m_impl->Apply( params );
    }

}