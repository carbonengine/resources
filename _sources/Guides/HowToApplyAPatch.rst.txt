How To Apply A Patch
====================

Patch application can be performed via the library.

Note: This document refers to filesystem types, see :doc:`../DesignDocuments/filesystemDesign` for more details.

Appling a patch using the library
---------------------------------

Input directory structure required for example

Patch
- PatchResourceGroup.yaml : Patch Resource Group containing patch meta data
- Patch/LocalCDNPatches/ : Directory containing Binary patch files, this example expects them in ``LOCAL_CDN`` structure.
- Patch/PreviousBuildResources/ : Directory containing the resources for the previous build.
- Patch/NextBuildResources/ : Directory containing the resources for the latest build.

.. code-block:: c++

    // Load the patch file
    CarbonResources::PatchResourceGroup patchResourceGroup;

    CarbonResources::ResourceGroupImportFromFileParams importParamsPrevious;

    importParamsPrevious.filename = "Patch/PatchResourceGroup.yaml";

    if(!patchResourceGroup.ImportFromFile( importParamsPrevious ).type == CarbonResources::ResultType::SUCCESS )
    {
        // Unexpected Error
        return false;
    }

    // Apply the patch
    CarbonResources::PatchApplyParams patchApplyParams;

    patchApplyParams.nextBuildResourcesSourceSettings.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;

    patchApplyParams.nextBuildResourcesSourceSettings.basePaths = { "Patch/NextBuildResources/" };

    patchApplyParams.patchBinarySourceSettings.sourceType = CarbonResources::ResourceSourceType::LOCAL_CDN;

    patchApplyParams.patchBinarySourceSettings.basePaths = { "Patch/LocalCDNPatches/" };

    patchApplyParams.resourcesToPatchSourceSettings.sourceType = CarbonResources::ResourceSourceType::LOCAL_RELATIVE;

    patchApplyParams.resourcesToPatchSourceSettings.basePaths = { "Patch/PreviousBuildResources/" };

    patchApplyParams.resourcesToPatchDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_RELATIVE;

    patchApplyParams.resourcesToPatchDestinationSettings.basePath = "ApplyPatchOut";

    patchApplyParams.temporaryFilePath = "tempFile.resource";

    if(!patchResourceGroup.Apply( patchApplyParams ).type == CarbonResources::ResultType::SUCCESS)
    {
        // Unexpected Error
        return false;
    }


Reconstituted files will be placed in ``ApplyPatchOut/`` in ``LOCAL_RELATIVE`` filesystem format.


Appling a patch using the CLI
-----------------------------

**The CLI must have been built with DEV_FEATURES in order to apply patches using the CLI.**

The following command will perform the same action as above given the same input file setup.

.. code::
    
    .\resources.exe apply-patch Patch/PatchResourceGroup.yaml --patch-binaries-base-path Patch/LocalCDNPatches/ --resources-to-patch-base-path Patch/PreviousBuildResources/ --next-resources-base-path Patch/NextBuildResources/


**Arguments:**

1. Positional argument - Path to the PatchResourceGroup.yaml that is to be bundled
2. ``--resources-to-patch-base-path`` - The paths to the folders containing resources to patch.
3. ``--next-resources-base-path`` - The path to resources after the patch. This is used to get fully added files which are not included in the generated patch files.

.. note::
    See CLI help for more information regarding options.

This will apply binary patches to the files inside Patch/PreviousBuildResources/.
Once complete files will have been updated to the same as files in Patch/NextBuildResources/