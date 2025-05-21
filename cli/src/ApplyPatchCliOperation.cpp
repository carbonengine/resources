#include "ApplyPatchCliOperation.h"

#include <PatchResourceGroup.h>

#include <iostream>
#include <argparse/argparse.hpp>

ApplyPatchCliOperation::ApplyPatchCliOperation() :
	CliOperation("apply-patch", "Apply a patch"),
	m_patchResourceGroupPathArgumentId("--patch-resource-group-path"),
	m_patchBinariesBasePathArgumentId("--patch-binaries-base-path"),
	m_patchBinariesSourceTypeArgumentId("--patch-binaries-source-type"),
	m_resourcesToPatchBasePathArgumentId("--resources-to-patch-base-path"),
	m_resourcesToPatchSourceTypeArgumentId("--resources-to-patch-source-type"),
	m_nextResourcesBasePathArgumentId("--next-resources-base-path"),
	m_nextResourcesSourceTypeArgumentId("--next-resources-source-type"),
	m_outputBasePathArgumentId("--output-base-path"),
	m_outputDestinationTypeArgumentId("--output-destination-type")
{
	AddArgument( m_patchResourceGroupPathArgumentId, "The path to the PatchResourceGroup.yaml file.", true);
	AddArgument( m_patchBinariesBasePathArgumentId,"The path to the folder containing the patch binaries.", true);
	AddArgument( m_patchBinariesSourceTypeArgumentId,"The type of repository the patch binaries are sourced from.", false, false, "LOCAL_RELATIVE");
	AddArgument( m_resourcesToPatchBasePathArgumentId,"The path to the folder containing resources to patch.", true);
	AddArgument( m_resourcesToPatchSourceTypeArgumentId,"The type of repository the resources to patch are sourced from.", false, false, "LOCAL_RELATIVE");
	AddArgument( m_nextResourcesBasePathArgumentId,"The path to the folder containing the resources after the patch. This is used to download added files, which are not included in the generated patch files.", true);
	AddArgument( m_nextResourcesSourceTypeArgumentId,"The type of repository the resources after the patch are sourced from.", false, false, "LOCAL_RELATIVE");
	AddArgument( m_outputBasePathArgumentId,"The path in which to place the patched version of the files.", true);
	AddArgument( m_outputDestinationTypeArgumentId,"The type of repository in which to place the patched version of the files.", false, false, "LOCAL_RELATIVE");
}

bool ApplyPatchCliOperation::Execute() const
{
	CarbonResources::ResourceGroupImportFromFileParams importParamsPrevious;

	std::optional<std::string> filename = m_argumentParser->present<std::string>( m_patchResourceGroupPathArgumentId );
	if( !filename.has_value() )
	{
		return false;
	}
    importParamsPrevious.filename = filename.value();

	CarbonResources::PatchApplyParams patchApplyParams;

	std::vector<std::filesystem::path> newBuildResourceSettingBasePaths;
	std::optional<std::string> nextResources = m_argumentParser->present<std::string>( m_nextResourcesBasePathArgumentId );
	if( !nextResources.has_value() )
	{
		return false;
	}
	newBuildResourceSettingBasePaths.push_back( nextResources.value() );
	std::string nextResourcesType = m_argumentParser->get( m_nextResourcesSourceTypeArgumentId );
	if( !StringToResourceSourceType( nextResourcesType, patchApplyParams.newBuildResourcesSourceSettings.sourceType ) )
	{
		return false;
	}

	std::vector<std::filesystem::path> patchBinarySourceSettingsBasePaths;
	std::optional<std::string> patchBinaries = m_argumentParser->present<std::string>( m_patchBinariesBasePathArgumentId );
	if( !patchBinaries.has_value() )
	{
		return false;
	}
	patchBinarySourceSettingsBasePaths.push_back( patchBinaries.value() );
	patchApplyParams.patchBinarySourceSettings.basePaths = patchBinarySourceSettingsBasePaths;
	patchApplyParams.newBuildResourcesSourceSettings.basePaths = newBuildResourceSettingBasePaths;
	std::string patchBinariesType = m_argumentParser->get( m_patchBinariesSourceTypeArgumentId );
	if( !StringToResourceSourceType( patchBinariesType, patchApplyParams.patchBinarySourceSettings.sourceType ) )
	{
		return false;
	}

	std::vector<std::filesystem::path> resourcesToPatchSourceSettingsBasePaths;
	std::optional<std::string> resourcesToPatch = m_argumentParser->present<std::string>( m_resourcesToPatchBasePathArgumentId );
	if( !resourcesToPatch.has_value() )
	{
		return false;
	}
	resourcesToPatchSourceSettingsBasePaths.push_back( resourcesToPatch.value() );
    patchApplyParams.resourcesToPatchSourceSettings.basePaths = resourcesToPatchSourceSettingsBasePaths;
	std::string resourcesToPatchType = m_argumentParser->get( m_resourcesToPatchSourceTypeArgumentId );
	if( !StringToResourceSourceType( resourcesToPatchType, patchApplyParams.resourcesToPatchSourceSettings.sourceType ) )
	{
		return false;
	}

	std::optional<std::string> outputPath = m_argumentParser->present( m_outputBasePathArgumentId );
	if( !outputPath.has_value() )
	{
		return false;
	}
    patchApplyParams.resourcesToPatchDestinationSettings.basePath = outputPath.value();
	std::string outputType = m_argumentParser->get( m_outputDestinationTypeArgumentId );
	if( !StringToResourceDestinationType( outputType , patchApplyParams.resourcesToPatchDestinationSettings.destinationType ) )
	{
		return false;
	}

    patchApplyParams.temporaryFilePath = "tempFile.resource";

	PrintStartBanner( importParamsPrevious, patchApplyParams );
	return ApplyPatch( importParamsPrevious, patchApplyParams );
}

std::string Stringify( const std::vector<std::filesystem::path>& v )
{
	std::string result;
	bool first{true};
	for( const auto& s : v )
	{
		if(!first)
		{
			result += ",";
		}
		first = false;
		result += s;
	}
	return result;
}

void ApplyPatchCliOperation::PrintStartBanner(const CarbonResources::ResourceGroupImportFromFileParams& importParamsPrevious, const CarbonResources::PatchApplyParams patchApplyParams) const
{
	if( s_verbosity <= 0 )
	{
		return;
	}

	std::cout << "---Applying Patch---" << std::endl;

    PrintCommonOperationHeaderInformation();

	std::cout << "Patch Resource Group: " << importParamsPrevious.filename << std::endl;
	std::cout << "Patch Binaries Base Path: " << Stringify( patchApplyParams.patchBinarySourceSettings.basePaths ) << std::endl;
	std::cout << "Patch Binaries Source Type: " << SourceTypeToString( patchApplyParams.patchBinarySourceSettings.sourceType ) << std::endl;
	std::cout << "Resources To Patch Base Path: " << Stringify( patchApplyParams.resourcesToPatchSourceSettings.basePaths ) << std::endl;
	std::cout << "Resources To Patch Source Type: " << SourceTypeToString( patchApplyParams.resourcesToPatchSourceSettings.sourceType ) << std::endl;
	std::cout << "Next Resources Base Path: " << Stringify( patchApplyParams.newBuildResourcesSourceSettings.basePaths ) << std::endl;
	std::cout << "Next Resources Source Type: " << SourceTypeToString( patchApplyParams.newBuildResourcesSourceSettings.sourceType ) << std::endl;
	std::cout << "Output Path Base Path: " << patchApplyParams.resourcesToPatchDestinationSettings.basePath << std::endl;
	std::cout << "Output Path Destination Type: " << DestinationTypeToString( patchApplyParams.resourcesToPatchDestinationSettings.destinationType ) << std::endl;

	std::cout << "----------------------------\n" << std::endl;
}

bool ApplyPatchCliOperation::ApplyPatch(const CarbonResources::ResourceGroupImportFromFileParams& importParamsPrevious, const CarbonResources::PatchApplyParams patchApplyParams) const
{
	// Load the patch file
	CarbonResources::PatchResourceGroup patchResourceGroup;
	if( patchResourceGroup.ImportFromFile( importParamsPrevious ).type != CarbonResources::ResultType::SUCCESS )
	{
		return false;
	}

    // Apply the patch
	return patchResourceGroup.Apply( patchApplyParams ).type == CarbonResources::ResultType::SUCCESS;
}
