#include "ResourceGroupImpl.h"
#include "Resource.h"

#include <fstream>
#include <sstream>
#include <yaml-cpp/yaml.h>
#include <ResourceTools.h>

namespace CarbonResources
{
    

    ResourceGroup::ResourceGroupImpl::ResourceGroupImpl( )
    {

    }

    ResourceGroup::ResourceGroupImpl::~ResourceGroupImpl()
    {

    }

    Result ResourceGroup::ResourceGroupImpl::ImportFromFile( const ResourceGroupImportFromFileParams& params )
    {
        // VERSION NEEDS TO BE CHECKED TO ENSURE ITS SUPPORTED ON IMPORT

        if (params.inputFilename.find(".txt") != std::string::npos)
        {
			return ImportFromCSVFile( params );
        }
        else if (params.inputFilename.find(".yml") != std::string::npos)
        {
			return ImportFromYamlFile( params );
        }
		else if( params.inputFilename.find( ".yaml" ) != std::string::npos )
		{
			return ImportFromYamlFile( params );
		}
        else
        {
			return Result::UNSUPPORTED_FILE_FORMAT;
        }
    }

    Result ResourceGroup::ResourceGroupImpl::ExportToFile( const ResourceGroupExportToFileParams& params )
    {
		return ExportYamlToFile( params );
    }

    Result ResourceGroup::ResourceGroupImpl::ImportFromCSVFile( const ResourceGroupImportFromFileParams& params )
    {
		std::ifstream inputStream;

		inputStream.open( params.inputFilename, std::ios::in );

		if( !inputStream )
		{
			return Result::FAILED_TO_OPEN_FILE;
		}

		std::string stringIn;

		while( !inputStream.eof() )
		{
			std::getline( inputStream, stringIn );

            std::stringstream ss(stringIn);

            std::string value;

            char delimiter = ',';

            CarbonResources::ResourceParams resourceParams;

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
			CarbonResources::Resource* resource = new CarbonResources::Resource( resourceParams );

            m_resourcesParameter.PushBack( resource );
            //m_resources.push_back( resource );
		}

        inputStream.close();

		return Result::SUCCESS;
    }

    Resource* ResourceGroup::ResourceGroupImpl::CreateResourceFromYaml( YAML::Node& resource )
	{
		CarbonResources::ResourceParams resourceParams;

		resourceParams.ImportFromYaml( resource, m_versionParameter.GetValue() );

        return new Resource( resourceParams );
	}

    Result ResourceGroup::ResourceGroupImpl::ImportFromYamlFile( const ResourceGroupImportFromFileParams& params )
    {
		// TODO Handle file not found gracefully
		YAML::Node resourceGroupFile = YAML::LoadFile( params.inputFilename );

        
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

    std::string ResourceGroup::ResourceGroupImpl::Type() const
    {
		return "ResourceGroup";
    }

    Result ResourceGroup::ResourceGroupImpl::ImportGroupSpecialisedYaml( YAML::Node& resourceGroupFile )
    {
        return Result::SUCCESS;
    }

    Result ResourceGroup::ResourceGroupImpl::ExportGroupSpecialisedYaml( YAML::Emitter& out, Version outputDocumentVersion ) const
    {
		return Result::SUCCESS;
    } 

    Result ResourceGroup::ResourceGroupImpl::ExportYamlToFile( const ResourceGroupExportToFileParams& params ) const
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

			Result resourceExportResult = r->ExportToYaml( out, sanitisedOutputDocumentVersion );

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



    Result ResourceGroup::ResourceGroupImpl::CreatePatch( const PatchCreateParams& params ) const
    {
        // TODO currently wip, working it through currently

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
				//std::string latestResourceData = "";
				//latestResource->GetData( params.basePath, latestResourceData );

                //std::string previousResourceData = "";
				//previousResource->GetData( params.basePath, previousResourceData );

                //std::string patchData = "";

				//ResourceTools::CreatePatch( previousResourceData, latestResourceData, patchData );

                //TODO there are lots of returns that are not being checked here, this all needs tightening up

                // Now save the patch file
                // Create an entry in the patch resource list

            }

        }

        // ResourceGroup needs to be referenced by the patchGroup

        return Result::FAIL;
    }

    

}