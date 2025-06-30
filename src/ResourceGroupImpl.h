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

#include <BundleStreamOut.h>
#include "ResourceGroup.h"
#include "ResourceInfo/ResourceInfo.h"
#include <vector>

#include "VersionInternal.h"
#include "ResourceInfo/PatchResourceInfo.h"

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

    	std::vector<std::filesystem::path> removedResources;

        StatusCallback statusCallback = nullptr;
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

	    virtual ~ResourceGroupImpl();

        Result CreateFromDirectory( const CreateResourceGroupFromDirectoryParams& params );

	    Result ImportFromFile( const ResourceGroupImportFromFileParams& params );

        Result ImportFromData( const std::string& data, DocumentType documentType = DocumentType::YAML );

	    Result ExportToFile( const ResourceGroupExportToFileParams& params ) const;

        Result ExportToData( std::string& data, VersionInternal outputDocumentVersion = S_DOCUMENT_VERSION ) const;

        Result CreateBundle( const BundleCreateParams& params ) const;

		Result ConstructPatchResourceInfo( const PatchCreateParams& params, int patchId, uintmax_t dataOffset, uint64_t patchSourceOffset, ResourceInfo* resourceNext, PatchResourceInfo*& patchResource ) const;

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

        size_t GetSize() const;

	    Result ImportFromYamlString( const std::string& data, StatusCallback statusCallback = nullptr );

    	Result ImportFromYaml( YAML::Node& data, StatusCallback statusCallback = nullptr );

		virtual Result GetGroupSpecificResourcesToBundle( std::vector<ResourceInfo*>& toBundle ) const;

    protected:

        virtual Result CreateResourceFromYaml( YAML::Node& resource, ResourceInfo*& resourceOut );

    private:

        virtual Result CreateResourceFromResource( const ResourceInfo& resourceIn, ResourceInfo*& resourceOut ) const;

	    virtual Result ImportGroupSpecialisedYaml( YAML::Node& resourceGroupFile );

	    virtual Result ExportGroupSpecialisedYaml( YAML::Emitter& out, VersionInternal outputDocumentVersion ) const;

    	[[deprecated( "Prefer yaml" )]]
		virtual Result ImportFromCSV( const std::string& data, StatusCallback statusCallback = nullptr );

	    Result ExportYaml( const VersionInternal& outputDocumentVersion, std::string& data, StatusCallback statusCallback = nullptr ) const;

	    Result ExportCsv( const VersionInternal& outputDocumentVersion, std::string& data, StatusCallback statusCallback = nullptr ) const;

        Result ProcessChunk( ResourceTools::GetChunk& chunkData, const std::filesystem::path& chunkRelativePath, BundleResourceGroupImpl& bundleResourceGroup, const ResourceDestinationSettings& chunkDestinationSettings ) const;

    protected:

	    // Document Parameters
	    DocumentParameter<VersionInternal> m_versionParameter = DocumentParameter<VersionInternal>( VERSION, TypeId() );

        DocumentParameter<std::string> m_type = DocumentParameter<std::string>( TYPE, TypeId() );

        DocumentParameter<uintmax_t> m_numberOfResources = DocumentParameter<uintmax_t>( NUMBER_OF_RESOURCES, TypeId() );

        DocumentParameter<uintmax_t> m_totalResourcesSizeCompressed = DocumentParameter<uintmax_t>( TOTAL_RESOURCE_SIZE_COMPRESSED, TypeId() );

        DocumentParameter<uintmax_t> m_totalResourcesSizeUncompressed = DocumentParameter<uintmax_t>( TOTAL_RESOURCE_SIZE_UNCOMPRESSED, TypeId() );

	    DocumentParameterCollection<ResourceInfo*> m_resourcesParameter = DocumentParameterCollection<ResourceInfo*>( RESOURCE, TypeId() );
    };

}

#endif // ResourceGroupImpl_H