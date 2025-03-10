#include "ResourceGroupImpl.h"

#include <fstream>
#include <sstream>
#include <yaml-cpp/yaml.h>
#include <ResourceTools.h>
//#include "Resource.h"
//#include "BinaryResource.h"
//#include "PatchResource.h"
#include "ResourceInfo/PatchResourceGroupInfo.h"
#include "ResourceInfo/ResourceGroupInfo.h"
#include "ResourceInfo/PatchResourceInfo.h"
#include "PatchResourceGroupImpl.h"

namespace CarbonResources
{
    

    ResourceGroupImpl::ResourceGroupImpl()
    {
		m_versionParameter = S_DOCUMENT_VERSION;

		m_type = TypeId();
    }

    ResourceGroupImpl::~ResourceGroupImpl()
    {
		m_resourcesParameter.Clear();
    }

    Result ResourceGroupImpl::ImportFromData( const std::string& data, DocumentType documentType /* = DocumentType::YAML */)
    {
        switch (documentType)
        {
		case DocumentType::CSV:
			return ImportFromCSV( data );
		case DocumentType::YAML:
			return ImportFromYaml( data );
		default:
			return Result::UNSUPPORTED_FILE_FORMAT;
        }

        return Result::FAIL;
    }

    Result ResourceGroupImpl::ImportFromFile( ResourceGroupImportFromFileParams& params )
    {
        if (params.filename.empty())
        {
			return Result::FILE_NOT_FOUND;
        }

        std::string data;

		if( !ResourceTools::GetLocalFileData( params.filename, data ) )
		{
			return Result::FAILED_TO_OPEN_FILE;
		}

        // VERSION NEEDS TO BE CHECKED TO ENSURE ITS SUPPORTED ON IMPORT
		std::filesystem::path filename = params.filename;

        std::string extension = filename.extension().string();
        
        if( extension == ".txt" )
        {
			return ImportFromCSV( data );
        }
		else if( extension == ".yml" )
        {
			return ImportFromYaml( data );
        }
		else if( extension == ".yaml" )
		{
			return ImportFromYaml( data );
		}
        else
        {
			return Result::UNSUPPORTED_FILE_FORMAT;
        }
    }

    Result ResourceGroupImpl::ExportToFile( const ResourceGroupExportToFileParams& params ) const
    {
		std::string data = "";

        Result exportYamlResult = ExportYaml( params.outputDocumentVersion, data );

        if (exportYamlResult != Result::SUCCESS)
        {
			return exportYamlResult;
        }

        if( !ResourceTools::SaveFile( params.filename, data ) )
		{
			return Result::FAILED_TO_SAVE_FILE;
		}

		return Result::SUCCESS;
    }

    Result ResourceGroupImpl::ExportToData( std::string& data,  Version outputDocumentVersion /* = S_DOCUMENT_VERSION*/) const
    {
		Result exportYamlResult = ExportYaml( outputDocumentVersion, data );

		if( exportYamlResult != Result::SUCCESS )
		{
			return exportYamlResult;
		}

        return Result::SUCCESS;
    }

    Result ResourceGroupImpl::ImportFromCSV( const std::string& data )
    {
        std::stringstream inputStream;

        inputStream << data;

		std::string stringIn;

		while( !inputStream.eof() )
		{
			std::getline( inputStream, stringIn );

            if (stringIn == "")
            {
				continue;
            }

            std::stringstream ss(stringIn);

            std::string value;

            char delimiter = ',';

            ResourceInfoParams resourceParams;

            if( !std::getline( ss, value, delimiter ) )
            {
				return Result::MALFORMED_RESOURCE_INPUT;
            }

            // Split filename and prefix
			std::string resourcePrefixDelimiter = ":/";
			std::string filename = value.substr( value.find(resourcePrefixDelimiter) + resourcePrefixDelimiter.size() );
			std::string resourceType = value.substr( 0, value.find( ":" ) );

			resourceParams.relativePath = filename;

			if( !std::getline( ss, value, delimiter ) )
			{
				return Result::MALFORMED_RESOURCE_INPUT;
			}

			resourceParams.location = value;

			if( !std::getline( ss, value, delimiter ) )
			{
				return Result::MALFORMED_RESOURCE_INPUT;
			}

			resourceParams.checksum = value;

			if( !std::getline( ss, value, delimiter ) )
			{
				return Result::MALFORMED_RESOURCE_INPUT;
			}

			resourceParams.uncompressedSize = atol( value.c_str() );

			if( !std::getline( ss, value, delimiter ) )
			{
				return Result::MALFORMED_RESOURCE_INPUT;
			}

			resourceParams.compressedSize = atol( value.c_str() );

            // ResourceGroup gets upgraded to 0.1.0
			m_versionParameter = Version{ 0, 1, 0 };

			// Create a Resource
			ResourceInfo* resource = new ResourceInfo( resourceParams );

            m_resourcesParameter.PushBack( resource );
		}

		return Result::SUCCESS;
    }

    //TODO not a good structure, Don't like returns like this
	//TODO Feels a bit strange that resourceGroup is thr one that creates this and not Resource
    //TODO there are two of these functions I think
	ResourceInfo* ResourceGroupImpl::CreateResourceFromResource( ResourceInfo* resource ) const
    {
		ResourceInfoParams resourceParams;

        resourceParams.relativePath = resource->GetRelativePath();

        resourceParams.location = resource->GetLocation();

        resourceParams.checksum = resource->GetChecksum();

        resourceParams.compressedSize = resource->GetCompressedSize();

        resourceParams.uncompressedSize = resource->GetUncompressedSize();

        resourceParams.something = resource->GetSomething();

		ResourceInfo* createdResource = new ResourceInfo( resourceParams );

        return createdResource;
    }

	Result ResourceGroupImpl::CreateResourceFromYaml( YAML::Node& resource, ResourceInfo*& resourceOut )
	{
		resourceOut = new ResourceInfo( ResourceInfoParams{} );

		Result importFromYamlResult = resourceOut->ImportFromYaml( resource, m_versionParameter.GetValue() );

        if( importFromYamlResult != Result::SUCCESS )
		{
			delete resourceOut;

			resourceOut = nullptr;

			return importFromYamlResult;
		}
        else
        {
			return Result::SUCCESS;
        }

	}

    Result ResourceGroupImpl::ImportFromYaml( const std::string& data )
    {
        YAML::Node resourceGroupFile = YAML::Load( data );
        
        std::string versionStr = resourceGroupFile[m_versionParameter.GetTag()].as<std::string>(); //version stringID needs to be in one place
		
        Version version;
		version.FromString( versionStr );
        m_versionParameter = version;

		if( m_versionParameter.GetValue().major > S_DOCUMENT_VERSION.major )
        {
			return Result::DOCUMENT_VERSION_UNSUPPORTED;
        }

        // If version is greater than the max version supported at compile then ceil to that
        if (version > S_DOCUMENT_VERSION)
        {
            //TODO there should perhaps be a warning that some data will be missed
			version = S_DOCUMENT_VERSION;
        }

        m_type = resourceGroupFile[m_type.GetTag()].as<std::string>();

        /*
        * TODO reinstate this validation
		if( m_type.GetValue() != TypeId() )
		{
			return Result::FILE_TYPE_MISMATCH;
		}
        */

        Result res = ImportGroupSpecialisedYaml( resourceGroupFile );

        if( res != Result::SUCCESS )
		{
			return res;
		}

        YAML::Node resources = resourceGroupFile[m_resourcesParameter.GetTag()];

        
        for (auto iter = resources.begin(); iter != resources.end(); iter++)
        {
            // This bit is a sequence
			YAML::Node resourceNode = (*iter);
            
            ResourceInfo* resource = nullptr;

            Result createResourceFromYamlResult = CreateResourceFromYaml( resourceNode, resource );

            if (createResourceFromYamlResult != Result::SUCCESS)
            {
				return createResourceFromYamlResult;
            }

            m_resourcesParameter.PushBack( resource );

        }

		return Result::SUCCESS;
    }

    std::string ResourceGroupImpl::GetType() const
	{
		return TypeId();
	}

    std::string ResourceGroupImpl::TypeId() 
    {
		return "ResourceGroup";
    }

    Result ResourceGroupImpl::ImportGroupSpecialisedYaml( YAML::Node& resourceGroupFile )
    {
        return Result::SUCCESS;
    }

    Result ResourceGroupImpl::ExportGroupSpecialisedYaml( YAML::Emitter& out, Version outputDocumentVersion ) const
    {
		return Result::SUCCESS;
    } 

    Result ResourceGroupImpl::ExportYaml( const Version& outputDocumentVersion, std::string& data ) const
    {
		
        YAML::Emitter out;

        // Output header information
		out << YAML::BeginMap;

        // It is possible to export a different version that the imported version
        // The version must be less than the version of the document and also no higher than supported by the binary at compile
		Version sanitisedOutputDocumentVersion = outputDocumentVersion;
		const Version documentCurrentVersion = m_versionParameter.GetValue();

        if( sanitisedOutputDocumentVersion > documentCurrentVersion )
        {
			sanitisedOutputDocumentVersion = documentCurrentVersion;
        }

        if (sanitisedOutputDocumentVersion > S_DOCUMENT_VERSION)
        {
			sanitisedOutputDocumentVersion = S_DOCUMENT_VERSION;
        }

        //Export document parameters
	    out << YAML::Key << m_versionParameter.GetTag();
		out << YAML::Value << sanitisedOutputDocumentVersion.ToString(); 

        out << YAML::Key << m_type.GetTag();
		out << YAML::Value << m_type.GetValue();

        Result res = ExportGroupSpecialisedYaml( out, sanitisedOutputDocumentVersion );

		if( res != Result::SUCCESS )
        {
			return res;
        }

        out << YAML::Key << m_resourcesParameter.GetTag();

        out << YAML::Value << YAML::BeginSeq;


        for (ResourceInfo* r : m_resourcesParameter)
        {
			out << YAML::BeginMap;

			Result resourceExportResult = r->ExportToYaml( out, sanitisedOutputDocumentVersion );

            if( resourceExportResult != Result::SUCCESS )
			{
				return resourceExportResult;
			}

            out << YAML::EndMap;
			
        }

        out << YAML::EndSeq;

		out << YAML::EndMap;

        data = out.c_str();

        return Result::SUCCESS;
      
    }

    Result ResourceGroupImpl::CreatePatch( PatchCreateParams& params ) const
    {
        if (params.previousResourceGroup->m_impl->GetType() != GetType())
        {
			return Result::PATCH_RESOURCE_LIST_MISSMATCH;
        }

        PatchResourceGroupImpl patchResourceGroup;

        // Subtraction //TODO this needs to match the format of the original input resource lists
        // Put in place when there is a factory
		ResourceGroupImpl resourceGroupSubtractionPrevious;

        ResourceGroupImpl resourceGroupSubtractionLatest;

        ResourceGroupSubtractionParams resourceGroupSubtractionParams;

		resourceGroupSubtractionParams.subtractResourceGroup = params.previousResourceGroup->m_impl;

		resourceGroupSubtractionParams.result1 = &resourceGroupSubtractionPrevious;

        resourceGroupSubtractionParams.result2 = &resourceGroupSubtractionLatest;

        Result subtractionResult = Diff( resourceGroupSubtractionParams );

        if (subtractionResult != Result::SUCCESS)
        {
			return subtractionResult;
        }

        // Ensure that the diff results have the same number of members
        if (resourceGroupSubtractionPrevious.m_resourcesParameter.GetSize() != resourceGroupSubtractionLatest.m_resourcesParameter.GetSize())
        {
			return Result::UNEXPECTED_PATCH_DIFF_ENCOUNTERED;
        }

        for (int i = 0; i < resourceGroupSubtractionLatest.m_resourcesParameter.GetSize(); i++)
        {
			ResourceInfo* resourcePrevious = resourceGroupSubtractionPrevious.m_resourcesParameter.At( i );

			ResourceInfo* resourceLatest = resourceGroupSubtractionLatest.m_resourcesParameter.At( i );

            // Check to see if previous entry contains dummy information
            // Suggesting that this is a new entry in latest
            // In which case there is no reason to create a patch
            // The new entry will be stored with the ResourceGroup related to the PatchResourceGroup
            if (resourcePrevious->GetCompressedSize() != 0)
            {
                // Resource is present in both previous and next with differing checksums
                // Therefore a patch for the binary is created

				// Get resource data previous
				ResourceGetDataParams resourceGetDataParamsFrom;

				resourceGetDataParamsFrom.resourceSourceSettings = params.resourceSourceSettingsFrom;

				Result fromResourceDataResult = resourcePrevious->GetData( resourceGetDataParamsFrom );

				if( fromResourceDataResult != Result::SUCCESS )
				{
					return fromResourceDataResult;
				}

				// Get resource data next
				ResourceGetDataParams resourceGetDataParamsTo;

				resourceGetDataParamsTo.resourceSourceSettings = params.resourceSourceSettingsTo;

				Result toResourceDataResult = resourceLatest->GetData( resourceGetDataParamsTo );

				if( toResourceDataResult != Result::SUCCESS )
				{
					return toResourceDataResult;
				}

				// TODO only create a patch if this isn't a new file

				// Create a patch from the data
				ResourcePutDataParams resourcePutDataParams;

				if( !ResourceTools::CreatePatch( resourceGetDataParamsFrom.data, resourceGetDataParamsTo.data, resourcePutDataParams.data ) )
				{
					return Result::FAILED_TO_CREATE_PATCH;
				}

				// Create a resource from patch data
				PatchResourceInfo* patchResource = new PatchResourceInfo( { resourceLatest->GetRelativePath() } );

				patchResource->SetParametersFromData( resourcePutDataParams.data );

				// Export patch file
				resourcePutDataParams.resourceDestinationSettings = params.resourcePatchBinaryDestinationSettings;

				Result putPatchDataResult = patchResource->PutData( resourcePutDataParams );

				if( putPatchDataResult != Result::SUCCESS )
				{
					delete patchResource;

					return putPatchDataResult;
				}

				// Add the patch resource to the patchResourceGroup
				patchResourceGroup.AddResource( patchResource );
            }

        }

        // Export the subtraction ResourceGroup
        std::string resourceGroupData;

        resourceGroupSubtractionLatest.ExportToData( resourceGroupData );

		ResourceGroupInfo subtractionResourceGroupInfo( { params.resourceGroupRelativePath } );

        subtractionResourceGroupInfo.SetParametersFromData( resourceGroupData );

        ResourcePutDataParams putDataParams;

        putDataParams.resourceDestinationSettings = params.resourcePatchBinaryDestinationSettings;

        putDataParams.data = resourceGroupData; // TODO copy here remove, make it take a pointer

        Result subtractionResourcePutResult = subtractionResourceGroupInfo.PutData( putDataParams );

        if (subtractionResourcePutResult != Result::SUCCESS)
        {
			return subtractionResourcePutResult;
        }

      

        // Export the patchGroup
		patchResourceGroup.SetResourceGroup( subtractionResourceGroupInfo );

        std::string patchResourceGroupData;

        patchResourceGroup.ExportToData( patchResourceGroupData );

		PatchResourceGroupInfo patchResourceGroupInfo( { params.resourceGroupPatchRelativePath } );

        patchResourceGroupInfo.SetParametersFromData( patchResourceGroupData );

        ResourcePutDataParams patchPutDataParams;

		patchPutDataParams.resourceDestinationSettings = params.resourcePatchResourceGroupDestinationSettings;

		patchPutDataParams.data = patchResourceGroupData; // TODO copy here remove, make it take a pointer

		Result patchResourceGroupPutResult = patchResourceGroupInfo.PutData( patchPutDataParams );

        if( patchResourceGroupPutResult != Result::SUCCESS )
		{
			return patchResourceGroupPutResult;
		}


 
        return Result::SUCCESS;
    }


    Result ResourceGroupImpl::AddResource( ResourceInfo* resource )
    {
		m_resourcesParameter.PushBack( resource );

		return Result::SUCCESS;
    }

    

    Result ResourceGroupImpl::Diff( ResourceGroupSubtractionParams& params ) const
    {
		DocumentParameterCollection<ResourceInfo*> subtractionResources = params.subtractResourceGroup->m_resourcesParameter;


        // Iterate through all resources
        for (ResourceInfo* resource : m_resourcesParameter)
        {
            // Note: here we can also detect if a value is not present in the latest that was present in previous
            // We could remove those files
			auto subtractionResourcesFindIter = subtractionResources.Find( resource );

            if( subtractionResourcesFindIter != subtractionResources.end() )
			{
				ResourceInfo* resource2 = ( *subtractionResourcesFindIter );

                // Has this resource changed?
                if (resource->GetChecksum() != resource2->GetChecksum())
                {
                    // The binary data has changed between versions, record an entry in both lists

					// Create a copy of the resource to result 2 (Latest)
					ResourceInfo* resourceCopy1 = CreateResourceFromResource( resource );

					params.result2->AddResource( resourceCopy1 );

                    // Create a copy of resource to result 1 (Previous)
					ResourceInfo* resource2 = ( *subtractionResourcesFindIter );

					ResourceInfo* resourceCopy2 = CreateResourceFromResource( resource2 );

					params.result1->AddResource( resourceCopy2 );
                }
			}
            else
            {
                // This is a new resource, add it to target
                // Note: Could be made optional, perhaps it is desirable to only include patch updates
				// Not new files, probably make as optional pass in setting
				ResourceInfo* resourceCopy1 = CreateResourceFromResource( resource );

				params.result2->AddResource( resourceCopy1 );

                // Place in a dummy entry into result1 which shows that resource is new
                // This ensures that both lists stay the same size which makes it easier
                // To parse later
				ResourceInfoParams dummyResourceParams;
				dummyResourceParams.relativePath = resource->GetRelativePath();

				ResourceInfo* dummyResource = new ResourceInfo( dummyResourceParams );
				params.result1->AddResource( dummyResource );

            }
            
        }

        return Result::SUCCESS;
    }


}