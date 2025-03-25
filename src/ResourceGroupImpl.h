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

#include "Version.h"

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

        Result CreateFromDirectory( const CreateResourceGroupFromDirectoryParams& params );

	    Result ImportFromFile( const ResourceGroupImportFromFileParams& params );

        Result ImportFromData( const std::string& data, DocumentType documentType = DocumentType::YAML );

	    Result ExportToFile( const ResourceGroupExportToFileParams& params ) const;

        Result ExportToData( std::string& data, VersionInternal outputDocumentVersion = S_DOCUMENT_VERSION ) const;

        Result CreateBundle( const BundleCreateParams& params ) const;

	    Result CreatePatch( const PatchCreateParams& params ) const;

	    Result AddResource( ResourceInfo* resource );

        Result Diff( ResourceGroupSubtractionParams& params ) const;
        
        virtual std::string GetType() const;

        static std::string TypeId();

        std::vector<ResourceInfo*>::iterator begin();

        std::vector<ResourceInfo*>::const_iterator begin() const;

		std::vector<ResourceInfo*>::const_iterator cbegin();

        std::vector<ResourceInfo*>::iterator end();

		std::vector<ResourceInfo*>::const_iterator end() const;

		std::vector<ResourceInfo*>::const_iterator cend();

    protected:

        virtual Result CreateResourceFromYaml( YAML::Node& resource, ResourceInfo*& resourceOut );

    private:

        virtual Result CreateResourceFromResource( const ResourceInfo& resourceIn, ResourceInfo*& resourceOut ) const;

	    virtual Result ImportGroupSpecialisedYaml( YAML::Node& resourceGroupFile );

	    virtual Result ExportGroupSpecialisedYaml( YAML::Emitter& out, VersionInternal outputDocumentVersion ) const;

	    virtual Result [[deprecated( "Prfer yaml" )]] ImportFromCSV( const std::string& data );

	    Result ImportFromYaml( const std::string& data );

	    Result ExportYaml( const VersionInternal& outputDocumentVersion, std::string& data, std::function<void( int, const std::string& )> statusCallback = nullptr ) const;

        Result ProcessChunk( std::string& chunkData, const std::filesystem::path& chunkRelativePath, BundleResourceGroupImpl& bundleResourceGroup, const ResourceDestinationSettings& chunkDestinationSettings ) const;

    protected:

	    // Document Parameters
	    DocumentParameter<VersionInternal> m_versionParameter = DocumentParameter<VersionInternal>( { 1, 0, 0 }, "Version" );

        DocumentParameter<std::string> m_type = DocumentParameter<std::string>( { 1, 0, 0 }, "Type" );

        DocumentParameter<unsigned long> m_numberOfResources = DocumentParameter<unsigned long>( { 1, 0, 0 }, "NumberOfResources" );

        DocumentParameter<unsigned long> m_totalResourcesSizeCompressed = DocumentParameter<unsigned long>( { 1, 0, 0 }, "TotalResourcesSizeCompressed" );

        DocumentParameter<unsigned long> m_totalResourcesSizeUncompressed = DocumentParameter<unsigned long>( { 1, 0, 0 }, "TotalResourcesSizeUnCompressed" );

	    DocumentParameterCollection<ResourceInfo*> m_resourcesParameter = DocumentParameterCollection<ResourceInfo*>( { 0, 0, 0 }, "Resources" );
    };

}

#endif // ResourceGroupImpl_H