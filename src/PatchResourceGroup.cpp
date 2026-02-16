// Copyright © 2025 CCP ehf.

#include "PatchResourceGroup.h"
#include "PatchResourceGroupImpl.h"

namespace CarbonResources
{

PatchResourceGroup::PatchResourceGroup() :
	ResourceGroup( new PatchResourceGroupImpl() ),
	m_impl( reinterpret_cast<PatchResourceGroupImpl*>( ResourceGroup::m_impl ) )
{
}

PatchResourceGroup::~PatchResourceGroup()
{
}

Result PatchResourceGroup::Apply( const PatchApplyParams& params )
{
	StatusSettings statusSettings;
	statusSettings.SetCallbackSettings( params.callbackSettings );
    statusSettings.Update( CarbonResources::StatusProgressType::START, 0, 0, "Starting Process" );

	return m_impl->Apply( params, statusSettings );
}

}