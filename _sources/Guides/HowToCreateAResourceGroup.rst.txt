How To Create A Resource Group
==============================

Resource Groups can be created via the lib or CLI. This example uses the CLI.

.. code::

    .\resources.exe create-group C:\Build --output-file ResourceGroup.yaml

**Arguments:**

1. Positional argument - Base directory to create resource group from.
2. ``--output-file`` - Filename for created resource group.

.. note::
    See CLI help for more information regarding options.

This will create a ``ResourceGroup.yaml`` file representing the input directory ``C:\Build``.

The resource group files are human readable yaml files and quite self explanatory. For more information see :doc:`../DesignDocuments/filesystemDesign`