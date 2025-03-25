/* 
	*************************************************************************

	PatchResourceInfo.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/
#pragma once
#ifndef PatchResourceInfo_H
#define PatchResourceInfo_H

#include "ResourceGroupInfo.h"

namespace CarbonResources
{
    struct PatchResourceInfoParams : public ResourceGroupInfoParams
    {
		std::filesystem::path targetResourceRelativePath = "NOT_SET";

        unsigned long dataOffset = 0;
    };

    class ResourceGroup;

    class PatchResourceInfo : public ResourceInfo
    {
    public:
	    PatchResourceInfo( const PatchResourceInfoParams& params );

	    ~PatchResourceInfo();

        static std::string TypeId( );

        Result GetTargetResourceRelativePath( std::filesystem::path& targetResourceRelativePath ) const;

        Result GetDataOffset( unsigned long& dataoffset ) const;

		virtual Result ImportFromYaml( YAML::Node& resource, const VersionInternal& documentVersion ) override;

		virtual Result ExportToYaml( YAML::Emitter& out, const VersionInternal& documentVersion ) override;

        virtual Result SetParametersFromResource( const ResourceInfo* other, const VersionInternal& documentVersion ) override;

    private:

        DocumentParameter<unsigned long> m_dataOffset = DocumentParameter<unsigned long>( { 0, 0, 0 }, "DataOffset" );

		DocumentParameter<std::filesystem::path> m_targetResourceRelativepath = DocumentParameter<std::filesystem::path>( { 0, 0, 0 }, "TargetResourceRelativePath" );
    };


}

#endif // PatchResourceInfo_H