#include "BinaryResourceGroupImpl.h"
#include "BinaryResource.h"

#include <yaml-cpp/yaml.h>
#include <fstream>

namespace CarbonResources
{

    BinaryResourceGroup::BinaryResourceGroupImpl::BinaryResourceGroupImpl( )
    {

    }

    BinaryResourceGroup::BinaryResourceGroupImpl::~BinaryResourceGroupImpl()
    {

    }

    std::string BinaryResourceGroup::BinaryResourceGroupImpl::Type() const
    {
        return "BinaryGroup";
    }

    Resource* BinaryResourceGroup::BinaryResourceGroupImpl::CreateResourceFromYaml( YAML::Node& resource )
	{
		CarbonResources::BinaryResourceParams binaryResourceParams;

		binaryResourceParams.ImportFromYaml( resource, m_versionParameter.GetValue() );

		return new BinaryResource( binaryResourceParams );
	}

    Result BinaryResourceGroup::BinaryResourceGroupImpl::ImportGroupSpecialisedYaml( YAML::Node& resourceGroupFile )
    {
		return Result::SUCCESS;
    }

    Result BinaryResourceGroup::BinaryResourceGroupImpl::ExportGroupSpecialisedYaml( YAML::Emitter& out, Version outputDocumentVersion ) const
    {
        return Result::SUCCESS;
    }

    Result BinaryResourceGroup::BinaryResourceGroupImpl::ImportFromCSVFile( const ResourceGroupImportFromFileParams& params )
	{
        // Note: code duplication with ResourceGroup deemed not important as this method is depricated and to be removed
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

			std::stringstream ss( stringIn );

			std::string value;

			char delimiter = ',';

			CarbonResources::BinaryResourceParams binaryResourceParams;

			if( !std::getline( ss, value, delimiter ) )
			{
				return Result::MALFORMED_RESOURCE_INPUT;
			}

			binaryResourceParams.relativePath = value;

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
			//m_resources.push_back( binaryResource );
		}

		inputStream.close();

		return Result::SUCCESS;
	}

}