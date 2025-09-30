// Copyright © 2025 CCP ehf.

#pragma once
#ifndef PatchResourceGroup_H
#define PatchResourceGroup_H

#include "Exports.h"
#include "ResourceGroup.h"
#include "Enums.h"
#include <memory>
#include <string>
#include <filesystem>

namespace CarbonResources
{

/** @struct PatchApplyParams
    *  @brief Function Parameters required for CarbonResources::PatchResourceGroup::Apply
    *  @var PatchApplyParams::newBuildResourcesSourceSettings
    *  Location where new resources can be sourced. Resources will be sourced from here if there are no patches related to them, indicating they are completely new files.
    *  @var PatchApplyParams::patchBinarySourceSettings
    *  Location where patch binaries can be sourced.
    *  @var PatchApplyParams::resourcesToPatchSourceSettings
    *  Location where the resources to be patched can be sourced.
    *  @var PatchApplyParams::resourcesToPatchDestinationSettings
    *  Location where to place patched resources. This can match PatchApplyParams::resourcesToPatchSourceSettings to overwrite. Allows creation of staging area in case of failure.
    *  @var PatchApplyParams::temporaryFilePath
    *  Name of a temporary filename to use when patching large files. This file will be cleaned up on process completion. 
    *  @var PatchApplyParams::statusCallback
    *  Optional status function callback. Callback is triggered at key status update events.
    */
struct PatchApplyParams final
{
	ResourceSourceSettings nextBuildResourcesSourceSettings{ ResourceSourceType::LOCAL_RELATIVE };

	ResourceSourceSettings patchBinarySourceSettings{};

	ResourceSourceSettings resourcesToPatchSourceSettings{ ResourceSourceType::LOCAL_RELATIVE };

	ResourceDestinationSettings resourcesToPatchDestinationSettings{ ResourceDestinationType::LOCAL_RELATIVE };

	std::filesystem::path temporaryFilePath = "tempFile.resource";

	StatusCallback statusCallback = nullptr;
};

/** @class PatchResourceGroup
    *  @brief Contains a collection of Patch Resources
    */
class API PatchResourceGroup final : public ResourceGroup
{

public:
	class PatchResourceGroupImpl;

	PatchResourceGroup();

	PatchResourceGroup( const PatchResourceGroup& ) = delete;

	~PatchResourceGroup();

	/// @brief Applies the Patches from the BundleResourceGroup.
	/// @param params input parameters, See PatchApplyParams for more details.
	/// @see ResourceGroup::CreatePatch for information regarding patch creation.
	/// @return Result see CarbonResources::Result for more details.
	Result Apply( const PatchApplyParams& params );

private:
	PatchResourceGroupImpl* m_impl;
};

}

#endif // PatchResourceGroup_H