// Copyright © 2025 CCP ehf.

#include "BundleResourceGroup.h"
#include "BundleResourceGroupImpl.h"

namespace CarbonResources
{

BundleResourceGroup::BundleResourceGroup() :
	ResourceGroup( new BundleResourceGroupImpl() ),
	m_impl( reinterpret_cast<BundleResourceGroupImpl*>( ResourceGroup::m_impl ) )
{
}

BundleResourceGroup::~BundleResourceGroup()
{
}

Result BundleResourceGroup::Unpack( const BundleUnpackParams& params )
{
	StatusSettings statusSettings;
	statusSettings.SetCallbackSettings( params.callbackSettings );
	statusSettings.Update( CarbonResources::StatusProgressType::START, 0, 0, "Starting Process" );

	return m_impl->Unpack( params, statusSettings );
}

}