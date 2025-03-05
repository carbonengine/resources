#include "ResourceGroupImpl.h"

#include <fstream>
#include <sstream>
#include <yaml-cpp/yaml.h>
#include <ResourceTools.h>
#include <PatchResourceGroup.h>

namespace CarbonResources
{
    

    ResourceGroupImpl::ResourceGroupImpl( const std::string& relativePath ) :
	    Resource( ResourceParams{ relativePath } )
    {
		m_versionParameter = S_DOCUMENT_VERSION;
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
	//TODO Feels a bit strange that resourceGroup is thr one that creates this and not Resource
	Resource* ResourceGroupImpl::CreateResourceFromResource( Resource* resource ) const
    {
		ResourceParams resourceParams;

        resourceParams.relativePath = resource->GetRelativePath().GetValue().ToString();

        resourceParams.location = resource->GetLocation().GetValue();

        resourceParams.checksum = resource->GetChecksum().GetValue();

        resourceParams.compressedSize = resource->GetCompressedSize().GetValue();

        resourceParams.uncompressedSize = resource->GetUncompressedSize().GetValue();

        //This would need guarding against for binary compatability thought exercise TODO
        resourceParams.something = resource->GetSomething().GetValue();

		Resource* createdResource = new Resource( resourceParams );

        return createdResource;
    }

    //TODO not a good structure, Don't like returns like this
    //TODO Feels a bit strange that resourceGroup is thr one that creates this and not Resource
    Resource* ResourceGroupImpl::CreateResourceFromYaml( YAML::Node& resource )
	{
		Resource* createdResource = new Resource( ResourceParams{} );

		Result importFromYamlResult = createdResource->ImportFromYaml( resource, m_versionParameter.GetValue() );

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

    Result ResourceGroupImpl::ExportYamlToFile( const ResourceGroupExportToFileParams& params )
    {
		
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

			Result resourceExportResult = r->ExportToYaml( out, sanitisedOutputDocumentVersion );

            if( resourceExportResult != Result::SUCCESS )
			{
				return resourceExportResult;
			}

            out << YAML::EndMap;
			
        }

        out << YAML::EndSeq;

		out << YAML::EndMap;

        ResourcePutDataParams resourcePutDataParams;

        resourcePutDataParams.data = out.c_str();

        // Update parameters
		SetParametersFromData( resourcePutDataParams.data );    // TODO return value

        resourcePutDataParams.resourceDestinationSettings = params.resourceDetinationSettings;

        return PutData( resourcePutDataParams );
      
    }

    Result ResourceGroupImpl::CreatePatch( PatchCreateParams& params ) const
    {
		RelativePath p = GetRelativePath().GetValue();

        RelativePath patchPath( "patch", p.filename );

        params.patchResourceGroup->SetRelativePath( patchPath.ToString() );

        params.patchResourceGroup->SetResourceGroup( this );

        for (Resource* resource : m_resourcesParameter)
        {
            // Get resource data from
			ResourceGetDataParams resourceGetDataParamsFrom;

            resourceGetDataParamsFrom.resourceSourceSettings = params.resourceSourceSettingsFrom;
                
            Result fromResourceDataResult = resource->GetData( resourceGetDataParamsFrom );

            if (fromResourceDataResult != Result::SUCCESS)
            {
				return fromResourceDataResult;
            }

            // Get resource data to
			ResourceGetDataParams resourceGetDataParamsTo;

			resourceGetDataParamsTo.resourceSourceSettings = params.resourceSourceSettingsTo;

			Result toResourceDataResult = resource->GetData( resourceGetDataParamsTo );

			if( toResourceDataResult != Result::SUCCESS )
			{
				return toResourceDataResult;
			}

            // Create a patch from the data
			ResourcePutDataParams resourcePutDataParams;

            if( !ResourceTools::CreatePatch( resourceGetDataParamsFrom.data, resourceGetDataParamsTo.data, resourcePutDataParams.data) )
            {
				return Result::FAILED_TO_CREATE_PATCH;
            }

            // Create a resource from patch data
            // TODO prefixes for resource types should be formally defined somewhere
			std::string patchResourceName = "patch:/" + resource->GetRelativePath().GetValue().filename;

            Resource* patchResource = new Resource( { patchResourceName } );
			patchResource->SetParametersFromData( resourcePutDataParams.data );

            // Export patch file
            resourcePutDataParams.resourceDestinationSettings = params.resourceDestinationSettings;

			Result putPatchDataResult = patchResource->PutData( resourcePutDataParams );

            if (putPatchDataResult != Result::SUCCESS)
            {
				return putPatchDataResult;
            }

            // Add the patch resource to the patchResourceGroup
			params.patchResourceGroup->AddResource( patchResource );
        }

 
        return Result::SUCCESS;
    }


    Result ResourceGroupImpl::AddResource( Resource* resource )
    {
		m_resourcesParameter.PushBack( resource );

		return Result::SUCCESS;
    }

    Result ResourceGroupImpl::Subtraction( ResourceGroupSubtractionParams& params ) const
    {
		DocumentParameterCollection<Resource*> subtractionResources = params.subtractResourceGroup->m_impl->m_resourcesParameter;


        // Iterate through all resources
        for (Resource* resource : m_resourcesParameter)
        {
            // If resource is not in subtraction group then add to result
			if( !subtractionResources.Contains( resource ) )
            {
                // Create a copy of the resource
				Resource* resourceCopy = CreateResourceFromResource( resource );

				params.result->m_impl->AddResource( resourceCopy );
            }
            
        }

        return Result::SUCCESS;
    }

   

}