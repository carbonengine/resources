#include "BinaryResourceGroupImpl.h"
//#include "BinaryResource.h"
#include "ResourceInfo/ResourceGroupInfo.h"

#include <yaml-cpp/yaml.h>
#include <fstream>
#include <iostream>
#include <ResourceTools.h>

namespace CarbonResources
{

    BinaryResourceGroupImpl::BinaryResourceGroupImpl( ) :
	    ResourceGroupImpl()
    {
		m_type = TypeId();
    }

    BinaryResourceGroupImpl::~BinaryResourceGroupImpl()
    {

    }

    std::string BinaryResourceGroupImpl::GetType() const
	{
		return TypeId();
	}

    std::string BinaryResourceGroupImpl::TypeId()
    {
        return "BinaryGroup";
    }

    Result BinaryResourceGroupImpl::CreateResourceFromYaml( YAML::Node& resource, ResourceInfo*& resourceOut )
	{
		BinaryResourceInfo* binaryResource = new BinaryResourceInfo( BinaryResourceInfoParams{} );

        Result importFromYamlResult = binaryResource->ImportFromYaml( resource, m_versionParameter.GetValue() );

		if( importFromYamlResult != Result::SUCCESS )
		{
			delete binaryResource;

			return importFromYamlResult;
		}
		else
		{
			resourceOut = binaryResource;

			return Result::SUCCESS;
		}
	}

    Result BinaryResourceGroupImpl::ImportGroupSpecialisedYaml( YAML::Node& resourceGroupFile )
    {
		return Result::SUCCESS;
    }

    Result BinaryResourceGroupImpl::ExportGroupSpecialisedYaml( YAML::Emitter& out, VersionInternal outputDocumentVersion ) const
    {
        return Result::SUCCESS;
    }

    Result BinaryResourceGroupImpl::ImportFromCSV( const std::string& data )
	{
		std::stringstream inputStream;

		inputStream << data;

		std::string stringIn;

		while( !inputStream.eof() )
		{
			std::getline( inputStream, stringIn );

            if( stringIn == "" )
			{
				continue;
			}

			std::stringstream ss( stringIn );

			std::string value;

			char delimiter = ',';

			CarbonResources::BinaryResourceInfoParams binaryResourceParams;

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
			m_versionParameter = VersionInternal{ 0, 0, 0 };

			// Create aBinary Resource
			BinaryResourceInfo* binaryResource = new BinaryResourceInfo( binaryResourceParams );

            Result addResourceResult = AddResource( binaryResource );

			if( addResourceResult != Result::SUCCESS )
			{
				return addResourceResult;
			}

		}

		return Result::SUCCESS;
	}

}