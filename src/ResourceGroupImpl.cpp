#include "ResourceGroupImpl.h"
#include "Resource.h"

#include <fstream>
#include <sstream>
#include <yaml-cpp/yaml.h>
#include <ResourceTools.h>
#include <PatchResourceGroup.h>
#include <PatchResource.h>

namespace CarbonResources
{
    

    ResourceGroupImpl::ResourceGroupImpl( const std::string& relativePath ) :
	    ResourceImpl( ResourceParams{ relativePath } )
    {

    }

    ResourceGroupImpl::~ResourceGroupImpl()
    {
		m_resourcesParameter.Clear();
    }

    Result ResourceGroupImpl::ImportFromFile( ResourceGroupImportFromFileParams& params )
    {
        // VERSION NEEDS TO BE CHECKED TO ENSURE ITS SUPPORTED ON IMPORT
		
        std::string filename = GetRelativePath().GetValue().filename;

        if( filename.find( ".txt" ) != std::string::npos )
        {
			return ImportFromCSVFile( params );
        }
		else if( filename.find( ".yml" ) != std::string::npos )
        {
			return ImportFromYamlFile( params );
        }
		else if( filename.find( ".yaml" ) != std::string::npos )
		{
			return ImportFromYamlFile( params );
		}
        else
        {
			return Result::UNSUPPORTED_FILE_FORMAT;
        }
    }

    Result ResourceGroupImpl::ExportToFile( const ResourceGroupExportToFileParams& params )
    {
		return ExportYamlToFile( params );
    }

    Result ResourceGroupImpl::ImportFromCSVFile( ResourceGroupImportFromFileParams& params )
    {
	
		Result getDataResult = GetData( params.dataParams );

        if( getDataResult != Result::SUCCESS )
        {
			return Result::FAILED_TO_OPEN_FILE;
        }

        std::stringstream inputStream;

        inputStream << params.dataParams.data;

		std::string stringIn;

		while( !inputStream.eof() )
		{
			std::getline( inputStream, stringIn );

            std::stringstream ss(stringIn);

            std::string value;

            char delimiter = ',';

            ResourceParams resourceParams;

            if( !std::getline( ss, value, delimiter ) )
            {
				return Result::MALFORMED_RESOURCE_INPUT;
            }

			resourceParams.relativePath = value;

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
			Resource* resource = new Resource( resourceParams );

            m_resourcesParameter.PushBack( resource );
		}

		return Result::SUCCESS;
    }

    //TODO not a good structure, Don't like returns like this
    Resource* ResourceGroupImpl::CreateResourceFromYaml( YAML::Node& resource )
	{
		Resource* createdResource = new Resource( ResourceParams{} );

		Result importFromYamlResult = createdResource->m_impl->ImportFromYaml( resource, m_versionParameter.GetValue() );

        if( importFromYamlResult != Result::SUCCESS )
		{
			delete createdResource;
			return nullptr;
		}
        else
        {
			return createdResource;
        }

		return nullptr;
	}

    Result ResourceGroupImpl::ImportFromYamlFile( ResourceGroupImportFromFileParams& params )
    {
		Result getDataResult = GetData( params.dataParams );

		if( getDataResult != Result::SUCCESS )
		{
			return Result::FAILED_TO_OPEN_FILE;
		}

        YAML::Node resourceGroupFile = YAML::Load( params.dataParams.data );
        
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

        m_typeParameter = resourceGroupFile[m_typeParameter.GetTag()].as<std::string>();

		if( m_typeParameter.GetValue() != Type() )
		{
			return Result::FILE_TYPE_MISMATCH;
		}

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
            
            Resource* resource = CreateResourceFromYaml( resourceNode );

            m_resourcesParameter.PushBack( resource );

        }

		return Result::SUCCESS;
    }

    std::string ResourceGroupImpl::Type() const
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

    Result ResourceGroupImpl::ExportYamlToFile( const ResourceGroupExportToFileParams& params ) const
    {
		std::ofstream outputStream;

		outputStream.open( params.outputFilename, std::ios::out );

        if( !outputStream )
		{
			return Result::FAILED_TO_OPEN_FILE;
		}

        YAML::Emitter out;

        // Output header information
		out << YAML::BeginMap;

        // It is possible to export a different version that the imported version
        // The version must be less than the version of the document and also no higher than supported by the binary at compile
		Version sanitisedOutputDocumentVersion = params.outputDocumentVersion;
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

        out << YAML::Key << m_typeParameter.GetTag();
		out << YAML::Value << Type();

        Result res = ExportGroupSpecialisedYaml( out, sanitisedOutputDocumentVersion );

		if( res != Result::SUCCESS )
        {
			return res;
        }

        out << YAML::Key << m_resourcesParameter.GetTag();

        out << YAML::Value << YAML::BeginSeq;


        for (Resource* r : m_resourcesParameter)
        {
			out << YAML::BeginMap;

			Result resourceExportResult = r->m_impl->ExportToYaml( out, sanitisedOutputDocumentVersion );

            if( resourceExportResult != Result::SUCCESS )
			{
				return resourceExportResult;
			}

            out << YAML::EndMap;
			
        }

        out << YAML::EndSeq;

		out << YAML::EndMap;

        outputStream << out.c_str();

        outputStream.close();


		return Result::SUCCESS;
    }



    Result ResourceGroupImpl::CreatePatch( const PatchCreateParams& params ) const
    {
        // TODO currently wip, working it through currently
        // TODO there needs to be validation of the input parameters and tests for all variations

        // Make a copy of the previous resources so that matches can be removed to speed up searches
		DocumentParameterCollection<Resource*> previousResources = params.previousResourceGroup->m_impl->m_resourcesParameter;

        // Loop through each resource and create a patch if necessary
        for (Resource* latestResource : m_resourcesParameter)
        {
            
            auto iter = std::find( previousResources.begin(), previousResources.end(), latestResource );

            if (iter != previousResources.end())
            {
				Resource* previousResource = ( *iter );

                previousResources.Remove( iter );

                // Create a patch between previousResource and resource
				ResourceGetDataParams getDataParamsForLatest;

				getDataParamsForLatest.resourceSourceSettings = params.resourceSourceSettings;

				latestResource->GetData( getDataParamsForLatest );

                // Get data for previous resource
                ResourceGetDataParams getDataParamsForPrevious;
				
                getDataParamsForPrevious.resourceSourceSettings = params.resourceSourceSettings;

                previousResource->GetData( getDataParamsForPrevious );

                // Create binary patch
				std::string patchData;
				ResourceTools::CreatePatch( getDataParamsForPrevious.data, getDataParamsForLatest.data, patchData );

                // Create a resource for the patch (TODO this is set to change out of here)
				PatchResourceParams patchResourceParams;

                patchResourceParams.relativePath = "patch:/TODO";

                patchResourceParams.location = "TODO";

                patchResourceParams.checksum = "TODO";

                patchResourceParams.compressedSize = 0;

                patchResourceParams.uncompressedSize = 0;

                patchResourceParams.something = 0;

				PatchResource patchResource( patchResourceParams );

                // Add as a patch resource entry
				params.patchResourceGroup->AddResource( patchResource );


                //TODO there are lots of returns that are not being checked here, this all needs tightening up

                // Save the patch file
				ResourceTools::SaveFile( "TODO", patchData );

            }

        }

        // Input resource group needs to be a resource itself saved to disk
		//std::string underlyingResourceGroupRelativePath = params.latestResourceGroup->GetRelativePath().GetValue();
		//params.patchResourceGroup->SetResourceGroupPath( underlyingResourceGroupRelativePath );


        // Should I save the patch group here?
		ResourceGroupExportToFileParams patchResourceGroupExportParams;
		patchResourceGroupExportParams.outputFilename = "TODO";

		params.patchResourceGroup->ExportToFile( patchResourceGroupExportParams );


        return Result::FAIL;
    }


    Result ResourceGroupImpl::AddResource( const Resource& resource )
    {
		return Result::FAIL;
    }

}