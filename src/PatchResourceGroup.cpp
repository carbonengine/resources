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

    /// @brief Apply patches
    /// @param data data which the checksum will be based on
    /// @param data_size size of data passed in
    /// @param checksum will contain the resulting checksum on success
    /// @return true on success, false on failure
    Result PatchResourceGroup::Apply( const PatchApplyParams& params )
    {
		return m_impl->Apply( params );
    }

}