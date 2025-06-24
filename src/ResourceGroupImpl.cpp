#include "ResourceGroupImpl.h"

#include <fstream>
#include <sstream>
#include <yaml-cpp/yaml.h>
#include <ResourceTools.h>
#include <BundleStreamOut.h>
#include <FileDataStreamIn.h>
#include <ScopedFile.h>
#include <Md5ChecksumStream.h>
#include <GzipCompressionStream.h>
#include "ResourceInfo/PatchResourceGroupInfo.h"
#include "ResourceInfo/BundleResourceGroupInfo.h"
#include "ResourceInfo/ResourceGroupInfo.h"
#include "ResourceInfo/PatchResourceInfo.h"
#include "Patching.h"
#include "PatchResourceGroupImpl.h"
#include "BundleResourceGroupImpl.h"
#include "ChunkIndex.h"

namespace CarbonResources
{
    

    ResourceGroupImpl::ResourceGroupImpl()
    {
		m_versionParameter = VersionInternal(S_DOCUMENT_VERSION);

		m_type = TypeId();

        m_numberOfResources = 0;

        m_totalResourcesSizeCompressed = 0;

        m_totalResourcesSizeUncompressed = 0;
    }

    ResourceGroupImpl::~ResourceGroupImpl()
    {
		m_resourcesParameter.Clear();
    }

    Result ResourceGroupImpl::CreateFromDirectory( const CreateResourceGroupFromDirectoryParams& params )
    {
		// Update status
		if( params.statusCallback )
		{
			params.statusCallback( CarbonResources::StatusLevel::PROCEDURE, CarbonResources::StatusProgressType::PERCENTAGE, 0, "Processing Files: " + params.directory.string() );
		}

        if (!std::filesystem::exists(params.directory))
        {
			return Result{ ResultType::INPUT_DIRECTORY_DOESNT_EXIST };
        }

        // Ensure document version is valid
        VersionInternal documentVersion( params.outputDocumentVersion );

        if (!documentVersion.isVersionValid())
        {
			return Result{ ResultType::DOCUMENT_VERSION_UNSUPPORTED };
        }

        // Walk directory and create a resource from each file using data
		auto recursiveDirectoryIter = std::filesystem::recursive_directory_iterator( params.directory );

        for (const std::filesystem::directory_entry& entry : recursiveDirectoryIter)
        {
            if (entry.is_regular_file())
            {
                // Update status
                if (params.statusCallback)
                {
					params.statusCallback( CarbonResources::StatusLevel::DETAIL, CarbonResources::StatusProgressType::UNBOUNDED, 0, "Processing File: " + entry.path().string() );
                }

                // Create resource
			    auto fileSize = entry.file_size();

                if( fileSize < params.resourceStreamThreshold )
                {
                    // Create resource from data
				    ResourceInfoParams resourceParams;
					
                    resourceParams.relativePath = std::filesystem::relative( entry.path(), params.directory );

                	resourceParams.binaryOperation = ResourceTools::CalculateBinaryOperation( entry.path() );

                	resourceParams.prefix = params.resourcePrefix;

				    ResourceInfo* resource = new ResourceInfo( resourceParams );

                    std::string resourceData;

                    ResourceGetDataParams resourceGetDataParams;

                    resourceGetDataParams.resourceSourceSettings.basePaths = { params.directory };

                    resourceGetDataParams.resourceSourceSettings.sourceType = ResourceSourceType::LOCAL_RELATIVE;

                    resourceGetDataParams.data = &resourceData;

                    Result getResourceDataResult = resource->GetData( resourceGetDataParams );

                    if (getResourceDataResult.type != ResultType::SUCCESS)
                    {
					    return getResourceDataResult;
                    }

                    Result setParametersFromDataResult = resource->SetParametersFromData( resourceData );

                    if (setParametersFromDataResult.type != ResultType::SUCCESS)
                    {
					    return setParametersFromDataResult;
                    }

                    Result addResourceResult = AddResource( resource );

                    if (addResourceResult.type != ResultType::SUCCESS)
                    {
					    return addResourceResult;
                    }
                }
                else
                {
                    // Process data via stream
                    ResourceTools::Md5ChecksumStream checksumStream;
                	std::string compressedData;

                    ResourceTools::GzipCompressionStream gzipCompressionStream( &compressedData );

                    ResourceTools::FileDataStreamIn fileStreamIn( params.resourceStreamThreshold );

                    if (!gzipCompressionStream.Start())
                    {
						return Result{ ResultType::FAILED_TO_COMPRESS_DATA };
                    }

                    if (!fileStreamIn.StartRead(entry.path()))
                    {
						return Result{ ResultType::FAILED_TO_OPEN_FILE_STREAM };
                    }

                    uintmax_t compressedDataSize = 0;

                    while (!fileStreamIn.IsFinished())
                    {
						// Update status
						if( params.statusCallback )
						{
							auto percentage = static_cast<unsigned int>( ( 100 * fileStreamIn.GetCurrentPosition() ) / fileStreamIn.Size() );
							params.statusCallback( CarbonResources::StatusLevel::DETAIL, CarbonResources::StatusProgressType::PERCENTAGE, percentage, "Percentage Update" );
						}

						std::string fileData;

                        if (!(fileStreamIn >> fileData))
                        {
							return Result{ ResultType::FAILED_TO_READ_FROM_STREAM };
                        }

                        if (!(checksumStream << fileData))
                        {
							return Result{ ResultType::FAILED_TO_GENERATE_CHECKSUM };
                        }

                        if( !( gzipCompressionStream << &fileData ) )
                        {
							return Result{ ResultType::FAILED_TO_COMPRESS_DATA };
                        }

                        compressedDataSize += compressedData.size();
                    	compressedData.clear();
                    }

                    if (!gzipCompressionStream.Finish())
                    {
						return Result{ ResultType::FAILED_TO_COMPRESS_DATA };
                    }

                	compressedDataSize += compressedData.size();
                	compressedData.clear();

                    std::string checksum;

                    if (!checksumStream.FinishAndRetrieve(checksum))
                    {
						return Result{ ResultType::FAILED_TO_GENERATE_CHECKSUM };
                    }

                    // Create resource from parameters
                    ResourceInfoParams resourceParams;

					resourceParams.relativePath = std::filesystem::relative( entry.path(), params.directory );

					resourceParams.uncompressedSize = fileSize;

                    resourceParams.compressedSize = compressedDataSize;

                    resourceParams.checksum = checksum;

                	resourceParams.binaryOperation = ResourceTools::CalculateBinaryOperation( entry.path() );

                    Location l;

					Result calculateLocationResult = l.SetFromRelativePathAndDataChecksum( resourceParams.relativePath, resourceParams.checksum );

                    if (calculateLocationResult.type != ResultType::SUCCESS)
                    {
						return calculateLocationResult;
                    }

                    resourceParams.location = l.ToString();

                    ResourceInfo* resource = new ResourceInfo( resourceParams );

                    Result addResourceResult = AddResource( resource );

					if( addResourceResult.type != ResultType::SUCCESS )
					{
						return addResourceResult;
					}
                    
                }
			}
        }

        if( params.statusCallback )
		{
			params.statusCallback( CarbonResources::StatusLevel::PROCEDURE, CarbonResources::StatusProgressType::PERCENTAGE, 100, "Resource group successfully created from directory" );
		}

        return Result{ ResultType::SUCCESS };

    }

    Result ResourceGroupImpl::ImportFromData( const std::string& data, DocumentType documentType /* = DocumentType::YAML */)
    {
        switch (documentType)
        {
		case DocumentType::CSV:
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4996 ) // Suppress deprecation warning.
#elif __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
			return ImportFromCSV( data );
#ifdef _MSC_VER
#pragma warning ( pop )
#elif __APPLE__
#pragma clang diagnostic pop
#endif
		case DocumentType::YAML:
			return ImportFromYamlString( data );
		default:
			return Result{ ResultType::UNSUPPORTED_FILE_FORMAT };
        }

        return Result{ ResultType::FAIL };
    }

    Result ResourceGroupImpl::ImportFromFile( const ResourceGroupImportFromFileParams& params )
    {
        // Status update
        if (params.statusCallback)
        {
			params.statusCallback( CarbonResources::StatusLevel::PROCEDURE, CarbonResources::StatusProgressType::PERCENTAGE, 0, "Importing Resource Group from file." );
        }

        if (params.filename.empty())
        {
			return Result{ ResultType::FILE_NOT_FOUND };
        }

        std::string data;

		if( !ResourceTools::GetLocalFileData( params.filename, data ) )
		{
			return Result{ ResultType::FAILED_TO_OPEN_FILE };
		}

        // VERSION NEEDS TO BE CHECKED TO ENSURE ITS SUPPORTED ON IMPORT
		std::filesystem::path filename = params.filename;

        std::string extension = filename.extension().string();
        
        Result importResult;

        if( extension == ".txt" )
        {
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4996 ) // Suppress deprecation warning.
#elif __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
			importResult = ImportFromCSV( data, params.statusCallback );
#ifdef _MSC_VER
#pragma warning( pop )
#elif __APPLE__
#pragma clang diagnostic pop
#endif
        }
		else if( extension == ".yml" || extension == ".yaml" || extension.empty() )
		{
			importResult = ImportFromYamlString( data, params.statusCallback );
		}
        else
        {
			return Result{ ResultType::UNSUPPORTED_FILE_FORMAT };
        }

        if( params.statusCallback )
		{
            if (importResult.type == ResultType::SUCCESS)
			{
				params.statusCallback( CarbonResources::StatusLevel::PROCEDURE, CarbonResources::StatusProgressType::PERCENTAGE, 100, "Successfully imported ResourceGroup" );
            }
		}

        return importResult;
    }

    Result ResourceGroupImpl::ExportToFile( const ResourceGroupExportToFileParams& params ) const
    {
		// Update status
		if( params.statusCallback )
		{
			params.statusCallback( CarbonResources::StatusLevel::PROCEDURE, CarbonResources::StatusProgressType::PERCENTAGE, 0, "Exporting Resource Group to file: " + params.filename.string() );
		}

		std::string data = "";

    	if( params.outputDocumentVersion.major == 0 && params.outputDocumentVersion.minor == 0 )
    	{
    		Result exportCsvResult = ExportCsv( params.outputDocumentVersion, data, params.statusCallback );
    		if( exportCsvResult.type != ResultType::SUCCESS )
    		{
    			return exportCsvResult;
    		}
    	}
    	else
    	{
    		Result exportYamlResult = ExportYaml( params.outputDocumentVersion, data, params.statusCallback );

    		if (exportYamlResult.type != ResultType::SUCCESS)
    		{
    			return exportYamlResult;
    		}
    	}

        if( !ResourceTools::SaveFile( params.filename, data ) )
		{
			return Result{ ResultType::FAILED_TO_SAVE_FILE };
		}

        if( params.statusCallback )
		{
			params.statusCallback( CarbonResources::StatusLevel::PROCEDURE, CarbonResources::StatusProgressType::PERCENTAGE, 100, "Resource group successfully exported." );
		}

		return Result{ ResultType::SUCCESS };
    }

    Result ResourceGroupImpl::ExportToData( std::string& data,  VersionInternal outputDocumentVersion /* = S_DOCUMENT_VERSION*/) const
    {
		Result exportYamlResult = ExportYaml( outputDocumentVersion, data );

		if( exportYamlResult.type != ResultType::SUCCESS )
		{
			return exportYamlResult;
		}

        return Result{ ResultType::SUCCESS };
    }

    Result ResourceGroupImpl::ImportFromCSV( const std::string& data, StatusCallback statusCallback /* = nullptr */ )
    {
		// Status update
		if( statusCallback )
		{
			statusCallback( CarbonResources::StatusLevel::PROCEDURE, CarbonResources::StatusProgressType::PERCENTAGE, 5, "Importing Resource Group from CSV file." );
		}

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
				return Result{ ResultType::MALFORMED_RESOURCE_INPUT };
            }

            // Split filename and prefix
			std::string resourcePrefixDelimiter = ":/";
			std::string filename = value.substr( value.find(resourcePrefixDelimiter) + resourcePrefixDelimiter.size() );
			std::string resourcePrefix = value.substr( 0, value.find( ":" ) );

			resourceParams.relativePath = filename;

			resourceParams.prefix = resourcePrefix;

			if( !std::getline( ss, value, delimiter ) )
			{
				return Result{ ResultType::MALFORMED_RESOURCE_INPUT };
			}

			resourceParams.location = value;

			if( !std::getline( ss, value, delimiter ) )
			{
				return Result{ ResultType::MALFORMED_RESOURCE_INPUT };
			}

			resourceParams.checksum = value;

			if( !std::getline( ss, value, delimiter ) )
			{
				return Result{ ResultType::MALFORMED_RESOURCE_INPUT };
			}

			resourceParams.uncompressedSize = atol( value.c_str() );

			if( !std::getline( ss, value, delimiter ) )
			{
				return Result{ ResultType::MALFORMED_RESOURCE_INPUT };
			}

			resourceParams.compressedSize = atol( value.c_str() );

			if( !std::getline( ss, value, delimiter ) )
			{
				resourceParams.binaryOperation = 0;
			}
			else
			{
				resourceParams.binaryOperation = atoi( value.c_str() );
			}

            // ResourceGroup gets upgraded to 0.1.0
			m_versionParameter = VersionInternal{ 0, 1, 0 };

			// Create a Resource
			ResourceInfo* resource = new ResourceInfo( resourceParams );

            Result addResourceResult = AddResource( resource );

            if (addResourceResult.type != ResultType::SUCCESS)
            {
				return addResourceResult;
            }

            if( statusCallback )
			{
				statusCallback( CarbonResources::StatusLevel::DETAIL, CarbonResources::StatusProgressType::UNBOUNDED, 0, "Imported resource: " + resourceParams.relativePath.string() );
			}
		}

		return Result{ ResultType::SUCCESS };
    }

    Result ResourceGroupImpl::CreateResourceFromResource( const ResourceInfo& resourceIn, ResourceInfo*& resourceOut ) const
    {
		resourceOut = nullptr;

		std::string resourceType = "";

		Result getResourceTypeResult = resourceIn.GetType( resourceType );

        if (getResourceTypeResult.type != ResultType::SUCCESS)
        {
			return getResourceTypeResult;
        }

        if (resourceType == ResourceInfo::TypeId())
        {
			resourceOut = new ResourceInfo( {} );

            Result setParametersFromResourceResult = resourceOut->SetParametersFromResource( &resourceIn, m_versionParameter.GetValue() );

            if (setParametersFromResourceResult.type != ResultType::SUCCESS)
            {
				return setParametersFromResourceResult;
            }
        }
        else if (resourceType == PatchResourceInfo::TypeId())
        {
			PatchResourceInfo* patchResourceInfo = new PatchResourceInfo( {} );

            Result setParametersFromResourceResult = patchResourceInfo->SetParametersFromResource( &resourceIn, m_versionParameter.GetValue() );

            if( setParametersFromResourceResult.type != ResultType::SUCCESS )
			{
				return setParametersFromResourceResult;
			}

            resourceOut = patchResourceInfo;
        }
		else if( resourceType == BundleResourceInfo::TypeId() )
		{
			BundleResourceInfo* bundleResourceInfo = new BundleResourceInfo( {} );

			Result setParametersFromResourceResult = bundleResourceInfo->SetParametersFromResource( &resourceIn, m_versionParameter.GetValue() );
            
            if( setParametersFromResourceResult.type != ResultType::SUCCESS )
			{
				return setParametersFromResourceResult;
			}

			resourceOut = bundleResourceInfo;
		}
		else if( resourceType == BundleResourceInfo::TypeId() )
		{
			ResourceInfo* binaryResourceInfo = new ResourceInfo( {} );

			Result setParametersFromResourceResult = binaryResourceInfo->SetParametersFromResource( &resourceIn, m_versionParameter.GetValue() );

            if( setParametersFromResourceResult.type != ResultType::SUCCESS )
			{
				return setParametersFromResourceResult;
			}

			resourceOut = binaryResourceInfo;
		}

        return Result{ ResultType::SUCCESS };

    }

	Result ResourceGroupImpl::CreateResourceFromYaml( YAML::Node& resource, ResourceInfo*& resourceOut )
	{
		resourceOut = new ResourceInfo( ResourceInfoParams{} );

		Result importFromYamlResult = resourceOut->ImportFromYaml( resource, m_versionParameter.GetValue() );

        if( importFromYamlResult.type != ResultType::SUCCESS )
		{
			delete resourceOut;

			resourceOut = nullptr;

			return importFromYamlResult;
		}
        else
        {
			return Result{ ResultType::SUCCESS };
        }

	}

    Result ResourceGroupImpl::ImportFromYamlString( const std::string& data, StatusCallback statusCallback /* = nullptr */ )
    {
	    YAML::Node resourceGroupFile;
    	try
    	{
    		resourceGroupFile = YAML::Load( data );
    	}
    	catch( YAML::ParserException& )
    	{
    		return Result{ ResultType::FAILED_TO_PARSE_YAML };
    	}
    	return ImportFromYaml( resourceGroupFile, statusCallback );
    }

	Result ResourceGroupImpl::ImportFromYaml( YAML::Node& resourceGroupFile, StatusCallback statusCallback )
	{
		YAML::Node typeNode = resourceGroupFile[m_type.GetTag()];
		if( !typeNode.IsDefined() )
		{
			return Result{ ResultType::MALFORMED_RESOURCE_GROUP };
		}
		m_type = typeNode.as<std::string>();
		if( m_type.GetValue() != GetType() )
		{
			return Result{ ResultType::FILE_TYPE_MISMATCH };
		}

    	YAML::Node resourceGroupVersionNode = resourceGroupFile[m_versionParameter.GetTag()];
    	if( !resourceGroupVersionNode.IsDefined() )
    	{
			return Result{ ResultType::MALFORMED_RESOURCE_GROUP };
    	}
        std::string versionStr = resourceGroupVersionNode.as<std::string>(); //version stringID needs to be in one place
		
        VersionInternal version;
		version.FromString( versionStr );
        m_versionParameter = version;

		if( m_versionParameter.GetValue().getMajor() > S_DOCUMENT_VERSION.major )
        {
			return Result{ ResultType::DOCUMENT_VERSION_UNSUPPORTED };
        }

        // If version is greater than the max version supported at compile then ceil to that
        if (version > S_DOCUMENT_VERSION)
        {
            //TODO there should perhaps be a warning that some data will be missed
			version = S_DOCUMENT_VERSION;
        }

		YAML::Node numberOfResourcesNode = resourceGroupFile[m_numberOfResources.GetTag()];
		if( !numberOfResourcesNode.IsDefined() )
		{
			return Result{ ResultType::MALFORMED_RESOURCE_GROUP };
		}

		YAML::Node totalResourceSizeCompressedNode = resourceGroupFile[m_totalResourcesSizeCompressed.GetTag()];
		if( !totalResourceSizeCompressedNode.IsDefined() )
		{
			return Result{ ResultType::MALFORMED_RESOURCE_GROUP };
		}

    	YAML::Node totalResourceSizeUncompressedNode = resourceGroupFile[m_totalResourcesSizeUncompressed.GetTag()];
		if( !totalResourceSizeUncompressedNode.IsDefined() )
		{
			return Result{ ResultType::MALFORMED_RESOURCE_GROUP };
		}

        Result res = ImportGroupSpecialisedYaml( resourceGroupFile );

        if( res.type != ResultType::SUCCESS )
		{
			return res;
		}

        YAML::Node resources = resourceGroupFile[m_resourcesParameter.GetTag()];
    	if( !resources.IsDefined() )
    	{
			return Result{ ResultType::MALFORMED_RESOURCE_GROUP };
    	}

        for (auto iter = resources.begin(); iter != resources.end(); iter++)
        {
            // This bit is a sequence
			YAML::Node resourceNode = (*iter);
            
            ResourceInfo* resource = nullptr;

            Result createResourceFromYamlResult = CreateResourceFromYaml( resourceNode, resource );

            if (createResourceFromYamlResult.type != ResultType::SUCCESS)
            {
				return createResourceFromYamlResult;
            }

            Result addResourceResult = AddResource( resource );

			if( addResourceResult.type != ResultType::SUCCESS )
			{
				return addResourceResult;
			}

        }

		return Result{ ResultType::SUCCESS };
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
		return Result{ ResultType::SUCCESS };
    }

    Result ResourceGroupImpl::ExportGroupSpecialisedYaml( YAML::Emitter& out, VersionInternal outputDocumentVersion ) const
    {
		return Result{ ResultType::SUCCESS };
    } 

    Result ResourceGroupImpl::ExportYaml( const VersionInternal& outputDocumentVersion, std::string& data, StatusCallback statusCallback /*= nullptr*/ ) const
    {
		
        YAML::Emitter out;

        // Output header information
		out << YAML::BeginMap;

        // It is possible to export a different version that the imported version
        // The version must be less than the version of the document and also no higher than supported by the binary at compile
		// Ensure document version is valid
		if( !outputDocumentVersion.isVersionValid() )
		{
			return Result{ ResultType::DOCUMENT_VERSION_UNSUPPORTED };
		}

		VersionInternal sanitisedOutputDocumentVersion = outputDocumentVersion;
		const VersionInternal documentCurrentVersion = m_versionParameter.GetValue();

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

        out << YAML::Key << m_numberOfResources.GetTag();
		out << YAML::Value << m_numberOfResources.GetValue();

        out << YAML::Key << m_totalResourcesSizeCompressed.GetTag();
		out << YAML::Value << m_totalResourcesSizeCompressed.GetValue();

        out << YAML::Key << m_totalResourcesSizeUncompressed.GetTag();
		out << YAML::Value << m_totalResourcesSizeUncompressed.GetValue();

        Result res = ExportGroupSpecialisedYaml( out, sanitisedOutputDocumentVersion );

		if( res.type != ResultType::SUCCESS )
        {
			return res;
        }

        out << YAML::Key << m_resourcesParameter.GetTag();

        out << YAML::Value << YAML::BeginSeq;

        int i = 0;

        for (ResourceInfo* r : m_resourcesParameter)
        {
			// Update status
			if( statusCallback )
			{
				std::filesystem::path relativePath;

				Result getRelativePathResult = r->GetRelativePath( relativePath );

                if (getRelativePathResult.type != ResultType::SUCCESS)
                {
					return Result{ ResultType::FAIL };
                }

				auto percentage = static_cast<unsigned int>( ( 100 * i ) / m_resourcesParameter.GetValue()->size() );

                std::string message = "Exporting: " + relativePath.string();

				statusCallback( CarbonResources::StatusLevel::DETAIL, CarbonResources::StatusProgressType::PERCENTAGE, percentage, message );

				i++;
			}

			out << YAML::BeginMap;

			Result resourceExportResult = r->ExportToYaml( out, sanitisedOutputDocumentVersion );

            if( resourceExportResult.type != ResultType::SUCCESS )
			{
				return resourceExportResult;
			}

            out << YAML::EndMap;
			
        }

        out << YAML::EndSeq;

		out << YAML::EndMap;

        data = out.c_str();

        return Result{ ResultType::SUCCESS };
      
    }

    Result ResourceGroupImpl::ExportCsv( const VersionInternal& outputDocumentVersion, std::string& data, StatusCallback statusCallback /*= nullptr*/ ) const
    {
    	if( outputDocumentVersion.getMajor() > 0 || outputDocumentVersion.getMinor() > 0 || outputDocumentVersion.getPatch() > 0 )
    	{
    		return Result{ ResultType::UNSUPPORTED_FILE_FORMAT };
    	}

    	std::string out;


    	auto resourceInfos = m_resourcesParameter.GetValue();
    	std::sort(
    		resourceInfos->begin(),
    		resourceInfos->end(),
    		[] (ResourceInfo* a, ResourceInfo* b) {
    			std::filesystem::path ap;
    			std::filesystem::path bp;
    			a->GetRelativePath( ap );
    			b->GetRelativePath( bp );
    			return ap < bp;
    		} );

    	int i{0};
		for (ResourceInfo* r : m_resourcesParameter)
        {
			// Update status
			if( statusCallback )
			{
				auto percentage = static_cast<unsigned int>( ( 100 * i ) / m_resourcesParameter.GetValue()->size() );
				statusCallback( CarbonResources::StatusLevel::DETAIL, CarbonResources::StatusProgressType::PERCENTAGE, percentage, "Percentage Update" );
				i++;
			}

			Result resourceExportResult = r->ExportToCsv( out, m_versionParameter.GetValue() );
            if( resourceExportResult.type != ResultType::SUCCESS )
			{
				return resourceExportResult;
			}
			data += out + "\n";
        }
    	return Result{ ResultType::SUCCESS };
    }

    Result ResourceGroupImpl::ProcessChunk( std::string& chunkData, const std::filesystem::path& chunkRelativePath, BundleResourceGroupImpl& bundleResourceGroup, const ResourceDestinationSettings& chunkDestinationSettings ) const
    {
		// Create resource from Patch Data
		BundleResourceInfo* chunkResource = new BundleResourceInfo( { chunkRelativePath } ); // TODO it feels strange that this is a BundleResource but it is referred to as a chunk

		chunkResource->SetParametersFromData( chunkData );

		// Export chunk file
		ResourcePutDataParams resourcePutDataParams;

		resourcePutDataParams.resourceDestinationSettings = chunkDestinationSettings;

		resourcePutDataParams.data = &chunkData;

		Result putChunkDataResult = chunkResource->PutData( resourcePutDataParams );

		if( putChunkDataResult.type != ResultType::SUCCESS )
		{
			delete chunkResource;

			return putChunkDataResult;
		}

		// Add the chunk resource to the bundleResourceGroup
		Result addResourceResult = bundleResourceGroup.AddResource( chunkResource );

		if( addResourceResult.type != ResultType::SUCCESS )
		{
			delete chunkResource;

			return addResourceResult;
		}

        return Result{ ResultType::SUCCESS };
    }


    // TODO - Create chunks based on compressed size
	/*
    * Currently the cache is built up with data (best uncompressed)
    * Then chunks are created from the uncompressed data
    * The chunks would then be compressed when saved for upload (external to this process)
    * We could compress the data before sending it to bundle, but
    * that would then incur double compression and so we don't get best compression
    * 
    * What we want to do is gather the cache as before using uncompressed data
    * but when creating the chunk it wants to compress the output first to see if
    * the resulting chunk is then over the chunk threshold, if not it should
    * add chunks to the resulting chunk and then compress both chunks of uncompressed data together
    * until the compressed size of the combined chunks matches or exceededs the target chunk size.
    * However, the return data from this should return the uncompressed data.
    * Which as stated before would be compressed before upload.
    * 
    * This would give chunks with a slight size variation.
    * It would join files with great compression ratios together but also split large files.
    * 
    * It will give better compression when multiple files are joined. 
    * eg. if a patchResourceGroup often contains lots of patch files with fantastic compression ratios
    * Chunking just based on uncompressed size would result in compressed chunks that are also tiny.
    * Chunking as above would join many patches into a single chunk that when compressed would likely create
    * A better compression ratio of the joined data and also a file size which is closer to the requested chunk size of the bundle.
    */
    Result ResourceGroupImpl::CreateBundle( const BundleCreateParams& params ) const
    {
		// Update status
		if( params.statusCallback )
		{
			params.statusCallback( CarbonResources::StatusLevel::PROCEDURE, CarbonResources::StatusProgressType::PERCENTAGE, 0, "Creating Bundle" );
		}

		uintmax_t numberOfChunks = 0;

        std::string chunkBaseName = params.resourceGroupRelativePath.filename().replace_extension().string();

		BundleResourceGroupImpl bundleResourceGroup;

        bundleResourceGroup.SetChunkSize( params.chunkSize );

		ResourceTools::BundleStreamOut bundleStream( params.chunkSize );


        // Update status
		if( params.statusCallback )
		{
			params.statusCallback( CarbonResources::StatusLevel::PROCEDURE, CarbonResources::StatusProgressType::PERCENTAGE, 5, "Generating Chunks" );
		}

        int i = 0;

		std::vector<ResourceInfo*> toBundle;

		std::copy( m_resourcesParameter.begin(), m_resourcesParameter.end(), std::back_inserter( toBundle ) );

		Result getGroupSpecificResourcesToBundleResult = GetGroupSpecificResourcesToBundle( toBundle );

		if( getGroupSpecificResourcesToBundleResult.type != ResultType::SUCCESS )
		{
			return getGroupSpecificResourcesToBundleResult;
		}

        // Loop through all resources and send data for chunking
		for ( ResourceInfo* resource : toBundle )
        {
			std::string location;

			Result getLocationResult = resource->GetLocation( location );

			if( getLocationResult.type != ResultType::SUCCESS )
			{
				return getLocationResult;
			}

			if( params.statusCallback )
			{
				std::filesystem::path relativePath;

				Result getRelativePathResult = resource->GetRelativePath( relativePath );

                if (getRelativePathResult.type != ResultType::SUCCESS)
                {
					return getRelativePathResult;
                }

				std::string message;

				if( location.empty() )
				{
					message = "No file to process: " + relativePath.string();
				}
				else
				{
					message = "Processing: " + relativePath.string();
				}

				auto percentComplete = static_cast<unsigned int>( ( 100 * i ) / toBundle.size() );

                i++;

				params.statusCallback( CarbonResources::StatusLevel::DETAIL, CarbonResources::StatusProgressType::PERCENTAGE, percentComplete, message );
			}
			if( location.empty() )
			{
				continue;
			}

            ResourceTools::FileDataStreamIn resourceDataStream(params.fileReadChunkSize);

            ResourceGetDataStreamParams resourceGetDataParams;

            resourceGetDataParams.resourceSourceSettings = params.resourceSourceSettings;

            resourceGetDataParams.dataStream = &resourceDataStream;

        	resourceGetDataParams.downloadRetrySeconds = params.downloadRetrySeconds;

			Result resourceGetDataResult = resource->GetDataStream( resourceGetDataParams );

            if (resourceGetDataResult.type != ResultType::SUCCESS)
            {
				return resourceGetDataResult;
            }
			
            while (!resourceDataStream.IsFinished() )
            {
				std::string resourceDataChunk;

                if (!(resourceDataStream >> resourceDataChunk))
                {
					return Result{ ResultType::FAILED_TO_READ_FROM_STREAM };
                }

                // Add Resource chunk to bundle stream
				bundleStream << resourceDataChunk;

                // Loop through possible created chunks
				std::string chunkData;

				ResourceTools::GetChunk chunkFile;

				chunkFile.data = &chunkData;

				chunkFile.clearCache = false;

                while( bundleStream >> chunkFile )
				{
					std::stringstream ss;
					ss << chunkBaseName << numberOfChunks << ".chunk";
					std::string chunkName = ss.str();

					std::filesystem::path chunkPath = params.chunkDestinationSettings.basePath / ss.str();

                    if( params.statusCallback )
					{
						std::stringstream ss;

						ss << "Generating Chunk: " << chunkPath;

						params.statusCallback( CarbonResources::StatusLevel::DETAIL, CarbonResources::StatusProgressType::UNBOUNDED, 0, ss.str() );
					}

					Result processChunkResult = ProcessChunk( chunkData, chunkPath, bundleResourceGroup, params.chunkDestinationSettings );

					if( processChunkResult.type != ResultType::SUCCESS )
					{
						return processChunkResult;
					}

					numberOfChunks++;
				}
            }

        }

        
        // Create final incomplete chunk
		std::string chunkData;

		ResourceTools::GetChunk chunkFile;

		chunkFile.data = &chunkData;

		chunkFile.clearCache = true;

		bundleStream >> chunkFile;

		std::stringstream ss;
		ss << chunkBaseName << numberOfChunks << ".chunk";
		std::string chunkName = ss.str();

		std::filesystem::path chunkPath = params.chunkDestinationSettings.basePath / ss.str();

		Result processChunkResult = ProcessChunk( chunkData, chunkPath, bundleResourceGroup, params.chunkDestinationSettings );

		if( processChunkResult.type != ResultType::SUCCESS )
		{
			return processChunkResult;
		}

		// Export this resource list
        // 
		// Update status
		if( params.statusCallback )
		{
			params.statusCallback( CarbonResources::StatusLevel::PROCEDURE, CarbonResources::StatusProgressType::PERCENTAGE, 75, "Exporting ResourceGroups" );
		}

		std::string resourceGroupData;

		Result exportToDataResult = ExportToData( resourceGroupData );

        if (exportToDataResult.type != ResultType::SUCCESS)
        {
			return exportToDataResult;
        }

		ResourceGroupInfo resourceGroupInfo( { params.resourceGroupRelativePath } );

		Result setParametersFromDataResult = resourceGroupInfo.SetParametersFromData( resourceGroupData );

        if (setParametersFromDataResult.type != ResultType::SUCCESS)
        {
			return setParametersFromDataResult;
        }

		ResourcePutDataParams putDataParams;

		putDataParams.resourceDestinationSettings = params.chunkDestinationSettings; // TODO the resource list is going where the chunks are, perhaps this is missleading

		putDataParams.data = &resourceGroupData;

		Result subtractionResourcePutResult = resourceGroupInfo.PutData( putDataParams );

		if( subtractionResourcePutResult.type != ResultType::SUCCESS )
		{
			return subtractionResourcePutResult;
		}

		// Export the bundleGroup
		Result setResourceGroupResult = bundleResourceGroup.SetResourceGroup( resourceGroupInfo );

        if (setResourceGroupResult.type != ResultType::SUCCESS)
        {
			return setResourceGroupResult;
        }

		std::string patchResourceGroupData;

		Result exportBundleResourceGroupToDataResult = bundleResourceGroup.ExportToData( patchResourceGroupData );

        if (exportBundleResourceGroupToDataResult.type != ResultType::SUCCESS)
        {
			return exportBundleResourceGroupToDataResult;
        }

		BundleResourceGroupInfo patchResourceGroupInfo( { params.resourceGroupBundleRelativePath } );

		Result setPatchParametersFromDataResult = patchResourceGroupInfo.SetParametersFromData( patchResourceGroupData );

        if (setPatchParametersFromDataResult.type != ResultType::SUCCESS)
        {
			return setPatchParametersFromDataResult;
        }

		ResourcePutDataParams bundlePutDataParams;

		bundlePutDataParams.resourceDestinationSettings = params.resourceBundleResourceGroupDestinationSettings;

		bundlePutDataParams.data = &patchResourceGroupData;

		Result patchResourceGroupPutResult = patchResourceGroupInfo.PutData( bundlePutDataParams );

		if( patchResourceGroupPutResult.type != ResultType::SUCCESS )
		{
			return patchResourceGroupPutResult;
		}

        // Update status
		if( params.statusCallback )
		{
			params.statusCallback( CarbonResources::StatusLevel::PROCEDURE, CarbonResources::StatusProgressType::PERCENTAGE, 100, "Bundle Creation Complete." );
		}

        return Result{ ResultType::SUCCESS };
    }

	Result ResourceGroupImpl::ConstructPatchResourceInfo( const PatchCreateParams& params, int patchId, uintmax_t dataOffset, uint64_t patchSourceOffset, ResourceInfo* resourceNext, PatchResourceInfo*& patchResource ) const
	{
    	// Create a resource from patch data
    	std::filesystem::path resourceLatestRelativePath;
    	Result getResourceLatestRelativePathResult = resourceNext->GetRelativePath( resourceLatestRelativePath );
    	if( getResourceLatestRelativePathResult.type != ResultType::SUCCESS )
    	{
    		return getResourceLatestRelativePathResult;
    	}

		PatchResourceInfoParams patchResourceInfoParams;
		std::string patchFilename = params.patchFileRelativePathPrefix.string() + "." + std::to_string( patchId ); // TODO odd
		patchResourceInfoParams.relativePath = patchFilename;
		patchResourceInfoParams.targetResourceRelativePath = resourceLatestRelativePath;
		patchResourceInfoParams.dataOffset = dataOffset;
		patchResourceInfoParams.sourceOffset = patchSourceOffset;
		patchResource = new PatchResourceInfo( patchResourceInfoParams );

    	return Result{ ResultType::SUCCESS };
	}

	Result ResourceGroupImpl::CreatePatch( const PatchCreateParams& params ) const
    {
        // Update status
		if( params.statusCallback )
		{
			params.statusCallback( CarbonResources::StatusLevel::PROCEDURE, CarbonResources::StatusProgressType::PERCENTAGE, 0, "Creating Patch" );
		}

        if (params.previousResourceGroup->m_impl->GetType() != GetType())
        {
			return Result{ ResultType::PATCH_RESOURCE_LIST_MISSMATCH };
        }

        PatchResourceGroupImpl patchResourceGroup;

        patchResourceGroup.SetMaxInputChunkSize( params.maxInputFileChunkSize );

        // Subtraction //TODO this needs to match the format of the original input resource lists
        // Put in place when there is a factory
		ResourceGroupImpl resourceGroupSubtractionPrevious;

        ResourceGroupImpl resourceGroupSubtractionLatest;

        ResourceGroupSubtractionParams resourceGroupSubtractionParams;

		resourceGroupSubtractionParams.subtractResourceGroup = params.previousResourceGroup->m_impl;

		resourceGroupSubtractionParams.result1 = &resourceGroupSubtractionPrevious;

        resourceGroupSubtractionParams.result2 = &resourceGroupSubtractionLatest;

        resourceGroupSubtractionParams.statusCallback = params.statusCallback;

        // Update status
		if( params.statusCallback )
		{
			params.statusCallback( CarbonResources::StatusLevel::PROCEDURE, CarbonResources::StatusProgressType::PERCENTAGE, 20, "Calculaing resourceGroups delta." );
		}

        Result subtractionResult = Diff( resourceGroupSubtractionParams );

        if (subtractionResult.type != ResultType::SUCCESS)
        {
			return subtractionResult;
        }

        // Ensure that the diff results have the same number of members
        if (resourceGroupSubtractionPrevious.m_resourcesParameter.GetSize() != resourceGroupSubtractionLatest.m_resourcesParameter.GetSize())
        {
			return Result{ ResultType::UNEXPECTED_PATCH_DIFF_ENCOUNTERED };
        }

        int patchId = 0;

        // Update status
		if( params.statusCallback )
		{
			params.statusCallback( CarbonResources::StatusLevel::PROCEDURE, CarbonResources::StatusProgressType::PERCENTAGE, 40, "Generating Patches" );
		}

        for (int i = 0; i < resourceGroupSubtractionLatest.m_resourcesParameter.GetSize(); i++)
        {
			
			ResourceInfo* resourcePrevious = resourceGroupSubtractionPrevious.m_resourcesParameter.At( i );

			ResourceInfo* resourceNext = resourceGroupSubtractionLatest.m_resourcesParameter.At( i );

            if( params.statusCallback )
			{
				auto percentageComplete = static_cast<unsigned int>( ( 100 * i ) / resourceGroupSubtractionLatest.m_resourcesParameter.GetSize() );

                std::filesystem::path relativePath;

                Result getRelativePathResult = resourcePrevious->GetRelativePath( relativePath );

                if (getRelativePathResult.type != ResultType::SUCCESS)
                {
					return getRelativePathResult;
                }

                std::string message = "Creating patch for: " + relativePath.string();

				params.statusCallback( CarbonResources::StatusLevel::DETAIL, CarbonResources::StatusProgressType::PERCENTAGE, percentageComplete, message );
			}

        	size_t patchSourceOffset{0};
        	uint64_t patchSourceOffsetDelta{0};

            // Check to see if previous entry contains dummy information
            // Suggesting that this is a new entry in latest
            // In which case there is no reason to create a patch
            // The new entry will be stored with the ResourceGroup related to the PatchResourceGroup
			uintmax_t previousUncompressedSize;

            Result getResourcePreviousCompressedSizeResult = resourcePrevious->GetUncompressedSize( previousUncompressedSize );

            if (getResourcePreviousCompressedSizeResult.type != ResultType::SUCCESS)
            {
				return getResourcePreviousCompressedSizeResult;
            }



            uintmax_t nextUncompressedSize;

			Result getResourceNextCompressedSizeResult = resourceNext->GetUncompressedSize( nextUncompressedSize );

			if( getResourceNextCompressedSizeResult.type != ResultType::SUCCESS )
			{
				return getResourceNextCompressedSizeResult;
			}


            if( previousUncompressedSize != 0 ) // TODO make a note of what is going on here, it is confusing
            {
				// Get resource data previous
				ResourceTools::FileDataStreamIn previousFileDataStream( params.maxInputFileChunkSize );

				ResourceGetDataStreamParams previousResourceGetDataStreamParams;

				previousResourceGetDataStreamParams.resourceSourceSettings = params.resourceSourceSettingsPrevious;

            	previousResourceGetDataStreamParams.downloadRetrySeconds = params.downloadRetrySeconds;

				previousResourceGetDataStreamParams.dataStream = &previousFileDataStream;

                Result getPreviousDataStreamResult = resourcePrevious->GetDataStream( previousResourceGetDataStreamParams );

                if (getPreviousDataStreamResult.type != ResultType::SUCCESS)
                {
					return getPreviousDataStreamResult;
                }

                // Get resource data next
				ResourceTools::FileDataStreamIn nextFileDataStream( params.maxInputFileChunkSize );

				ResourceGetDataStreamParams nextResourceGetDataStreamParams;

				nextResourceGetDataStreamParams.resourceSourceSettings = params.resourceSourceSettingsNext;

				nextResourceGetDataStreamParams.dataStream = &nextFileDataStream;

                Result getNextDataStreamResult = resourceNext->GetDataStream( nextResourceGetDataStreamParams );

				if( getNextDataStreamResult.type != ResultType::SUCCESS )
				{
					return getNextDataStreamResult;
				}

            	std::filesystem::path relativePath;
				Result getRelativePathResult = resourcePrevious->GetRelativePath( relativePath );
				if (getRelativePathResult.type != ResultType::SUCCESS)
				{
					return getRelativePathResult;
            	}

            	std::function<void(unsigned int, const std::string&)> callback = [params](unsigned int percent, const std::string& msg) {
            		if( params.statusCallback )
            		{
            			params.statusCallback( StatusLevel::DETAIL, StatusProgressType::PERCENTAGE, percent, msg );
            		}
            	};
        		ResourceTools::ChunkIndex index(previousFileDataStream.GetPath(), params.maxInputFileChunkSize, params.indexFolder, callback );
        		if( params.statusCallback )
        		{
        			std::string message = "Generating index for " + relativePath.string();
        			params.statusCallback( StatusLevel::DETAIL , StatusProgressType::PERCENTAGE, 0, message );
        		}
            	index.GenerateChecksumFilter( nextFileDataStream.GetPath() );
        		if( !index.Generate() )
        		{
					std::string message = "Index generation failed for " + relativePath.string();
					params.statusCallback(StatusLevel::DETAIL, StatusProgressType::PERCENTAGE, 0, message);
        		}

                // Process one chunk at a time
				for( uintmax_t dataOffset = 0; dataOffset < nextUncompressedSize; dataOffset += params.maxInputFileChunkSize )
                {
					std::string previousFileData = "";

					if( previousFileDataStream.IsFinished() )
					{
						if( previousFileDataStream.Size() > nextFileDataStream.GetCurrentPosition() )
						{
							// We ran out of data because we found a chunk match later in the file,
							// but we can rewind back to where the read stream is in hopes
							// of getting a good diff, rather than just treating it as new data.
							previousFileDataStream.StartRead( previousFileDataStream.GetPath() );
						}
					}

                    // Handling if previous file is smaller than next file
                    // If so then previousFileData will be nothing and
                    // All next data will be used for the patch
                    if (!previousFileDataStream.IsFinished())
                    {
						if( !( previousFileDataStream >> previousFileData ) )
						{
							return Result{ ResultType::FAILED_TO_RETRIEVE_CHUNK_DATA };
						}
                    }

					size_t nextStreamPosition = nextFileDataStream.GetCurrentPosition();
                    // Note: in the case that the next file is smaller than previous
                    // nothing is stored, application of the patch will chop off the extra file data
                    std::string nextFileData;

					if(!nextFileDataStream.IsFinished())
					{
						if(!(nextFileDataStream >> nextFileData))
						{
							return Result{ ResultType::FAILED_TO_RETRIEVE_CHUNK_DATA };
						}
					}

                    // Create a patch
					// Create a patch from the data
					std::string patchData;

					bool chunkMatchFound{false};
					size_t matchCount{0};


                    if (previousFileData != "")
					{
                    	// Here's how this should work:
                    	// We find a matching chunk if it exists. If the chunk exists we make a patch with no data, because we'll get
                    	// the data from the source file using the patch info. Consecutive patches should be collapsed into one big one.
                    	// If we can't find a matching chunk, we will base the current diff off the chunk in the source starting after the final byte
                    	// in the chunk from the source file that we last used.
                    	// These should keep our patches pretty minimal, even if lots of data gets added early in the file causing offsets.
                    	// It should also handle small changes in moved parts of the file pretty well.
                    	if( params.statusCallback )
                    	{
                    		unsigned int progress = static_cast<uint32_t>( ( dataOffset * 100 ) / nextUncompressedSize );
                    		std::stringstream ss;
                    		ss << "Generating patch files: " << relativePath.string();
                    		params.statusCallback( StatusLevel::DETAIL, StatusProgressType::PERCENTAGE, progress, ss.str() );
                    	}

                    	chunkMatchFound = index.FindMatchingChunk( nextFileData, patchSourceOffset );

                    	if( chunkMatchFound )
                    	{
                    		matchCount = 1;
                    		matchCount += ResourceTools::CountMatchingChunks(
                    			nextFileDataStream.GetPath(),
                    			nextFileDataStream.GetCurrentPosition(),
                    			previousFileDataStream.GetPath(),
                    			patchSourceOffset + params.maxInputFileChunkSize,
                    			params.maxInputFileChunkSize );

                    		size_t matchSize = std::min(params.maxInputFileChunkSize * matchCount, previousFileDataStream.Size() - patchSourceOffset);

                    		PatchResourceInfo* patchResource{nullptr};
                    		ConstructPatchResourceInfo( params, patchId, dataOffset, patchSourceOffset, resourceNext, patchResource );
                    		if( previousFileDataStream.IsFinished() )
                    		{
                    			previousFileDataStream.StartRead( previousFileDataStream.GetPath() );
                    		}
                    		previousFileDataStream.Seek( patchSourceOffset );
                    		patchResource->SetParametersFromSourceStream( previousFileDataStream, matchSize );

                    		// Advance the first stream by the size of the matching data,
                    		// but move the point we generate patches from for the previous
                    		// file data stream to the end of the match.
                    		// It's hard to tell if it would be smarter to simply advance
                    		// the destination data by the same amount of the source data,
                    		// or perhaps even not to move it at all.
                    		nextFileDataStream.Seek( std::min( nextFileDataStream.Size(), nextStreamPosition + matchSize ) );
                    		previousFileDataStream.Seek( std::min(previousFileDataStream.Size(), patchSourceOffset + matchSize ) );
                    		dataOffset += matchSize - params.maxInputFileChunkSize;
                    		patchSourceOffset += matchSize;

                    		if( nextStreamPosition == 0 && patchSourceOffset == 0 )
                    		{
                    			// This is the beginning of the file and it matches.
                    			// There is no need to write patch data.
                    			continue;
                    		}

                    		// Add the patch resource to the patchResourceGroup
                    		Result addResourceResult = patchResourceGroup.AddResource( patchResource );

                    		if( addResourceResult.type != ResultType::SUCCESS )
                    		{
                    			delete patchResource;

                    			return addResourceResult;
                    		}

                    		patchId++;

                    		continue;
                    	}
						else
						{
							// Previous and next data chunk are different, create a patch
							if( !ResourceTools::CreatePatch( previousFileData, nextFileData, patchData ) )
							{
								return Result{ ResultType::FAILED_TO_CREATE_PATCH };
							}
							patchSourceOffsetDelta = previousFileData.size();
						}
                    }
                    else
                    {
                        // If there is no previous data then just store the data straight from the file
                        // All this data is new
                    	if( !ResourceTools::CreatePatch( "", nextFileData, patchData ) )
                    	{
							return Result{ ResultType::FAILED_TO_CREATE_PATCH };
                    	}
                    	patchSourceOffsetDelta = nextFileData.size();
                    }
					

                    //TODO handle the case where previousFileData and nextFileData match, in this case don't create a patch




					PatchResourceInfo* patchResource{nullptr};
					ConstructPatchResourceInfo( params, patchId, dataOffset, patchSourceOffset, resourceNext, patchResource );
					patchSourceOffset += patchSourceOffsetDelta;
					if( !patchData.empty() )
					{
						Result setParametersFromDataResult = patchResource->SetParametersFromData( patchData );

						if( setParametersFromDataResult.type != ResultType::SUCCESS )
						{
							return setParametersFromDataResult;
						}

						// Export patch file
						ResourcePutDataParams resourcePutDataParams;

						resourcePutDataParams.resourceDestinationSettings = params.resourcePatchBinaryDestinationSettings;

						resourcePutDataParams.data = &patchData;

						Result putPatchDataResult = patchResource->PutData( resourcePutDataParams );

						if( putPatchDataResult.type != ResultType::SUCCESS )
						{
							delete patchResource;

							return putPatchDataResult;
						}
					}

					// Add the patch resource to the patchResourceGroup
					Result addResourceResult = patchResourceGroup.AddResource( patchResource );

					if( addResourceResult.type != ResultType::SUCCESS )
					{
						delete patchResource;

						return addResourceResult;
					}

                    patchId++;


                }

            }

        }

    	patchResourceGroup.SetRemovedResourceRelativePaths( resourceGroupSubtractionParams.removedResources );

        // Update status
		if( params.statusCallback )
		{
			params.statusCallback( CarbonResources::StatusLevel::PROCEDURE, CarbonResources::StatusProgressType::PERCENTAGE, 60, "Exporting ResourceGroups." );
		}

        // Export the subtraction ResourceGroup
        std::string resourceGroupData;

        Result exportResourceGroupSubtractionLatestResult = resourceGroupSubtractionLatest.ExportToData( resourceGroupData );

        if (exportResourceGroupSubtractionLatestResult.type != ResultType::SUCCESS)
        {
			return exportResourceGroupSubtractionLatestResult;
        }

		ResourceGroupInfo subtractionResourceGroupInfo( { params.resourceGroupRelativePath } );

        Result setParametersFromDataResult = subtractionResourceGroupInfo.SetParametersFromData( resourceGroupData );

        if (setParametersFromDataResult.type != ResultType::SUCCESS)
        {
			return setParametersFromDataResult;
        }

        ResourcePutDataParams putDataParams;

        putDataParams.resourceDestinationSettings = params.resourcePatchBinaryDestinationSettings;

        putDataParams.data = &resourceGroupData; 

        Result subtractionResourcePutResult = subtractionResourceGroupInfo.PutData( putDataParams );

        if (subtractionResourcePutResult.type != ResultType::SUCCESS)
        {
			return subtractionResourcePutResult;
        }

      

        // Export the patchGroup
		Result setResourceGroupResult = patchResourceGroup.SetResourceGroup( subtractionResourceGroupInfo );

        if (setResourceGroupResult.type != ResultType::SUCCESS)
        {
			return setResourceGroupResult;
        }

        std::string patchResourceGroupData;

        Result exportToDataResult = patchResourceGroup.ExportToData( patchResourceGroupData );

        if (exportToDataResult.type != ResultType::SUCCESS)
        {
			return exportToDataResult;
        }

		PatchResourceGroupInfo patchResourceGroupInfo( { params.resourceGroupPatchRelativePath } );

        Result setPatchParametersFromDataResult = patchResourceGroupInfo.SetParametersFromData( patchResourceGroupData );

        if (setPatchParametersFromDataResult.type != ResultType::SUCCESS)
        {
			return setPatchParametersFromDataResult;
        }

        ResourcePutDataParams patchPutDataParams;

		patchPutDataParams.resourceDestinationSettings = params.resourcePatchResourceGroupDestinationSettings;

		patchPutDataParams.data = &patchResourceGroupData;

		Result patchResourceGroupPutResult = patchResourceGroupInfo.PutData( patchPutDataParams );

        if( patchResourceGroupPutResult.type != ResultType::SUCCESS )
		{
			return patchResourceGroupPutResult;
		}

        // Update status
		if( params.statusCallback )
		{
			params.statusCallback( CarbonResources::StatusLevel::PROCEDURE, CarbonResources::StatusProgressType::PERCENTAGE, 100, "Patch Created" );
		}

        return Result{ ResultType::SUCCESS };
    }


    Result ResourceGroupImpl::AddResource( ResourceInfo* resource )
    {
		m_resourcesParameter.PushBack( resource );

        m_numberOfResources = m_numberOfResources.GetValue() + 1;

        uintmax_t resourceUncompressedSize;
            
        Result resourceGetUncompressedSizeResult = resource->GetUncompressedSize( resourceUncompressedSize );

        if( resourceGetUncompressedSizeResult.type != ResultType::SUCCESS )
        {
			return resourceGetUncompressedSizeResult;
        }

        m_totalResourcesSizeUncompressed = m_totalResourcesSizeUncompressed.GetValue() + resourceUncompressedSize;

        uintmax_t resourceCompressedSize;

		Result resourceGetCompressedSizeResult = resource->GetCompressedSize( resourceCompressedSize );

		if( resourceGetCompressedSizeResult.type != ResultType::SUCCESS )
		{
			return resourceGetCompressedSizeResult;
		}

		m_totalResourcesSizeCompressed = m_totalResourcesSizeCompressed.GetValue() + resourceCompressedSize;

		return Result{ ResultType::SUCCESS };
    }

    Result PatchResourceGroupImpl::SetRemovedResourceRelativePaths( const std::vector<std::filesystem::path>& paths )
    {
    	for( auto path : paths )
    	{
    		m_removedResources.PushBack( path );
    	}
    	return Result{ ResultType::SUCCESS };
    }

    Result ResourceGroupImpl::Diff( ResourceGroupSubtractionParams& params ) const
    {
		if( params.statusCallback )
		{
			params.statusCallback( CarbonResources::StatusLevel::DETAIL, CarbonResources::StatusProgressType::PERCENTAGE, 0, "Calculating diff between two resource groups." );
		}

		DocumentParameterCollection<ResourceInfo*> subtractionResources = params.subtractResourceGroup->m_resourcesParameter;
        // Iterate through all resources

        // Value only required for status updates
		int i = 0;
    	std::vector<ResourceInfo*> sortedResourcesParameter( m_resourcesParameter.begin(), m_resourcesParameter.end() );
    	std::vector<ResourceInfo*> sortedSubtractionResources( subtractionResources.begin(), subtractionResources.end() );
    	std::sort(sortedResourcesParameter.begin(), sortedResourcesParameter.end(), [](const ResourceInfo* a, const ResourceInfo* b){return *a < *b;});
    	std::sort( sortedSubtractionResources.begin(), sortedSubtractionResources.end(), [](const ResourceInfo* a, const ResourceInfo* b){return *a < *b;});

    	std::vector<ResourceInfo*> addedResources;
    	std::set_difference(
    		sortedResourcesParameter.begin(), sortedResourcesParameter.end(),
    		sortedSubtractionResources.begin(), sortedSubtractionResources.end(),
    		std::back_inserter(addedResources ),
    		[](const ResourceInfo* a, const ResourceInfo* b){ return *a < *b; });

    	std::vector<ResourceInfo*> removedResources;
    	std::set_difference(
    		sortedSubtractionResources.begin(), sortedSubtractionResources.end(),
    		sortedResourcesParameter.begin(), sortedResourcesParameter.end(),
    		std::back_inserter( removedResources ),
    		[](const ResourceInfo* a, const ResourceInfo* b){ return *a < *b; });

    	std::vector<ResourceInfo*> potentiallyModifiedResources;
    	std::set_intersection(
    		sortedResourcesParameter.begin(), sortedResourcesParameter.end(),
    		sortedSubtractionResources.begin(), sortedSubtractionResources.end(),
    		std::back_inserter( potentiallyModifiedResources ),
    		[](const ResourceInfo* a, const ResourceInfo* b){ return *a < *b; });

        for (ResourceInfo* resource : potentiallyModifiedResources)
        {
			if( params.statusCallback )
			{
				std::filesystem::path relativePath;

				Result getRelativePathResult = resource->GetRelativePath( relativePath );

                if (getRelativePathResult.type != ResultType::SUCCESS)
                {
					return getRelativePathResult;
                }

				std::string message = "Processing: " + relativePath.string();

                auto percentComplete = static_cast<unsigned int>( ( 100 * i )  / m_resourcesParameter.GetSize() );

				params.statusCallback( CarbonResources::StatusLevel::DETAIL, CarbonResources::StatusProgressType::PERCENTAGE, percentComplete, message );
				
                i++;
			}

        	ResourceInfo* resource2 = *std::lower_bound(
        		sortedSubtractionResources.begin(), sortedSubtractionResources.end(),
        		resource,
        		[](const ResourceInfo* a, const ResourceInfo* b) { return *a < *b; } );

            std::string resource1Checksum;

            Result getResource1ChecksumResult = resource->GetChecksum( resource1Checksum );

            if (getResource1ChecksumResult.type != ResultType::SUCCESS)
            {
				return getResource1ChecksumResult;
            }

            std::string resource2Checksum;

            Result getResource2ChecksumResult = resource2->GetChecksum( resource2Checksum );

            if (getResource2ChecksumResult.type != ResultType::SUCCESS)
            {
				return getResource2ChecksumResult;
            }

            // Has this resource changed?
			if( resource1Checksum != resource2Checksum )
            {
                // The binary data has changed between versions, record an entry in both lists

				// Create a copy of the resource to result 2 (Latest)
				ResourceInfo* resourceCopy1 = nullptr;

                Result createResourceFromResource1Result = CreateResourceFromResource( *resource, resourceCopy1 );

                if (createResourceFromResource1Result.type != ResultType::SUCCESS)
                {
					return createResourceFromResource1Result;
                }

				params.result2->AddResource( resourceCopy1 );

                ResourceInfo* resourceCopy2 = nullptr;

                Result createResourceFromResource2Result = CreateResourceFromResource( *resource2, resourceCopy2 );

                if( createResourceFromResource2Result.type != ResultType::SUCCESS )
				{
					return createResourceFromResource2Result;
				}

				params.result1->AddResource( resourceCopy2 );
			}
        }

    	for( auto resource : addedResources )
        {
            // This is a new resource, add it to target
            // Note: Could be made optional, perhaps it is desirable to only include patch updates
			// Not new files, probably make as optional pass in setting
    		if( params.statusCallback )
    		{
    			std::filesystem::path relativePath;
				Result getRelativePathResult = resource->GetRelativePath( relativePath );
                if (getRelativePathResult.type != ResultType::SUCCESS)
                {
					return getRelativePathResult;
                }
    			std::string message = "Processing new resource: " + relativePath.string();
    			auto percentComplete = static_cast<unsigned int>( ( 100 * i ) / m_resourcesParameter.GetSize() );
    			params.statusCallback( CarbonResources::StatusLevel::DETAIL, CarbonResources::StatusProgressType::PERCENTAGE, percentComplete, message );
    			i++;
    		}

			ResourceInfo* resourceCopy1 = nullptr;

            Result createResourceFromResourceResult = CreateResourceFromResource( *resource, resourceCopy1 );

            if (createResourceFromResourceResult.type != ResultType::SUCCESS)
            {
				return createResourceFromResourceResult;
            }

			params.result2->AddResource( resourceCopy1 );

            // Place in a dummy entry into result1 which shows that resource is new
            // This ensures that both lists stay the same size which makes it easier
            // To parse later
			std::filesystem::path resourceRelativePath;

            Result getResourceRelativepathResult = resource->GetRelativePath( resourceRelativePath );

            if (getResourceRelativepathResult.type != ResultType::SUCCESS)
            {
				return getResourceRelativepathResult;
            }

			ResourceInfoParams dummyResourceParams;
			dummyResourceParams.relativePath = resourceRelativePath;

			ResourceInfo* dummyResource = new ResourceInfo( dummyResourceParams );
			params.result1->AddResource( dummyResource );
        }

    	for( auto resource : removedResources )
    	{
    		if( params.statusCallback )
    		{
    			std::filesystem::path relativePath;
				Result getRelativePathResult = resource->GetRelativePath( relativePath );
                if (getRelativePathResult.type != ResultType::SUCCESS)
                {
					return getRelativePathResult;
                }
    			std::string message = "Processing removed resource: " + relativePath.string();
    			auto percentComplete = static_cast<uint32_t>( ( 100 * i ) / m_resourcesParameter.GetSize() );
    			params.statusCallback( CarbonResources::StatusLevel::DETAIL, CarbonResources::StatusProgressType::PERCENTAGE, percentComplete, message );
    			i++;
    		}
    		std::filesystem::path path;
    		auto result = resource->GetRelativePath( path );
    		if( result.type != ResultType::SUCCESS )
    		{
    			return result;
    		}
    		params.removedResources.push_back( path );
    	}

        if( params.statusCallback )
		{
			params.statusCallback( CarbonResources::StatusLevel::DETAIL, CarbonResources::StatusProgressType::PERCENTAGE, 100, "Diff calculation complete." );
		}

        return Result{ ResultType::SUCCESS };
    }

    std::vector<ResourceInfo*>::iterator ResourceGroupImpl::begin()
	{
		return m_resourcesParameter.begin();
	}

	std::vector<ResourceInfo*>::const_iterator ResourceGroupImpl::begin() const
	{
		return m_resourcesParameter.begin();
	}

	std::vector<ResourceInfo*>::const_iterator ResourceGroupImpl::cbegin()
	{
		return m_resourcesParameter.begin();
	}

	std::vector<ResourceInfo*>::iterator ResourceGroupImpl::end()
	{
		return m_resourcesParameter.end();
	}

	std::vector<ResourceInfo*>::const_iterator ResourceGroupImpl::end() const
	{
		return m_resourcesParameter.end();
	}

	std::vector<ResourceInfo*>::const_iterator ResourceGroupImpl::cend()
	{
		return m_resourcesParameter.end();
	}

    size_t ResourceGroupImpl::GetSize() const
    {
		return m_resourcesParameter.GetSize();
    }

	Result ResourceGroupImpl::GetGroupSpecificResourcesToBundle( std::vector<ResourceInfo*>& toBundle ) const
	{
		return Result{ ResultType::SUCCESS };
	}


}