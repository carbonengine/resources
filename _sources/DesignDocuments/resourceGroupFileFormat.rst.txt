Resource Group File Format
==========================

Resource Groups are human readable Yaml files which encapsulate all data associated a group of resources.

The word 'Resources' is used here, however the term is interchangeable with 'file'.

Resource Groups are an evolution of the resfileindex.txt files used in EVE and Frontier. Many of the file management
concepts of resources originate there.

Most operations require Resource Groups as inputs. Operations output may also contain generated ResourceGroups.

ResourceGroup is a base type, extended versions which add additional fields for with special operations include

1. PatchResourceGroup - Contains a location to a ResourceGroup which it is based on the resources represent binary diffs.

2. BundleResourceGroup - Contains a location to a ResourceGroup which it was based on and the resources are chunks which make up the bundle.

Example ResourceGroup file
--------------------------

.. code-block:: yaml

    Version: 0.1.0
    Type: ResourceGroup
    NumberOfResources: 621
    TotalResourcesSizeCompressed: 36013964
    TotalResourcesSizeUnCompressed: 155170840
    Resources:

    RelativePath: buildtrees/argparse/config-x64-windows-carbon-err.log
    Type: Resource
    Location: 4a/4a2879b6b400c8cc_d41d8cd98f00b204e9800998ecf8427e
    Checksum: d41d8cd98f00b204e9800998ecf8427e
    UncompressedSize: 0
    CompressedSize: 20
    BinaryOperation: 33206

    RelativePath: buildtrees/argparse/config-x64-windows-carbon-out.log
    Type: Resource
    Location: 16/169c6bb00fdceb69_c888d1e49f02ac82658ebad2a0808f70
    Checksum: c888d1e49f02ac82658ebad2a0808f70
    UncompressedSize: 3719
    CompressedSize: 1294
    BinaryOperation: 33206
    ...

ResourceGroup Fields
--------------------

.. list-table:: Document Fields
   :widths: 25 25
   :header-rows: 1

   * - Field
     - Description
   * - Version
     - Version number of the Resource Group document
   * - Type
     - Type of ResourceGroup. Type ‘ResourceGroup’ is the base type. There are also specialisations which add further fields related to their function.
   * - NumberOfResources
     - Number of resources which appear in the ResourceGroup.
   * - TotalResourcesSizeCompressed
     - Total compressed size of all the files which appear in the ResourceGroup.
   * - TotalResourcesSizeUnCompressed
     - Total uncompressed size of all the files which appear in the ResourceGroup
   * - Resources
     - List of resources and information relating to them

.. list-table:: Resource Fields
   :widths: 25 25
   :header-rows: 1

   * - Field
     - Description
   * - RelativePath
     - Relative path to the resource.
   * - Type
     - Type of resource.
   * - Location
     - Location where resource would appear in a Content Deliver Network (CDN) structure.
   * - Checksum
     - Data checksum of the uncompressed resource.
   * - UncompressedSize
     - Uncompressed size of resource in bytes.
   * - CompressedSize
     - Compressed size of resource in bytes.
   * - BinaryOperation
     - Binary operation for the resource

Example PatchResourceGroup file
-------------------------------

.. code-block:: yaml

    Version: 0.1.0
    Type: PatchGroup
    NumberOfResources: 2
    TotalResourcesSizeCompressed: 125
    TotalResourcesSizeUnCompressed: 228
    ResourceGroupResource:
      RelativePath: PatchResourceGroup.yaml
      Type: ResourceGroup
      Location: 16/169490e8f16ecbbd_064c6a93988a6dfb92cf480826a1de45
      Checksum: 064c6a93988a6dfb92cf480826a1de45
      UncompressedSize: 868
      CompressedSize: 341
    MaxInputChunkSize: 50000000
    Resources:
      - RelativePath: Patches/Patch.0
        Type: BinaryPatch
        Location: 89/8917d12f0dc2f2a4_23038413c133ea89d37201b6660a0383
        Checksum: 23038413c133ea89d37201b6660a0383
        UncompressedSize: 145
        CompressedSize: 67
        TargetResourceRelativePath: TextDocument1.txt
        DataOffset: 0
        SourceOffset: 0
      - RelativePath: Patches/Patch.1
        Type: BinaryPatch
        Location: 89/8917d12f0dc2f2a5_27729bde0c1cb7e299408059a3809617
        Checksum: 27729bde0c1cb7e299408059a3809617
        UncompressedSize: 83
        CompressedSize: 58
        TargetResourceRelativePath: TextDocument2.txt
        DataOffset: 0
        SourceOffset: 0
       ...


PatchResourceGroup Additional Fields
------------------------------------

.. list-table:: Document Fields
   :widths: 25 25
   :header-rows: 1

   * - Field
     - Description
   * - ResourceGroupResource
     - Resource information for a Resource Group containing resources to patch

.. list-table:: Patch Resource Fields
   :widths: 25 25
   :header-rows: 1

   * - Field
     - Description
   * - DataOffset
     - Data offset for use in the patch procedure
   * - SourceOffset
     - Data source offset for use in the patch procedure


Example BundleResourceGroup file
--------------------------------

.. code-block:: yaml

    Version: 0.1.0
    Type: BundleGroup
    NumberOfResources: 1
    TotalResourcesSizeCompressed: 8389
    TotalResourcesSizeUnCompressed: 41961
    ResourceGroupResource:
      RelativePath: ResourceGroup.yaml
      Type: ResourceGroup
      Location: cc/cc00d34500ea7a31_f32613169b3491afb5d029828d269ff3
      Checksum: f32613169b3491afb5d029828d269ff3
      UncompressedSize: 851
      CompressedSize: 363
    ChunkSize: 1000
    Resources:
      - RelativePath: CreateBundleOut/ResourceGroup0.chunk
        Type: BinaryChunk
        Location: 24/2445432734181d30_c6ffd7f72188cbf71e665c3714b5b148
        Checksum: c6ffd7f72188cbf71e665c3714b5b148
        UncompressedSize: 41961
        CompressedSize: 8389
       ...

BundleResourceGroup Additional Fields
-------------------------------------

Bundle Resource Groups extend Resource Groups and so contain all the base fields for a Resource Group.

.. list-table:: Document Fields
   :widths: 25 25
   :header-rows: 1

   * - Field
     - Description
   * - ResourceGroupResource
     - Resource information for a Resource Group containing resources that have been bundled