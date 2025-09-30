// Copyright © 2025 CCP ehf.

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
	return m_impl->CreateBundle( params );
}

Result ResourceGroup::CreatePatch( const PatchCreateParams& params ) const
{
	return m_impl->CreatePatch( params );
}

Result ResourceGroup::ImportFromFile( const ResourceGroupImportFromFileParams& params ) const
{
	return m_impl->ImportFromFile( params );
}

Result ResourceGroup::ExportToFile( const ResourceGroupExportToFileParams& params ) const
{
	return m_impl->ExportToFile( params );
}

Result ResourceGroup::CreateFromDirectory( const CreateResourceGroupFromDirectoryParams& params )
{
	return m_impl->CreateFromDirectory( params );
}

Result ResourceGroup::Merge( const ResourceGroupMergeParams& params ) const
{
	return m_impl->Merge( params );
}

Result ResourceGroup::DiffAgainstGroup( const ResourceGroupDiffAgainstGroupParams& params ) const
{
	return m_impl->DiffChangesAsLists( params );
}

Result ResourceGroup::RemoveResources( const ResourceGroupRemoveResourcesParams& params ) const
{
	return m_impl->RemoveResources( params );
}

}