How To Unpack A Bundle
======================

Unpacking a bundle can be performed via the library.

Note: This document refers to filesystem types, see :doc:`../DesignDocuments/filesystemDesign` for more details.

Unpacking a bundle using the library
------------------------------------

Input directory structure required for example

Bundle
- BundleResourceGroup.yaml : Bundle Resource Group containing Bundle meta data
- Chunks/ : Directory containing chunks stored in ``LOCAL_CDN`` filesystem format.

.. code-block:: c++

    // Load the bundle file
    CarbonResources::BundleResourceGroup bundleResourceGroup;

    CarbonResources::ResourceGroupImportFromFileParams importParamsPrevious;

    importParamsPrevious.filename = GetTestFileFileAbsolutePath( "Bundle/BundleResourceGroup.yaml" );

    if(!bundleResourceGroup.ImportFromFile( importParamsPrevious ).type == CarbonResources::ResultType::SUCCESS )
    {
        // Unexpected Error
        return false;
    }


    // Unpack the bundle
    CarbonResources::BundleUnpackParams bundleUnpackParams;

    bundleUnpackParams.chunkSourceSettings.sourceType = CarbonResources::ResourceSourceType::LOCAL_CDN;

    bundleUnpackParams.chunkSourceSettings.basePaths = { "Bundle/Chunks/" };

    bundleUnpackParams.resourceDestinationSettings.destinationType = CarbonResources::ResourceDestinationType::LOCAL_RELATIVE;

    bundleUnpackParams.resourceDestinationSettings.basePath = "UnpackBundleOut/";

    if(!bundleResourceGroup.Unpack( bundleUnpackParams ).type == CarbonResources::ResultType::SUCCESS )
    {
        // Unexpected Error
        return false;
    }


Reconsituted files will be placed in ``UnpackBundleOut/`` in ``LOCAL_RELATIVE`` filesystem format.


Unpacking a bundle using the CLI
--------------------------------

**The CLI must have been built with DEV_FEATURES in order to unpack using the CLI.**

The following command will perform the same action as above given the same input file setup.

.. code::
    
    .\resources.exe unpack-bundle Bundle\BundleResourceGroup.yaml --chunk-source-base-path \Bundle\Chunks --resource-destination-type LOCAL_RELATIVE


**Arguments:**

1. Positional argument - Path to the BundleResourceGroup.yaml that is to be bundled
2. ``--chunk-source-base-path`` - The path to the directory containing the bundled files.
3. ``--resource-destination-type`` - The type of repository in which to place the bundle files.

.. note::
    See CLI help for more information regarding options.

This will unpack chunks back into resource files at the default location ``UnpackedBundleOut``. The resources will be stored following filesystem type ``LOCAL_RELATIVE``.