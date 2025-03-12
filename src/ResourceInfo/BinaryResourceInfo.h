/* 
	*************************************************************************

	BinaryResourceInfo.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/
#pragma once
#ifndef BinaryResourceInfo_H
#define BinaryResourceInfo_H

#include "ResourceInfo.h"

namespace CarbonResources
{
    struct BinaryResourceInfoParams : public ResourceInfoParams
    {

	    unsigned int binaryOperation = 0;

    };

    class BinaryResourceInfo : public ResourceInfo
    {
    public:
	    BinaryResourceInfo( const BinaryResourceInfoParams& params );

	    ~BinaryResourceInfo();

	    Result GetBinaryOperation(unsigned int& binaryOperation) const;

	    virtual Result ImportFromYaml( YAML::Node& resource, const Version& documentVersion ) override;

	    virtual Result ExportToYaml( YAML::Emitter& out, const Version& documentVersion ) override;

        static std::string TypeId();

    private:
	    DocumentParameter<unsigned int> m_binaryOperation = DocumentParameter<unsigned int>( { 0, 0, 0 }, "BinaryOperation" );
    };

}

#endif // BinaryResourceInfo_H