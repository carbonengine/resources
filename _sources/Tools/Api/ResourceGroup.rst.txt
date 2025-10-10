ResourceGroup
=============

The ResourceGroup is the central component of the Resources system.

ResourceGroups represent a collection of Resources.

They can be saved/loaded from a filetype which supersedes resfileindex files.

See :doc:`../../DesignDocuments/resourceGroupFileFormat` file specification for more details.

.. doxygenclass:: CarbonResources::ResourceGroup
    :members:


Input Parameters
----------------

.. doxygenstruct:: CarbonResources::ResourceGroupImportFromFileParams
    :members:

.. doxygenstruct:: CarbonResources::ResourceGroupExportToFileParams
    :members:

.. doxygenstruct:: CarbonResources::CreateResourceGroupFromDirectoryParams
    :members:

.. doxygenstruct:: CarbonResources::PatchCreateParams
    :members:

.. doxygenstruct:: CarbonResources::BundleCreateParams
    :members:

.. doxygenstruct:: CarbonResources::ResourceGroupMergeParams
    :members:

.. doxygenstruct:: CarbonResources::ResourceGroupDiffAgainstGroupParams
    :members:

.. doxygenstruct:: CarbonResources::ResourceGroupRemoveResourcesParams
    :members:

