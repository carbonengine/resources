#include "BinaryResourceGroupImpl.h"
#include "BinaryResource.h"

#include <yaml-cpp/yaml.h>
#include <fstream>

namespace CarbonResources
{

    BinaryResourceGroupImpl::BinaryResourceGroupImpl( const std::string& relativePath ) :
	    ResourceGroupImpl(relativePath)
    {
		m_type = TypeId();
    }

    BinaryResourceGroupImpl::~BinaryResourceGroupImpl()
    {

    }

    std::string BinaryResourceGroupImpl::TypeId()
    {
        return "BinaryGroup";
    }

    Resource* BinaryResourceGroupImpl::CreateResourceFromYaml( YAML::Node& resource )
	{
		BinaryResource* createdResource = new BinaryResource( BinaryResourceParams{} );

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

    Result BinaryResourceGroupImpl::ImportGroupSpecialisedYaml( YAML::Node& resourceGroupFile )
    {
		return Result::SUCCESS;
    }

    Result BinaryResourceGroupImpl::ExportGroupSpecialisedYaml( YAML::Emitter& out, Version outputDocumentVersion ) const
    {
        return Result::SUCCESS;
    }

    Result BinaryResourceGroupImpl::ImportFromCSVFile( ResourceGroupImportFromFileParams& params )
	{
        // Note: code duplication with ResourceGroup deemed not that important as this method is depricated and to be removed
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

			std::stringstream ss( stringIn );

			std::string value;

			char delimiter = ',';

			CarbonResources::BinaryResourceParams binaryResourceParams;

			if( !std::getline( ss, value, delimiter ) )
			{
				return Result::MALFORMED_RESOURCE_INPUT;
			}

           // Split filename and prefix
			std::string resourcePrefixDelimiter = ":/";
			std::string filename = value.substr( value.find( resourcePrefixDelimiter ) + resourcePrefixDelimiter.size() );
			std::string resourceType = value.substr( 0, value.find( ":" ) );

			binaryResourceParams.relativePath = filename;

			if( !std::getline( ss, value, delimiter ) )
			{
				return Result::MALFORMED_RESOURCE_INPUT;
			}

			binaryResourceParams.location = value;

			if( !std::getline( ss, value, delimiter ) )
			{
				return Result::MALFORMED_RESOURCE_INPUT;
			}

			binaryResourceParams.checksum = value;

			if( !std::getline( ss, value, delimiter ) )
			{
				return Result::MALFORMED_RESOURCE_INPUT;
			}

			binaryResourceParams.uncompressedSize = atol( value.c_str() );

			if( !std::getline( ss, value, delimiter ) )
			{
				return Result::MALFORMED_RESOURCE_INPUT;
			}

			binaryResourceParams.compressedSize = atol( value.c_str() );

            if( !std::getline( ss, value, delimiter ) )
			{
				return Result::MALFORMED_RESOURCE_INPUT;
			}

			binaryResourceParams.binaryOperation = atol( value.c_str() );

            // Version is 0.0.0 which denotes legacy file support
			m_versionParameter = Version{ 0, 0, 0 };

			// Create aBinary Resource
			CarbonResources::BinaryResource* binaryResource = new CarbonResources::BinaryResource( binaryResourceParams );

            m_resourcesParameter.PushBack( binaryResource );

		}

		return Result::SUCCESS;
	}

}