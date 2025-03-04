#include "PatchResourceGroup.h"
#include "PatchResourceGroupImpl.h"

namespace CarbonResources
{
    PatchResourceGroup::PatchResourceGroup( const std::string& relativePath ) :
	    ResourceGroup( new PatchResourceGroupImpl( relativePath ) ),
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
	    return Result::FAIL;
    }


    // Not public expose
	Result PatchResourceGroup::SetResourceGroupPath( const std::string& resourceGroupPath )
    {
		return m_impl->SetResourceGroupPath( resourceGroupPath );
    }
}