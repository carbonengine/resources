#include "BinaryResourceImpl.h"

#include <sstream>

#include <ResourceTools.h>

namespace CarbonResources
{

    BinaryResource::BinaryResourceImpl::BinaryResourceImpl( const BinaryResourceParams& params ):
        ResourceImpl(params),
	    m_binaryOperation(params.binaryOperation)
    {

    }

    BinaryResource::BinaryResourceImpl::~BinaryResourceImpl()
    {

    }

    DocumentParameter<unsigned int> BinaryResource::BinaryResourceImpl::GetBinaryOperation() const
	{
		return m_binaryOperation;
	}

}