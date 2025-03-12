#include "BundleResourceGroup.h"
#include "BundleResourceGroupImpl.h"

namespace CarbonResources
{

    BundleResourceGroup::BundleResourceGroup(  ) :
	    ResourceGroup( new BundleResourceGroupImpl( )),
		m_impl( reinterpret_cast<BundleResourceGroupImpl*>( ResourceGroup::m_impl ) )
    {

    }

    BundleResourceGroup::~BundleResourceGroup()
    {

    }

    /// @brief Unpack files from a bundle
	/// @param data data which the checksum will be based on
	/// @param data_size size of data passed in
	/// @param checksum will contain the resulting checksum on success
	/// @return true on success, false on failure
	Result BundleResourceGroup::Unpack( const BundleUnpackParams& params )
    {
		return m_impl->Unpack(params);
    }

}