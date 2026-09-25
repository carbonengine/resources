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
    *  @var PatchApplyParams::nextBuildResourcesSourceSettings
    *  Location where new resources can be sourced. Resources will be sourced from here if there are no patches related to them, indicating they are completely new files.
    *  @var PatchApplyParams::patchBinarySourceSettings
    *  Location where patch binaries can be sourced.
    *  @var PatchApplyParams::resourcesToPatchSourceSettings
    *  Location where the resources to be patched can be sourced.
    *  @var PatchApplyParams::resourcesToPatchDestinationSettings
    *  Location where to place patched resources. This can match PatchApplyParams::resourcesToPatchSourceSettings to overwrite. Allows creation of staging area in case of failure.
    *  @var PatchApplyParams::resourcesToRemove
    *  This will be populated with relative paths of the files that have been removed between previous and next supplied.
    *  @var PatchApplyParams::temporaryFilePath
    *  Name of a temporary filename to use when patching large files. This file will be cleaned up on process completion. 
    *  @var PatchApplyParams::callbackSettings
    *  Settings relating to status callback messaging
    *  @var PatchApplyParams::downloadSettings
    *  Settings relating to downloads
    *  @var PatchApplyParams::skipNewFiles
    *  If set then any new files will be skipped rather than retrieved from nextBuildResourcesSource
    */
struct PatchApplyParams final
{
	ResourceSourceSettings nextBuildResourcesSourceSettings{ ResourceSourceType::LOCAL_RELATIVE };

	ResourceSourceSettings patchBinarySourceSettings{};

	ResourceSourceSettings resourcesToPatchSourceSettings{ ResourceSourceType::LOCAL_RELATIVE };

	ResourceDestinationSettings resourcesToPatchDestinationSettings{ ResourceDestinationType::LOCAL_RELATIVE };

    std::vector<std::filesystem::path> resourcesToRemove;

	std::filesystem::path temporaryFilePath = "tempFile.resource";

	CallbackSettings callbackSettings;

    DownloadSettings downloadSettings;

    bool skipNewFiles = false;
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
	Result Apply( PatchApplyParams& params );

private:
	PatchResourceGroupImpl* m_impl;
};

}

#endif // PatchResourceGroup_H