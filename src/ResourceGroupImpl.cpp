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
#include "ResourceInfo/BinaryResourceInfo.h"
#include "ResourceInfo/PatchResourceGroupInfo.h"
#include "ResourceInfo/BundleResourceGroupInfo.h"
#include "ResourceInfo/ResourceGroupInfo.h"
#include "ResourceInfo/PatchResourceInfo.h"
#include "PatchResourceGroupImpl.h"
#include "BundleResourceGroupImpl.h"

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
			params.statusCallback( 0, "Creating Resource Group From Directory: " + params.directory.string() );
		}

        if (!std::filesystem::exists(params.directory))
        {
			return Result::INPUT_DIRECTORY_DOESNT_EXIST;
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
					params.statusCallback( 0, "Processing File: " + entry.path().string() );
                }

                // Create resource
			    auto fileSize = entry.file_size();

                if( fileSize < params.resourceStreamThreshold )  
                {
                    // Create resource from data
				    ResourceInfoParams resourceParams;
					
                    resourceParams.relativePath = std::filesystem::relative( entry.path(), params.directory );

				    ResourceInfo* resource = new ResourceInfo( resourceParams );

                    std::string resourceData;

                    ResourceGetDataParams resourceGetDataParams;

                    resourceGetDataParams.resourceSourceSettings.basePath = params.directory;

                    resourceGetDataParams.resourceSourceSettings.sourceType = ResourceSourceType::LOCAL_RELATIVE;

                    resourceGetDataParams.data = &resourceData;

                    Result getResourceDataResult = resource->GetData( resourceGetDataParams );

                    if (getResourceDataResult != Result::SUCCESS)
                    {
					    return getResourceDataResult;
                    }

                    Result setParametersFromDataResult = resource->SetParametersFromData( resourceData );

                    if (setParametersFromDataResult != Result::SUCCESS)
                    {
					    return setParametersFromDataResult;
                    }

                    Result addResourceResult = AddResource( resource );

                    if (addResourceResult != Result::SUCCESS)
                    {
					    return addResourceResult;
                    }
                }
                else
                {
                    // Process data via stream
                    ResourceTools::Md5ChecksumStream checksumStream;

                    //ResourceTools::GzipCompressionStream gzipCompressionStream;   //TODO reinstate when fixed

                    ResourceTools::FileDataStreamIn fileStreamIn( params.resourceStreamThreshold );

                    /*
                    if (!gzipCompressionStream.Start())
                    {
						return Result::FAILED_TO_COMPRESS_DATA;
                    }
                    */

                    if (!fileStreamIn.StartRead(entry.path()))
                    {
						return Result::FAILED_TO_OPEN_FILE_STREAM;
                    }

                    unsigned long compressedDataSize = 0;

                    while (!fileStreamIn.IsFinished())
                    {
						// Update status
						if( params.statusCallback )
						{
							float percentage = (100.0 / fileStreamIn.Size()) * fileStreamIn.GetCurrentPosition();
							params.statusCallback( percentage, "Percentage Update" );
						}

						std::string fileData;

                        if (!(fileStreamIn >> fileData))
                        {
							return Result::FAILED_TO_READ_FROM_STREAM;
                        }

                        if (!(checksumStream << fileData))
                        {
							return Result::FAILED_TO_GENERATE_CHECKSUM;
                        }

						std::string compressedData;

                        /*
                        ResourceTools::CompressionChunk compressionChunk;

                        compressionChunk.uncompressedData = &fileData;

                        compressionChunk.compressedData = &compressedData;

                        if( !( gzipCompressionStream << compressionChunk ) )
                        {
							return Result::FAILED_TO_COMPRESS_DATA;
                        }
                        */

                        compressedDataSize += compressedData.size();
                    }
					/*
                    if (!gzipCompressionStream.Finish())
                    {
						return Result::FAILED_TO_COMPRESS_DATA;
                    }
                    */

                    std::string checksum;

                    if (!checksumStream.FinishAndRetrieve(checksum))
                    {
						return Result::FAILED_TO_GENERATE_CHECKSUM;
                    }

                    // Create resource from parameters
                    ResourceInfoParams resourceParams;

					resourceParams.relativePath = std::filesystem::relative( entry.path(), params.directory );

					resourceParams.uncompressedSize = fileSize;

                    resourceParams.compressedSize = compressedDataSize;

                    resourceParams.checksum = checksum;

                    Location l;

					Result calculateLocationResult = l.SetFromRelativePathAndDataChecksum( resourceParams.relativePath, resourceParams.checksum );

                    if (calculateLocationResult != Result::SUCCESS)
                    {
						return calculateLocationResult;
                    }

                    resourceParams.location = l.ToString();

                    ResourceInfo* resource = new ResourceInfo( resourceParams );

                    Result addResourceResult = AddResource( resource );

					if( addResourceResult != Result::SUCCESS )
					{
						return addResourceResult;
					}
                    
                }
			}
        }

        if( params.statusCallback )
		{
			params.statusCallback( 0, "Resource group successfully created from directory");
		}

        return Result::SUCCESS;

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

    Result ResourceGroupImpl::ImportFromFile( const ResourceGroupImportFromFileParams& params )
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
		// Update status
		if( params.statusCallback )
		{
			params.statusCallback( 0, "Exporting Resource Group to file: " + params.filename.string() );
		}

		std::string data = "";

        Result exportYamlResult = ExportYaml( params.outputDocumentVersion, data, params.statusCallback );

        if (exportYamlResult != Result::SUCCESS)
        {
			return exportYamlResult;
        }

        if( !ResourceTools::SaveFile( params.filename, data ) )
		{
			return Result::FAILED_TO_SAVE_FILE;
		}

        if( params.statusCallback )
		{
			params.statusCallback( 0, "Resource group successfully exported." );
		}

		return Result::SUCCESS;
    }

    Result ResourceGroupImpl::ExportToData( std::string& data,  VersionInternal outputDocumentVersion /* = S_DOCUMENT_VERSION*/) const
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
			m_versionParameter = VersionInternal{ 0, 1, 0 };

			// Create a Resource
			ResourceInfo* resource = new ResourceInfo( resourceParams );

            Result addResourceResult = AddResource( resource );

            if (addResourceResult != Result::SUCCESS)
            {
				return addResourceResult;
            }
		}

		return Result::SUCCESS;
    }

    Result ResourceGroupImpl::CreateResourceFromResource( const ResourceInfo& resourceIn, ResourceInfo*& resourceOut ) const
    {
		resourceOut = nullptr;

		std::string resourceType = "";

		Result getResourceTypeResult = resourceIn.GetType( resourceType );

        if (getResourceTypeResult != Result::SUCCESS)
        {
			return getResourceTypeResult;
        }

        if (resourceType == ResourceInfo::TypeId())
        {
			resourceOut = new ResourceInfo( {} );

            Result setParametersFromResourceResult = resourceOut->SetParametersFromResource( &resourceIn, m_versionParameter.GetValue() );

            if (setParametersFromResourceResult != Result::SUCCESS)
            {
				return setParametersFromResourceResult;
            }
        }
        else if (resourceType == PatchResourceInfo::TypeId())
        {
			PatchResourceInfo* patchResourceInfo = new PatchResourceInfo( {} );

            Result setParametersFromResourceResult = patchResourceInfo->SetParametersFromResource( &resourceIn, m_versionParameter.GetValue() );

            if( setParametersFromResourceResult != Result::SUCCESS )
			{
				return setParametersFromResourceResult;
			}

            resourceOut = patchResourceInfo;
        }
		else if( resourceType == BundleResourceInfo::TypeId() )
		{
			BundleResourceInfo* bundleResourceInfo = new BundleResourceInfo( {} );

			Result setParametersFromResourceResult = bundleResourceInfo->SetParametersFromResource( &resourceIn, m_versionParameter.GetValue() );
            
            if( setParametersFromResourceResult != Result::SUCCESS )
			{
				return setParametersFromResourceResult;
			}

			resourceOut = bundleResourceInfo;
		}
		else if( resourceType == BundleResourceInfo::TypeId() )
		{
			BinaryResourceInfo* binaryResourceInfo = new BinaryResourceInfo( {} );

			Result setParametersFromResourceResult = binaryResourceInfo->SetParametersFromResource( &resourceIn, m_versionParameter.GetValue() );

            if( setParametersFromResourceResult != Result::SUCCESS )
			{
				return setParametersFromResourceResult;
			}

			resourceOut = binaryResourceInfo;
		}

        return Result::SUCCESS;

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
		
        VersionInternal version;
		version.FromString( versionStr );
        m_versionParameter = version;

		if( m_versionParameter.GetValue().getMajor() > S_DOCUMENT_VERSION.major )
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

            Result addResourceResult = AddResource( resource );

			if( addResourceResult != Result::SUCCESS )
			{
				return addResourceResult;
			}

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

    Result ResourceGroupImpl::ExportGroupSpecialisedYaml( YAML::Emitter& out, VersionInternal outputDocumentVersion ) const
    {
		return Result::SUCCESS;
    } 

    Result ResourceGroupImpl::ExportYaml( const VersionInternal& outputDocumentVersion, std::string& data, std::function<void( int, const std::string& )> statusCallback /*= nullptr*/ ) const
    {
		
        YAML::Emitter out;

        // Output header information
		out << YAML::BeginMap;

        // It is possible to export a different version that the imported version
        // The version must be less than the version of the document and also no higher than supported by the binary at compile
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

		if( res != Result::SUCCESS )
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
				float percentage = (100.0 / m_resourcesParameter.GetValue()->size()) * i;
				statusCallback( percentage, "Percentage Update" );
				i++;
			}

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

		if( putChunkDataResult != Result::SUCCESS )
		{
			delete chunkResource;

			return putChunkDataResult;
		}

		// Add the chunk resource to the bundleResourceGroup
		Result addResourceResult = bundleResourceGroup.AddResource( chunkResource );

		if( addResourceResult != Result::SUCCESS )
		{
			delete chunkResource;

			return addResourceResult;
		}

        return Result::SUCCESS;
    }

    Result ResourceGroupImpl::CreateBundle( const BundleCreateParams& params ) const
    {
        unsigned long numberOfChunks = 0;

        std::string chunkBaseName = params.resourceGroupRelativePath.filename().replace_extension().string();

		BundleResourceGroupImpl bundleResourceGroup;

        bundleResourceGroup.SetChunkSize( params.chunkSize );

		ResourceTools::BundleStreamOut bundleStream( params.chunkSize );

        // Loop through all resources and send data for chunking
        for (ResourceInfo* resource : m_resourcesParameter)
        {
            ResourceTools::FileDataStreamIn resourceDataStream(params.fileReadChunkSize);

            ResourceGetDataStreamParams resourceGetDataParams;

            resourceGetDataParams.resourceSourceSettings = params.resourceSourceSettings;

            resourceGetDataParams.dataStream = &resourceDataStream;

			Result resourceGetDataResult = resource->GetDataStream( resourceGetDataParams );

            if (resourceGetDataResult != Result::SUCCESS)
            {
				return resourceGetDataResult;
            }
			
            while (!resourceDataStream.IsFinished() )
            {
				std::string resourceDataChunk;

                if (!(resourceDataStream >> resourceDataChunk))
                {
					return Result::FAILED_TO_READ_FROM_STREAM;
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

					Result processChunkResult = ProcessChunk( chunkData, chunkPath, bundleResourceGroup, params.chunkDestinationSettings );

					if( processChunkResult != Result::SUCCESS )
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

		if( processChunkResult != Result::SUCCESS )
		{
			return processChunkResult;
		}




		// Export this resource list
		std::string resourceGroupData;

		ExportToData( resourceGroupData );

		ResourceGroupInfo resourceGroupInfo( { params.resourceGroupRelativePath } );

		resourceGroupInfo.SetParametersFromData( resourceGroupData );

		ResourcePutDataParams putDataParams;

		putDataParams.resourceDestinationSettings = params.chunkDestinationSettings; // TODO the resource list is going where the chunks are, perhaps this is missleading

		putDataParams.data = &resourceGroupData;

		Result subtractionResourcePutResult = resourceGroupInfo.PutData( putDataParams );

		if( subtractionResourcePutResult != Result::SUCCESS )
		{
			return subtractionResourcePutResult;
		}



		// Export the bundleGroup
		bundleResourceGroup.SetResourceGroup( resourceGroupInfo );

		std::string patchResourceGroupData;

		bundleResourceGroup.ExportToData( patchResourceGroupData );

		BundleResourceGroupInfo patchResourceGroupInfo( { params.resourceGroupBundleRelativePath } );

		patchResourceGroupInfo.SetParametersFromData( patchResourceGroupData );

		ResourcePutDataParams bundlePutDataParams;

		bundlePutDataParams.resourceDestinationSettings = params.resourceBundleResourceGroupDestinationSettings;

		bundlePutDataParams.data = &patchResourceGroupData;

		Result patchResourceGroupPutResult = patchResourceGroupInfo.PutData( bundlePutDataParams );

		if( patchResourceGroupPutResult != Result::SUCCESS )
		{
			return patchResourceGroupPutResult;
		}

        return Result::SUCCESS;
    }

    Result ResourceGroupImpl::CreatePatch( const PatchCreateParams& params ) const
    {
        if (params.previousResourceGroup->m_impl->GetType() != GetType())
        {
			return Result::PATCH_RESOURCE_LIST_MISSMATCH;
        }

        PatchResourceGroupImpl patchResourceGroup;

        patchResourceGroup.SetMaxInputChunkSize( params.maxInputFileSize );

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

        int patchId = 0;

        for (int i = 0; i < resourceGroupSubtractionLatest.m_resourcesParameter.GetSize(); i++)
        {
			ResourceInfo* resourcePrevious = resourceGroupSubtractionPrevious.m_resourcesParameter.At( i );

			ResourceInfo* resourceNext = resourceGroupSubtractionLatest.m_resourcesParameter.At( i );

            // Check to see if previous entry contains dummy information
            // Suggesting that this is a new entry in latest
            // In which case there is no reason to create a patch
            // The new entry will be stored with the ResourceGroup related to the PatchResourceGroup
			unsigned long previousUncompressedSize;

            Result getResourcePreviousCompressedSizeResult = resourcePrevious->GetUncompressedSize( previousUncompressedSize );

            if (getResourcePreviousCompressedSizeResult != Result::SUCCESS)
            {
				return getResourcePreviousCompressedSizeResult;
            }



            unsigned long nextUncompressedSize;

			Result getResourceNextCompressedSizeResult = resourceNext->GetUncompressedSize( nextUncompressedSize );

			if( getResourceNextCompressedSizeResult != Result::SUCCESS )
			{
				return getResourceNextCompressedSizeResult;
			}


            if( previousUncompressedSize != 0 ) // TODO make a note of what is going on here, it is confusing
            {
				// Get resource data previous
				ResourceTools::FileDataStreamIn previousFileDataStream( params.maxInputFileSize );

				ResourceGetDataStreamParams previousResourceGetDataStreamParams;

				previousResourceGetDataStreamParams.resourceSourceSettings = params.resourceSourceSettingsFrom;

				previousResourceGetDataStreamParams.dataStream = &previousFileDataStream;

                Result getPreviousDataStreamResult = resourcePrevious->GetDataStream( previousResourceGetDataStreamParams );

                if (getPreviousDataStreamResult != Result::SUCCESS)
                {
					return getPreviousDataStreamResult;
                }

                // Get resource data next
				ResourceTools::FileDataStreamIn nextFileDataStream( params.maxInputFileSize );

				ResourceGetDataStreamParams nextResourceGetDataStreamParams;

				nextResourceGetDataStreamParams.resourceSourceSettings = params.resourceSourceSettingsTo;

				nextResourceGetDataStreamParams.dataStream = &nextFileDataStream;

                Result getNextDataStreamResult = resourceNext->GetDataStream( nextResourceGetDataStreamParams );

				if( getNextDataStreamResult != Result::SUCCESS )
				{
					return getNextDataStreamResult;
				}

                // Process one chunk at a time
				for( unsigned long dataOffset = 0; dataOffset < nextUncompressedSize; dataOffset += params.maxInputFileSize )
                {

					std::string previousFileData = "";

                    // Handling if previous file is smaller than next file
                    // If so then previousFileData will be nothing and
                    // All next data will be used for the patch
                    if (!previousFileDataStream.IsFinished())
                    {
						if( !( previousFileDataStream >> previousFileData ) )
						{
							return Result::FAILED_TO_RETRIEVE_CHUNK_DATA;
						}
                    }
                    

                    // Note: in the case that the next file is smaller than previous
                    // nothing is stored, application of the patch will chop off the extra file data
                    std::string nextFileData;

                    if(!(nextFileDataStream >> nextFileData))
					{
						return Result::FAILED_TO_RETRIEVE_CHUNK_DATA;
					}

                    // Create a patch
					// Create a patch from the data
					std::string patchData;

                    if (previousFileData != "")
					{
                        // Calculate checksum of the chunks
                        // If they match there is no need for a patch
						std::string previousFileDataChecksum;

                        if (!ResourceTools::GenerateMd5Checksum(previousFileData, previousFileDataChecksum))
                        {
							return Result::FAILED_TO_GENERATE_CHECKSUM;
                        }

                        std::string nextFileDataChecksum;

						if( !ResourceTools::GenerateMd5Checksum( nextFileData, nextFileDataChecksum ) )
						{
							return Result::FAILED_TO_GENERATE_CHECKSUM;
						}

                        if (previousFileDataChecksum == nextFileDataChecksum)
                        {
                            // The chunks of the file are the same
                            // No patch is required
                            // TODO test
							continue;
                        }

                        // Previous and next data chunk are different, create a patch
						if( !ResourceTools::CreatePatch( previousFileData, nextFileData, patchData ) )
						{
							return Result::FAILED_TO_CREATE_PATCH;
						}
                    }
                    else
                    {
                        // If there is no previous data then just store the data straight from the file
                        // All this data is new
						patchData = nextFileData;
                    }
					

                    //TODO handle the case where previousFileData and nextFileData match, in this case don't create a patch


					// Create a resource from patch data
					std::filesystem::path resourceLatestRelativePath;

					Result getResourceLatestRelativePathResult = resourceNext->GetRelativePath( resourceLatestRelativePath );

					if( getResourceLatestRelativePathResult != Result::SUCCESS )
					{
						return getResourceLatestRelativePathResult;
					}

					PatchResourceInfoParams patchResourceInfoParams;

					std::string patchFilename = params.patchFileRelativePathPrefix.string() + "." + std::to_string( patchId ); // TODO odd

					patchResourceInfoParams.relativePath = patchFilename;

					patchResourceInfoParams.targetResourceRelativePath = resourceLatestRelativePath;

                    patchResourceInfoParams.dataOffset = dataOffset;


					PatchResourceInfo* patchResource = new PatchResourceInfo( patchResourceInfoParams );

					patchResource->SetParametersFromData( patchData );



					// Export patch file
					ResourcePutDataParams resourcePutDataParams;

					resourcePutDataParams.resourceDestinationSettings = params.resourcePatchBinaryDestinationSettings;

					resourcePutDataParams.data = &patchData;

					Result putPatchDataResult = patchResource->PutData( resourcePutDataParams );

					if( putPatchDataResult != Result::SUCCESS )
					{
						delete patchResource;

						return putPatchDataResult;
					}

					// Add the patch resource to the patchResourceGroup
					Result addResourceResult = patchResourceGroup.AddResource( patchResource );

					if( addResourceResult != Result::SUCCESS )
					{
						delete patchResource;

						return addResourceResult;
					}

                    patchId++;


                }

            }

        }

        // Export the subtraction ResourceGroup
        std::string resourceGroupData;

        resourceGroupSubtractionLatest.ExportToData( resourceGroupData );

		ResourceGroupInfo subtractionResourceGroupInfo( { params.resourceGroupRelativePath } );

        subtractionResourceGroupInfo.SetParametersFromData( resourceGroupData );

        ResourcePutDataParams putDataParams;

        putDataParams.resourceDestinationSettings = params.resourcePatchBinaryDestinationSettings;

        putDataParams.data = &resourceGroupData; 

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

		patchPutDataParams.data = &patchResourceGroupData;

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

        m_numberOfResources = m_numberOfResources.GetValue() + 1;

        unsigned long resourceUncompressedSize;
            
        Result resourceGetUncompressedSizeResult = resource->GetUncompressedSize( resourceUncompressedSize );

        if( resourceGetUncompressedSizeResult != Result::SUCCESS )
        {
			return resourceGetUncompressedSizeResult;
        }

        m_totalResourcesSizeUncompressed = m_totalResourcesSizeUncompressed.GetValue() + resourceUncompressedSize;

        unsigned long resourceCompressedSize;

		Result resourceGetCompressedSizeResult = resource->GetCompressedSize( resourceCompressedSize );

		if( resourceGetCompressedSizeResult != Result::SUCCESS )
		{
			return resourceGetCompressedSizeResult;
		}

		m_totalResourcesSizeCompressed = m_totalResourcesSizeCompressed.GetValue() + resourceCompressedSize;

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

                std::string resource1Checksum;

                Result getResource1ChecksumResult = resource->GetChecksum( resource1Checksum );

                if (getResource1ChecksumResult != Result::SUCCESS)
                {
					return getResource1ChecksumResult;
                }

                std::string resource2Checksum;

                Result getResource2ChecksumResult = resource2->GetChecksum( resource2Checksum );

                if (getResource2ChecksumResult != Result::SUCCESS)
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

                    if (createResourceFromResource1Result != Result::SUCCESS)
                    {
						return createResourceFromResource1Result;
                    }

					params.result2->AddResource( resourceCopy1 );

                    // Create a copy of resource to result 1 (Previous)
					ResourceInfo* resource2 = ( *subtractionResourcesFindIter );

                    ResourceInfo* resourceCopy2 = nullptr;

                    Result createResourceFromResource2Result = CreateResourceFromResource( *resource2, resourceCopy2 );

                    if( createResourceFromResource2Result != Result::SUCCESS )
					{
						return createResourceFromResource2Result;
					}

					params.result1->AddResource( resourceCopy2 );
                }
			}
            else
            {
                // This is a new resource, add it to target
                // Note: Could be made optional, perhaps it is desirable to only include patch updates
				// Not new files, probably make as optional pass in setting
				ResourceInfo* resourceCopy1 = nullptr;

                Result createResourceFromResourceResult = CreateResourceFromResource( *resource, resourceCopy1 );

                if (createResourceFromResourceResult != Result::SUCCESS)
                {
					return createResourceFromResourceResult;
                }

				params.result2->AddResource( resourceCopy1 );

                // Place in a dummy entry into result1 which shows that resource is new
                // This ensures that both lists stay the same size which makes it easier
                // To parse later
				std::filesystem::path resourceRelativePath;

                Result getResourceRelativepathResult = resource->GetRelativePath( resourceRelativePath );

                if (getResourceRelativepathResult != Result::SUCCESS)
                {
					return getResourceRelativepathResult;
                }

				ResourceInfoParams dummyResourceParams;
				dummyResourceParams.relativePath = resourceRelativePath;

				ResourceInfo* dummyResource = new ResourceInfo( dummyResourceParams );
				params.result1->AddResource( dummyResource );

            }
            
        }

        return Result::SUCCESS;
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


}