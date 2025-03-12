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
#include "ResourceInfo/ResourceInfo.h"
#include <vector>

#include "Macros.h"

namespace YAML
{
    class Emitter;
    class Node;
}

namespace CarbonResources
{

    class BundleResourceGroupImpl;

    struct ResourceGroupSubtractionParams
    {
	    ResourceGroupImpl* subtractResourceGroup = nullptr;

	    ResourceGroupImpl* result1 = nullptr;

	    ResourceGroupImpl* result2 = nullptr;
    };

    enum class DocumentType
    {
        CSV,
        YAML
    };


    class ResourceGroupImpl
    {
    public:
		ResourceGroupImpl();

	    ~ResourceGroupImpl();

	    Result ImportFromFile( const ResourceGroupImportFromFileParams& params );

        Result ImportFromData( const std::string& data, DocumentType documentType = DocumentType::YAML );

	    Result ExportToFile( const ResourceGroupExportToFileParams& params ) const;

        Result ExportToData( std::string& data, Version outputDocumentVersion = S_DOCUMENT_VERSION ) const;

        Result CreateBundle( const BundleCreateParams& params ) const;

	    Result CreatePatch( const PatchCreateParams& params ) const;

	    Result AddResource( ResourceInfo* resource );

        Result Diff( ResourceGroupSubtractionParams& params ) const;
        
        virtual std::string GetType() const;

        static std::string TypeId();

    protected:

        virtual Result CreateResourceFromYaml( YAML::Node& resource, ResourceInfo*& resourceOut );

    private:
	    
        
        virtual ResourceInfo* CreateResourceFromResource( ResourceInfo* resource ) const; // TODO this function should match signature of others return Result etc

	    virtual Result ImportGroupSpecialisedYaml( YAML::Node& resourceGroupFile );

	    virtual Result ExportGroupSpecialisedYaml( YAML::Emitter& out, Version outputDocumentVersion ) const;

	    virtual Result [[deprecated( "Prfer yaml" )]] ImportFromCSV( const std::string& data );

	    Result ImportFromYaml( const std::string& data );

	    Result ExportYaml( const Version& outputDocumentVersion, std::string& data ) const;

        Result ProcessChunk( std::string& chunkData, const std::filesystem::path& chunkRelativePath, BundleResourceGroupImpl& bundleResourceGroup, const ResourceDestinationSettings& chunkDestinationSettings ) const;

    public: // TODO not thrilled by this is there a better way?
	    // Document Parameters
	    DocumentParameter<Version> m_versionParameter = DocumentParameter<Version>( { 1, 0, 0 }, "Version" );

        DocumentParameter<std::string> m_type = DocumentParameter<std::string>( { 1, 0, 0 }, "Type" );

	    DocumentParameterCollection<ResourceInfo*> m_resourcesParameter = DocumentParameterCollection<ResourceInfo*>( { 0, 0, 0 }, "Resources" );
    };

}

#endif // ResourceGroupImpl_H