// Copyright © 2025 CCP ehf.

#include "FilterIndexMappingFile.h"

#include <yaml-cpp/yaml.h>

bool FilterIndexMappingFile::LoadFromFile( const std::filesystem::path& path )
{

	YAML::Node file;
	try
	{
		file = YAML::LoadFile( path.u8string() );
	}
	catch( YAML::ParserException& )
	{
		return false;
	}

	YAML::Node version = file["Version"];

	if( !version.IsDefined() )
	{
		return false;
	}

	YAML::Node filterIndexMappings = file["FilterIndexMappings"];

	if( !filterIndexMappings.IsDefined() )
	{
		return false;
	}

	// Iterate through all the mappings
	for( auto iter = filterIndexMappings.begin(); iter != filterIndexMappings.end(); iter++ )
	{
		std::unique_ptr<FilterMapping> mapping = std::make_unique<FilterMapping>();

		YAML::Node filterIndexMapping = ( *iter );

		YAML::Node outputIndexFilename = filterIndexMapping["OutputIndexFilename"];

		if( !outputIndexFilename.IsDefined() )
		{
			return false;
		}

		mapping->outputPath = outputIndexFilename.as<std::string>();

		YAML::Node filterMapping = filterIndexMapping["FilterMapping"];

		for( auto iter = filterMapping.begin(); iter != filterMapping.end(); iter++ )
		{
			YAML::Node filterFilePath = ( *iter );

			YAML::Node filterFile = filterFilePath["FilterFile"];

			mapping->filterFilePaths.push_back( filterFile.as<std::string>() );
		}

		if( mapping->filterFilePaths.size() == 0 )
		{
			return false;
		}

		m_filterMappings.push_back( std::move( mapping ) );
	}

	if( m_filterMappings.size() == 0 )
	{
		return false;
	}

	return true;
}

const std::vector<std::unique_ptr<FilterMapping>>& FilterIndexMappingFile::GetFilterMappings() const
{
	return m_filterMappings;
}
