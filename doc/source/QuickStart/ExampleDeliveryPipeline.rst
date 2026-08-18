Example Delivery Pipeline
=========================

This is a simple delivery pipeline soley using the CLI with dev tools available.

In a real scenario the consumer of the bundles and patches would use the library api.


Scenario
--------

This will outline a method for updating a new client build.

The build will offer two methods for delivering the updated client.

1. A full bundle containing the new client data.
2. Patches to bring a current install of a previous client to the new version.

The data used here will simply be both accessible locally, however this doesn't have to be the case, they could be sourced remotely.

1. ClientA (The previous client build)
2. ClientB (The new client build)

Build Side
----------

The build side will be responsible for creation of the data and uploading to a remote location.

1. Create Resource Groups for both builds

This will create resource group representations of each directory of data.

``--skip-compression`` is used as we don't need the compression calculation information and this is a slow calculation

``.\resources.exe create-group ClientA --verbosity-level -1 --output-file ResourceGroupClientA.yaml --skip-compression``

``.\resources.exe create-group ClientB --verbosity-level -1 --output-file ResourceGroupClientB.yaml --skip-compression``

2. Create Patch

This will create patch files between the two builds.
A max overall patch size is specified ``--max-overall-patch-size``, if during processing a patch exceeds this value then the patch process will fail.
This stops huge inefficient patches being created. In this instance no further processing or upload of patch data is requred and the processing pipeline can skip directly to 5.

``.\resources.exe create-patch ResourceGroupClientA.yaml ResourceGroupClientB.yaml --resource-source-base-path-previous ClientA --resource-source-base-path-next ClientB --chunk-size 1048576 --skip-compression --max-overall-patch-size 1073741824 --verbosity-level -1``

3. Bundle Patch

The patch files will be numerous and have a high opportunity for compression, therefore they are bundled by splitting on compression, which is the default behaviour.

``.\resources.exe create-bundle PatchOut/PatchResourceGroup.yaml --resource-source-path .\PatchOut\Patches\ --resource-source-type LOCAL_CDN --chunk-destination-path ToCDN/BundledPatches/Chunks/ --chunk-destination-type REMOTE_CDN --bundle-resourcegroup-destination-path ToCDN/BundledPatches/ --bundle-resourcegroup-relative-path BundledPatchResourceGroup.yaml --verbosity-level -1``

4. Bundle New Files

By default patch creation in step 2 will also create a Resource Group containing the new files that have been added between the two builds.
These new files are not included in the patch files and so need to be delivered in some way to the client.
There are two ways to do this. The default is to supply a location to the ``apply-patch`` operation where resources can download the new files from.
However this may not be the most efficient approach as the files may be very small or large.
Therefore here we create a bundle of the new files and deliver as part of our pipeline.

``.\resources.exe create-bundle PatchOut/NewFilesResourceGroup.yaml --resource-source-path ClientB --resource-source-type LOCAL_RELATIVE --chunk-destination-path ToCDN/BundledNewFiles/Chunks/ --chunk-destination-type REMOTE_CDN --bundle-resourcegroup-destination-path ToCDN/BundledNewFiles/ --bundle-resourcegroup-relative-path BundledNewFilesResourceGroup.yaml --verbosity-level -1``

5. Bundle Entire new Build

Finally we will offer a full bundle for the new build. This is useful if a consumer doesn't already have a version of the old build on their system, or if there is no patch available for some reason (e.g. the patch would have been huge).
This operation has no dependency on the previous steps and can be done in parallel.

``.\resources.exe create-bundle .\ResourceGroupClientB.yaml --verbosity-level -1 --resource-source-path ClientB --split-on-uncompressed-size --chunk-destination-type REMOTE_CDN --chunk-destination-path ToCDN/BundledClient/Chunks/ --bundle-resourcegroup-destination-path ToCDN/BundledClient/``

6. Upload to remote location (CDN)

In this example a dir named ``ToCDN`` now contains everything for this latest build and can be uploaded. If patch creation was also successful this will include directores:
    1. BundledClient - The full bundle of the ClientB.
    2. BundledNewFiles - A bundle for all the files added between ClientA and ClientB
    3. BundledPatches - A bundle for all the patches between ClientA and ClientB


Delivery Side
-------------

This is the side that will be managed by some sort of launcher on the users system.

Current state of the users's system could be a few scenarios and each require a different code path.

1. ClientA is not currently on the system: In this case go unbundle the whole ClientB

2. ClientA is on the system but there is no patch available: Again unbundle the whole ClientB

3. ClientA is on the system and a patch is available: Apply the patch and unbundle the new files, in case of any failure fallback to unbundling full ClientB.


Unbundling ClientB
------------------
Assuming the build files are available at ``https://cdn.com/ToCDN``

1. Download the bundle resource group file:

``https://cdn.com/ToCDN/BundledClient/BundleResourceGroup.yaml``

2. Unbundle the client

``.\resources.exe unpack-bundle .\ToCDN\BundledClient\BundleResourceGroup.yaml --chunk-source-base-path https://cdn.com/ToCDN/ToCDN/BundledClient/Chunks/ --chunk-source-type REMOTE_CDN --resource-destination-base-path Out/UnpackedClient --resource-destination-type LOCAL_RELATIVE --verbosity-level -1``

3. Copy bundle files to the correct location
The files are in a staging area, this stops the possibility of a bundle failing half way through and being left with a broken build.
The files will be in the following location:
    1. ``Out/UnpackedClient`` - all client files.


Patching From ClientA to ClientB
--------------------------------

Assuming the build files are available at ``https://cdn.com/ToCDN``

1. Download the bundled patch resource group file:

``https://cdn.com/ToCDN/BundledPatches/BundledPatchResourceGroup.yaml``

If this file is not found then assume that no patch is available and download via bundle.

2. Download the bundled new files resource group file:

``https://cdn.com/ToCDN/BundledNewFiles/BundledNewFilesResourceGroup.yaml``

If the bundled patch exists but this doesn't then the patch is corrupt, fall back to download via bundle.

3. Unbundle the patch files

``.\resources.exe unpack-bundle .BundledPatchResourceGroup.yaml --chunk-source-base-path https://cdn.com/ToCDN/ToCDN/BundledNewFiles/Chunks/ --chunk-source-type REMOTE_CDN --resource-destination-base-path Out/UnpackedNewFiles --resource-destination-type LOCAL_RELATIVE --verbosity-level -1``

4. Unbundle the new files

``.\resources.exe unpack-bundle .BundledNewFilesResourceGroup.yaml --chunk-source-base-path https://cdn.com/ToCDN/ToCDN/BundledPatches/Chunks/ --chunk-source-type REMOTE_CDN --resource-destination-base-path Out/UnpackedPatches --resource-destination-type LOCAL_CDN --verbosity-level -1`` 

5. Apply patches

``.\resources.exe apply-patch .\Out\UnpackedPatches\ResourceGroup.yaml --patch-binaries-base-path .\out\UnpackedPatches\ --resources-to-patch-base-path .\ClientA\ --output-base-path Out/PatchedClientFiles/ --skip-new-files --verbosity-level -1``

6. Copy the patch files to the correct location

The files are in a staging area for safety, this stops the possibilty of a patch failing half way through and being left with a broken build.
Once the patch process has completed succcessfully the files need to be copied to the real ClientA location.
The files will be in the following locations:
    1. ``Out/UnpackedNewFiles`` - New files that will need to copied in.
    2. ``Out/PatchedClientFiles/`` - Updated files after patching procedure sould replace files already present on the system.
    3. Removed files are returned in the lib in vector '', these will need to be removed from the destination.