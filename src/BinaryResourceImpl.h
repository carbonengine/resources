/* 
	*************************************************************************

	BinaryResourceImpl.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/
#pragma once
#ifndef BinaryResourceImpl_H
#define BinaryResourceImpl_H

#include "BinaryResource.h"

#include "ResourceImpl.h"

class CarbonResources::BinaryResource::BinaryResourceImpl : public CarbonResources::Resource::ResourceImpl
{
public:
	BinaryResourceImpl( const BinaryResourceParams& params );

    ~BinaryResourceImpl();

    DocumentParameter<unsigned int> GetBinaryOperation() const;

private:

    DocumentParameter<unsigned int> m_binaryOperation;

};

#endif // BinaryResourceImpl_H