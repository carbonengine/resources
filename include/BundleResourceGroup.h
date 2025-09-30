// Copyright © 2025 CCP ehf.

#pragma once
#ifndef BundleResourceGroup_H
#define BundleResourceGroup_H

#include "Exports.h"
#include "ResourceGroup.h"
#include "Enums.h"
#include <memory>
#include <string>

namespace CarbonResources
{

/** @struct BundleUnpackParams
    *  @brief Function Parameters required for CarbonResources::BundleResourceGroup::Unpack
    *  @var BundleUnpackParams::chunkSourceSettings
    *  Location where chunks can be sourced.
    *  @var BundleUnpackParams::resourceDestinationSettings
    *  Location where the unpacked resources should be saved.
    *  @var BundleUnpackParams::statusCallback
    *  Optional status function callback. Callback is triggered at key status update events.
    */
struct BundleUnpackParams final
{
	ResourceSourceSettings chunkSourceSettings;

	ResourceDestinationSettings resourceDestinationSettings;

	StatusCallback statusCallback = nullptr;
};

/** @class BundleResourceGroup
    *  @brief Contains a collection of Chunk Resources
    */
class API BundleResourceGroup final : public ResourceGroup
{
public:
	class BundleResourceGroupImpl;

	BundleResourceGroup();

	BundleResourceGroup( const BundleResourceGroup& ) = delete;

	~BundleResourceGroup();

	/// @brief Unpacks the Resoruces from the BundleResourceGroup.
	/// @param params input parameters, See BundleUnpackParams for more details.
	/// @see ResourceGroup::CreateBundle for information regarding bundle creation.
	/// @return Result see CarbonResources::Result for more details.
	Result Unpack( const BundleUnpackParams& params );

private:
	BundleResourceGroupImpl* m_impl;
};

}

#endif // BundleResourceGroup_H