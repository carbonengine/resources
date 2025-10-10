Installation
============

resources supports Windows and macOS.

Dependencies are sourced via `VCPKG <https://learn.microsoft.com/en-us/vcpkg/>`_


Building
--------

1. Get submodules

.. code::

  git submodule update --init

2. At the top level of your project (alongside your CMakeLists.txt file) create a file called: `CMakeUserPresets.json`
3. Copy the contents of the `CMakePresets.json` file into `CMakeUserPresets.json` and delete everything from the `configurePresets` array
4. Add a new preset into the `configurePresets` array:

.. code-block:: json

  {
    "name": "local",
    // inherit all of the settings from this preset in CMakePresets.json
    "inherits": "carbon_windows_vcpkg_vs",
    "environment": {
      // put any other environment variables that you want here
      "CCP_EVE_PERFORCE_BRANCH_PATH": "D:/perforce/eve-frontier/branches/sandbox/VCPKG-MIGRATION"
    },
    "cacheVariables": {
      // put any extra cache variables that you want in here
    }
  }

.. note::

  set `inherits` to `carbon_osx_vcpkg` for MacOS development

5. Configure and building using CMakeLists provided in repository root.


.. list-table:: Available build options
   :widths: 25 75
   :header-rows: 1

   * - Cmake option
     - Description
   * - BUILD_DOCUMENTATION
     - Build the documentation.
   * - DEV_FEATURES
     - Setting to true will build the extended CLI. This version of the CLI exposes more operations.
   

See :doc:`guides` and :doc:`tools` for usage information