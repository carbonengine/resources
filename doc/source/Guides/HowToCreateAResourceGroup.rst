How To Create A Resource Group
==============================

Resource Groups can be created via the lib or CLI. This example uses the CLI.

.. code::

    .\resources.exe create-group C:\Build --output-file ResourceGroup.yaml --document-version 0.1.0 --filter-file C:\Build\Resources\FolderForFilters\filterRules1.ini --filter-file-prefixmap-basepath C:\Build\Resources

**Arguments:**

1. Positional argument - Base directory to create resource group from.
2. ``--output-file`` - Filename for created resource group.
3. ``--document-version`` - The type of resource group to create/output. Valid values are: default=0.1.0 (yaml) and 0.0.0 (csv).
4. ``--filter-file`` - Absolute path to a .ini file containing include/exclude resource filtering rules. If not set = no filtering. Can be specified multiple times to combine filter rules from multiple files. See :doc:`../DesignDocuments/filterIniFiles` for more information on resource filter .ini files.
5. ``--filter-file-prefixmap-basepath`` - The absolute base directory for resolving relative paths contained within the supplied filter .ini file(s) **prefixmap** attribute. Ignored if the filter-file argument is not supplied.

.. note::
    See CLI help for more information regarding options.

The example CLI operation from the above example will:

* Create an output ``ResourceGroup.yaml`` file of the default document version 0.1.0 (yaml) format.
* Representing the contents of the input directory ``C:\Build``
* Limiting it based on include/exclude filter rules specified in ``C:\Build\Resources\FolderForFilters\filterRules1.ini``.
* While resolving relative **prefixmap** paths within it using the supplied ``C:\Build\Resources`` base path.


The resource group files (in document-version 0.1.0) are human readable yaml files and quite self explanatory. For more information see :doc:`../DesignDocuments/filesystemDesign`
