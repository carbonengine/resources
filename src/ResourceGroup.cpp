// Copyright © 2025 CCP ehf.

#include "StatusSettings.h"
#include "ResourceGroup.h"
#include "ResourceGroupImpl.h"
#include "Enums.h"

namespace CarbonResources
{

ResourceGroup::ResourceGroup( ResourceGroupImpl* impl ) :
	m_impl( impl )
{
}

ResourceGroup::ResourceGroup() :
	m_impl( new ResourceGroupImpl() )
{
}

ResourceGroup::~ResourceGroup()
{
	delete m_impl;
}

Result ResourceGroup::CreateBundle( const BundleCreateParams& params ) const
{
	StatusSettings statusSettings;
	statusSettings.SetCallbackSettings( params.callbackSettings );
	statusSettings.Update( CarbonResources::StatusProgressType::START, 0, 0, "Starting Process" );

	return m_impl->CreateBundle( params, statusSettings );
}

Result ResourceGroup::CreatePatch( const PatchCreateParams& params ) const
{
	StatusSettings statusSettings;
	statusSettings.SetCallbackSettings( params.callbackSettings );
	statusSettings.Update( CarbonResources::StatusProgressType::START, 0, 0, "Starting Process" );

	return m_impl->CreatePatch( params, statusSettings );
}

Result ResourceGroup::ImportFromFile( const ResourceGroupImportFromFileParams& params ) const
{
	StatusSettings statusSettings;
	statusSettings.SetCallbackSettings( params.callbackSettings );
	statusSettings.Update( CarbonResources::StatusProgressType::START, 0, 0, "Starting Process" );

	return m_impl->ImportFromFile( params, statusSettings );
}

Result ResourceGroup::ExportToFile( const ResourceGroupExportToFileParams& params ) const
{
	StatusSettings statusSettings;
	statusSettings.SetCallbackSettings( params.callbackSettings );
	statusSettings.Update( CarbonResources::StatusProgressType::START, 0, 0, "Starting Process" );

	return m_impl->ExportToFile( params, statusSettings );
}

Result ResourceGroup::CreateFromDirectory( const CreateResourceGroupFromDirectoryParams& params )
{
	StatusSettings statusSettings;
	statusSettings.SetCallbackSettings( params.callbackSettings );
	statusSettings.Update( CarbonResources::StatusProgressType::START, 0, 0, "Starting Process" );

	return m_impl->CreateFromDirectory( params, statusSettings );
}

Result ResourceGroup::Merge( const ResourceGroupMergeParams& params ) const
{
	StatusSettings statusSettings;
	statusSettings.SetCallbackSettings( params.callbackSettings );
	statusSettings.Update( CarbonResources::StatusProgressType::START, 0, 0, "Starting Process" );

	return m_impl->Merge( params, statusSettings );
}

Result ResourceGroup::DiffAgainstGroup( const ResourceGroupDiffAgainstGroupParams& params ) const
{
	StatusSettings statusSettings;
	statusSettings.SetCallbackSettings( params.callbackSettings );
	statusSettings.Update( CarbonResources::StatusProgressType::START, 0, 0, "Starting Process" );

	return m_impl->DiffChangesAsLists( params, statusSettings );
}

Result ResourceGroup::RemoveResources( const ResourceGroupRemoveResourcesParams& params ) const
{
	StatusSettings statusSettings;
	statusSettings.SetCallbackSettings( params.callbackSettings );
	statusSettings.Update( CarbonResources::StatusProgressType::START, 0, 0, "Starting Process" );

	return m_impl->RemoveResources( params, statusSettings );
}

}