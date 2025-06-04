API
===

The carbon-resources API is centered around the concept of a ResourceGroup. Where a ResourceGroup holds a collection of Resources.

There are extensions to this type, see BundleResourceGroup and PatchResourceGroup which are targetted at specific data types chunks and patchs respectively.

ResourceGroups can be imported and exported to file. See filetype section.

Global variables
----------------

.. doxygenvariable:: S_LIBRARY_VERSION

.. doxygenvariable:: S_DOCUMENT_VERSION

Structures
----------

.. toctree::
   :maxdepth: 1
   
   Api/ResourceGroup
   Api/BundleResourceGroup
   Api/PatchResourceGroup
   Api/SupplementaryStructures