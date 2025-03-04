/* 
	*************************************************************************

	ResourceGroupImpl.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/
#pragma once
#ifndef ResourceGroupImpl_H
#define ResourceGroupImpl_H

#include "ResourceGroup.h"
#include "Resource.h"
#include "ResourceImpl.h"
#include <vector>

namespace YAML
{
    class Emitter;
    class Node;
}

namespace CarbonResources
{

    class ResourceGroupImpl : public ResourceImpl
    {
    public:
		ResourceGroupImpl( const std::string& relativePath );

	    ~ResourceGroupImpl();

	    Result ImportFromFile( ResourceGroupImportFromFileParams& params );

	    Result ExportToFile( const ResourceGroupExportToFileParams& params );

	    Result CreatePatch( const PatchCreateParams& params ) const;

	    Result AddResource( const Resource& resource );

    private:
	    virtual std::string Type() const;

	    virtual Resource* CreateResourceFromYaml( YAML::Node& resource );   // TODO this function should match signature of others return Result etc

	    virtual Result ImportGroupSpecialisedYaml( YAML::Node& resourceGroupFile );

	    virtual Result ExportGroupSpecialisedYaml( YAML::Emitter& out, Version outputDocumentVersion ) const;

	    virtual Result [[deprecated( "Prfer yaml" )]] ImportFromCSVFile( ResourceGroupImportFromFileParams& params );

	    Result ImportFromYamlFile( ResourceGroupImportFromFileParams& params );

	    Result ExportYamlToFile( const ResourceGroupExportToFileParams& params ) const;

    protected:
	    // Document Parameters
	    DocumentParameter<Version> m_versionParameter = DocumentParameter<Version>( { 1, 0, 0 }, "Version" );

	    DocumentParameter<std::string> m_typeParameter = DocumentParameter<std::string>( { 1, 0, 0 }, "Type" );

	    DocumentParameterCollection<Resource*> m_resourcesParameter = DocumentParameterCollection<Resource*>( { 0, 0, 0 }, "Resources" );
    };

}

#endif // ResourceGroupImpl_H