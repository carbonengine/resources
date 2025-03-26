ResourceGroup
=============

The ResourceGroup is the central component of the Resources system.

ResourceGroups represent a collection of Resources.

They can be saved/loaded from a filetype which superseeds resfileindex files.

See ResourceGroup file specification for more details.

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

.. doxygenenum:: CarbonResources::ResourceSourceType

.. doxygenstruct:: CarbonResources::ResourceSourceSettings
    :members:

.. doxygenenum:: CarbonResources::ResourceDestinationType

.. doxygenstruct:: CarbonResources::ResourceDestinationSettings
    :members:

.. doxygenstruct:: CarbonResources::BundleCreateParams
    :members: