#include "CreatePatchCliOperation.h"

#include <string>
#include <argparse/argparse.hpp>
#include <ResourceGroup.h>

CreatePatchCliOperation::CreatePatchCliOperation() :
	CliOperation( "create-patch", "Create a patch from two supplied ResourceGroups" ),
	m_previousResourceGroupPathArgumentId( "previous-resourcegroup-path" ),
	m_nextResourceGroupPathArgumentId( "next-resourcegroup-path" ),
	m_resourceGroupRelativePathArgumentId( "--resourcegroup-relative-path" ),
	m_patchResourceGroupRelativePathArgumentId( "--patchResourcegroup-relative-path" ),
	m_resourceSourceTypePreviousArgumentId( "--resource-source-type-previous" ),
	m_resourceSourceBasePathPreviousArgumentId( "--resource-source-base-path-previous" ),
	m_resourceSourceTypeNextArgumentId( "--resource-source-type-next" ),
	m_resourceSourceBasePathNextArgumentId( "--resource-source-base-path-next" ),
	m_patchBinaryDestinationTypeArgumentId( "--patch-destination-type" ),
	m_patchBinaryDestinationBasePathArgumentId( "--patch-destination-base-path" ),
	m_patchResourceGroupDestinationTypeArgumentId( "--patch-resourcegroup-destination-type" ),
	m_patchResourceGroupDestinationBasePathArgumentId( "--patch-resourcegroup-destination-path" ),
	m_patchFileRelativePathPrefix( "--patch-prefix" ),
	m_maxInputChunkSize( "--chunk-size" )
{

    // TODO: The interface here is totally WIP, it needs actually designing to be easy to use.
	// This is linked to having sensible default values so that not all options are always required and large complex commands are opt in.
	AddRequiredPositionalArgument( m_previousResourceGroupPathArgumentId, "Filename to previous resourceGroup." );

    AddRequiredPositionalArgument( m_nextResourceGroupPathArgumentId, "Filename to next resourceGroup." );

    AddArgument( m_resourceGroupRelativePathArgumentId, "-rg", "Relative path for output resourceGroup which will contain the diff between the supplied previous ResourceGroup and next ResourceGroup.", true, "ResourceGroup.yaml" );

    AddArgument( m_patchResourceGroupRelativePathArgumentId, "-pg", "Relative path for output PatchResourceGroup which will contain all the patches produced.", true, "PatchResourceGroup.yaml" );

    AddArgument( m_resourceSourceTypePreviousArgumentId, "-rtp", "Represents the type of repository to source resources for previous.", true, "LOCAL_RELATIVE" );

    AddArgument( m_resourceSourceBasePathPreviousArgumentId, "-rbp", "Represents the base path to source resources for previous.", true, "" );

    AddArgument( m_resourceSourceTypeNextArgumentId, "-rtn", "Represents the type of repository to source resources for next.", true, "LOCAL_RELATIVE" );

	AddArgument( m_resourceSourceBasePathNextArgumentId, "-rbn", "Represents the base path to source resources for next.", true, "" );

    AddArgument( m_patchBinaryDestinationTypeArgumentId, "-bt", "Represents the type of repository where binary patches will be saved.", true, "LOCAL_RELATIVE" );

	AddArgument( m_patchBinaryDestinationBasePathArgumentId, "-bb", "Represents the base path where binary patches will be saved.", true, "" );

    AddArgument( m_patchResourceGroupDestinationTypeArgumentId, "-bt", "Represents the type of repository where the patch ResourceGroup will be saved.", true, "LOCAL_RELATIVE" );

	AddArgument( m_patchResourceGroupDestinationBasePathArgumentId, "-bb", "Represents the base path where the patch ResourceGroup will be saved.", true, "" );

    AddArgument( m_patchFileRelativePathPrefix, "-pp", "Relative path prefix for produced patch binaries. Default is “Patches/Patch” which will produce patches such as Patches/Patch.1 …", false, "Patches/Patch" );

    AddArgument( m_patchFileRelativePathPrefix, "-c", "Files are processed in chunks, maxInputFileChunkSize indicate the size of this chunk. Files smaller than chunk will be processed in one pass.", false, "100000000" );

}

bool CreatePatchCliOperation::Execute() const
{
	CarbonResources::ResourceGroupImportFromFileParams previousResourceGroupParams;

    previousResourceGroupParams.filename = m_argumentParser->get<std::string>( m_previousResourceGroupPathArgumentId );

    CarbonResources::ResourceGroupImportFromFileParams nextResourceGroupParams;

    nextResourceGroupParams.filename = m_argumentParser->get<std::string>( m_nextResourceGroupPathArgumentId );

    CarbonResources::PatchCreateParams createPatchParams;

    createPatchParams.resourceGroupRelativePath = m_argumentParser->get<std::string>( m_resourceGroupRelativePathArgumentId );

    createPatchParams.resourceGroupPatchRelativePath = m_argumentParser->get<std::string>( m_patchResourceGroupRelativePathArgumentId );

    std::string resourceSourceTypePrevious = m_argumentParser->get<std::string>( m_resourceSourceTypePreviousArgumentId );
	
    if( resourceSourceTypePrevious == "LOCAL_CDN" )
    {
		createPatchParams.resourceSourceSettingsFrom.sourceType = CarbonResources::ResourceSourceType::LOCAL_CDN;
    }
	else if( resourceSourceTypePrevious == "REMOTE_CDN" )
	{
		createPatchParams.resourceSourceSettingsFrom.sourceType = CarbonResources::ResourceSourceType::REMOTE_CDN;
	}
	else if( resourceSourceTypePrevious == "LOCAL_RELATIVE" )
	{
		createPatchParams.resourceSourceSettingsFrom.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;
	}
    else
    {
		return false;
    }

    createPatchParams.resourceSourceSettingsFrom.basePath = m_argumentParser->get<std::string>( m_resourceSourceBasePathPreviousArgumentId );

    std::string resourceSourceTypeNext = m_argumentParser->get<std::string>( m_resourceSourceTypeNextArgumentId );

	if( resourceSourceTypeNext == "LOCAL_CDN" )
	{
		createPatchParams.resourceSourceSettingsTo.sourceType = CarbonResources::ResourceSourceType::LOCAL_CDN;
	}
	else if( resourceSourceTypeNext == "REMOTE_CDN" )
	{
		createPatchParams.resourceSourceSettingsTo.sourceType = CarbonResources::ResourceSourceType::REMOTE_CDN;
	}
	else if( resourceSourceTypeNext == "LOCAL_RELATIVE" )
	{
		createPatchParams.resourceSourceSettingsTo.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;
	}
	else
	{
		return false;
	}

	createPatchParams.resourceSourceSettingsTo.basePath = m_argumentParser->get<std::string>( m_resourceSourceBasePathNextArgumentId );


    std::string patchBinaryDestinationType = m_argumentParser->get<std::string>( m_patchBinaryDestinationTypeArgumentId );

	if( patchBinaryDestinationType == "LOCAL_CDN" )
	{
		createPatchParams.resourcePatchBinaryDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_CDN;
	}
	else if( patchBinaryDestinationType == "REMOTE_CDN" )
	{
		createPatchParams.resourcePatchBinaryDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::REMOTE_CDN;
	}
	else if( patchBinaryDestinationType == "LOCAL_RELATIVE" )
	{
		createPatchParams.resourcePatchBinaryDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_RELATIVE;
	}
	else
	{
		return false;
	}

	createPatchParams.resourcePatchBinaryDestinationSettings.basePath = m_argumentParser->get<std::string>( m_patchBinaryDestinationBasePathArgumentId );




    std::string patchResourceGroupDestinationType = m_argumentParser->get<std::string>( m_patchResourceGroupDestinationTypeArgumentId );

	if( patchResourceGroupDestinationType == "LOCAL_CDN" )
	{
		createPatchParams.resourcePatchResourceGroupDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_CDN;
	}
	else if( patchResourceGroupDestinationType == "REMOTE_CDN" )
	{
		createPatchParams.resourcePatchResourceGroupDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::REMOTE_CDN;
	}
	else if( patchResourceGroupDestinationType == "LOCAL_RELATIVE" )
	{
		createPatchParams.resourcePatchResourceGroupDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_RELATIVE;
	}
	else
	{
		return false;
	}

	createPatchParams.resourcePatchResourceGroupDestinationSettings.basePath = m_argumentParser->get<std::string>( m_patchResourceGroupDestinationBasePathArgumentId );

    createPatchParams.patchFileRelativePathPrefix = m_argumentParser->get<std::string>( m_patchFileRelativePathPrefix );

	createPatchParams.maxInputFileChunkSize = m_argumentParser->get<uintmax_t>( m_maxInputChunkSize );

	return CreatePatch( previousResourceGroupParams, nextResourceGroupParams, createPatchParams );
}

bool CreatePatchCliOperation::CreatePatch( const CarbonResources::ResourceGroupImportFromFileParams& previousResourceGroupParams, const CarbonResources::ResourceGroupImportFromFileParams& nextResourceGroupParams, const CarbonResources::PatchCreateParams& createPatchParams ) const
{
	// Previous ResourceGroup
	CarbonResources::ResourceGroup resourceGroupPrevious;

	CarbonResources::Result importPreviousFromFileResult = resourceGroupPrevious.ImportFromFile( previousResourceGroupParams );

	if( importPreviousFromFileResult != CarbonResources::Result::SUCCESS )
	{
		PrintCarbonResourcesError( importPreviousFromFileResult );

		return false;
	}

	// Latest ResourceGroup
	CarbonResources::ResourceGroup resourceGroupLatest;

	CarbonResources::Result importNextFromFileResult = resourceGroupLatest.ImportFromFile( nextResourceGroupParams );

	if( importNextFromFileResult != CarbonResources::Result::SUCCESS )
	{
		PrintCarbonResourcesError( importPreviousFromFileResult );

		return false;
	}

	CarbonResources::Result createPatchResult = resourceGroupLatest.CreatePatch( createPatchParams );

	if( createPatchResult != CarbonResources::Result::SUCCESS )
	{
		PrintCarbonResourcesError( createPatchResult );

		return false;
	}
}