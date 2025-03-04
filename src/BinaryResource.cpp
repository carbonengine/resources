#include "BinaryResource.h"

#include "BinaryResourceImpl.h"

namespace CarbonResources
{
    
    BinaryResource::BinaryResource( const BinaryResourceParams& params ) :
		Resource( new BinaryResourceImpl(params) ),
		m_impl( reinterpret_cast<BinaryResourceImpl*>( Resource::m_impl ) )
    {

    }

    BinaryResource::~BinaryResource( )
    {
		
    }

    /// @brief Returns binary operation a resource
	/// @param data data which the checksum will be based on
	/// @param data_size size of data passed in
	/// @param checksum will contain the resulting checksum on success
	/// @return true on success, false on failure
	unsigned int BinaryResource::GetBinaryOperation() const
	{
		return m_impl->GetBinaryOperation().GetValue();
	}
	
}