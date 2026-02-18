Filter .ini File Format
==========================

When generating Resource Groups it is possible to control the included/excluded files and folders, with the help of a resource filter .ini file(s).


Example Filter .ini file
------------------------

.. code-block:: ini

    # =============== This is a comment ===============
    # Every filter .ini file needs the following:
    # - [DEFAULT] section containing:
    #   - "prefixmap" attribute (mandatory):
    #      This is a space separated list of prefixes and their associated relative paths (semicolon separated).
    #      Format is: prefixA:path1;path2 prefixB:path3 (see actual example below)
    #
    # - [OneOrMoreNamedSections] containing:
    #   - "filter" attribute (optional):
    #      A top-level include/exclude filter ruleset that is applied to any
    #      "respaths" and "resfile" attribute element within this [NamedSection].
    #      Format is: [ .includeExtension1 IncludeFullOrPartialFileOrFolderName ]
    #                 ![ .excludeExtension1 ExcludeFullOrPartialFileOrFolderName ]
    #      If there is no filter attribute specified (i.e. optional), then wild-card include all is implied ([ * ]).
    #   - "respaths" attribute (mandatory, multi-line):
    #      A multi-line list of resource path entries, with or without wild-cards (* or ...).
    #      The paths are relative to their basepath prefix from the "prefixmap".
    #      Each line is a separate resource path entry, with or without an optional in-line filter.
    #      Search paths are resolved for each entry using a lookup from the "prefixmap".
    #      Supported wildcards are:
    #      - * to match any file within the current folder
    #      - ... to match any file or folder recursively from the current folder
    #      Format is: prefix:some/path/possible_wildcard [ OptionalExtraInclude ] ![ OptionalExtraExclude ]
    #   - "resfile" attribute (optional, single-line):
    #      Identical to a "respaths" entry, but for a single file path entry only.
    #      Only supported for backwards compatibility of older filter .ini files.
    #      Any single, specific file entries can (and should) be represented in the "respaths" attribute instead.
    ; =============== This is also a comment ===============

    [DEFAULT]
    prefixmap = res:res;../common/res resbin:../bin

    [NamedSection1]
    filter   = [ .yaml .txt ]
    respaths = res:/...
               res:/SomeFolder/... ![ AlsoExclude ]
               res:/SomeOtherFolder/* [ .red ]
               resbin:/* [ .dll ] ![ .yaml .txt ]
               res:/SomeFolder/FolderWithAlsoExcludeName/includeThisFile.csv ]

    # =============== For this example CLI create-group call ===============
    # resources.exe
    #     create-group C:\Build
    #     --output-file ResourceGroup.yaml
    #     --document-version 0.1.0
    #     --filter-file C:\Build\Resources\FolderForFilters\filterRules1.ini
    #     --filter-file-prefixmap-basepath C:\Build\Resources
    # ================ The following rules would be applied ================
    # 1. Any .yaml or .txt file within:
    #    "res:/..."
    #      - C:\Build\Resources\res (and its subfolders "...") would be included.
    #      - C:\Build\common\res (and its subfolders "...") would be included.
    # 2. Unless the .yaml or .txt file is within:
    #    "res:/SomeFolder/..." and "AlsoExclude" is part of its name, then it would be excluded.
    #      - C:\Build\Resources\res\SomeFolder (and its subfolders "...")
    #      - C:\Build\common\res\SomeFolder (and its subfolders "...")
    # 3. Additionally, any file within:
    #    "res:/SomeOtherFolder/*" would be included, but only if it is a .red file.
    #      - C:\Build\Resources\res\SomeOtherFolder (only this folder)
    #      - C:\Build\common\res\SomeOtherFolder (only this folder)
    # 4. Then any .dll file within:
    #    "resbin:/*" only .dll files would be included, excluding .yaml and .txt files.
    #      - C:\Build\bin (only this folder)
    # 5. Finally, we should include this specific file rule:
    #    "res:/SomeFolder/FolderWithAlsoExcludeName/includeThisFile.csv"
    #      - C:\Build\Resources\res\SomeFolder\FolderWithAlsoExcludeName\includeThisFile.csv
    #      - C:\Build\common\res\SomeFolder\FolderWithAlsoExcludeName\includeThisFile.csv
    # ======================================================================
    ...
