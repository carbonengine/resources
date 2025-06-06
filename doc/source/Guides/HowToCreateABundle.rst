How To Create A Bundle
======================

Patch creation can create many files. This is due many factors such as chunked input.

Furthermore, once the patch binaries are compressed their size may reduce dramatically.

This gives a patch which is created of many very small files which is not ideal for transfer over the internet.

In order to solve this carbon-resources also provides a concept of bundling.

Bundling joins files together into chunks based on their final combined compressed size.

Note: This document refers to filesystem types, see :doc:`../DesignDocuments/filesystemDesign` for more details.


.. code::
    
    .\carbon-resources.exe create-bundle PatchOut\PatchResourceGroup.yaml --resource-source-path PatchOut\Patches --resource-source-type LOCAL_CDN --resourcegroup-type PatchResourceGroup --chunk-destination-type REMOTE_CDN
    

**Arguments:**

1. Positional argument - Path to the PatchResourceGroup.yaml that is to be bundled
2. ``--resource-source-path`` - Source base path where resources referred to in the PatchResourceGroup can be sourced
3. ``--resource-source-type`` - filesystem type that the resources adhere to. Create patch command used in the previous section outputted to LOCAL_CDN so this is used as an input here.
4. ``--resourcegroup-type`` - Type of resource group supplied, here we are bundling a patch resource group so this is reflected in this setting.
5. ``--chunk-destination`` - Destination filesystem type to output the chunks of the bundle to. REMOTE_CDN will output in a CDN friendly structure and also compress the files ready for upload.

.. note::
    See CLI help for more information regarding options.

This command will create 
1. A ``BundleResourceGroup.yaml`` file at the default location ``./BundleResourceGroup.yaml``.
2. Chunks at default location ``BundleOut/`` in destination filesystem format ``REMOTE_CDN``
