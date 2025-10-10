Filesystem Design - Local/Remote
================================

resources works with 3 filesystem target types:

1. ``LOCAL_RELATIVE``
2. ``LOCAL_CDN``
3. ``REMOTE_CDN``

Most operations allow the user to specify which filesystem type the source/target files adhere to.

Operations also allow lots of mixing and matching for different inputs. For example patches may source previous build resources using ``LOCAL_RELATIVE`` but source latest from ``REMOTE_CDN``.

LOCAL_RELATIVE
--------------

Reading/Writing resources using ``LOCAL_RELATIVE`` type will read/write files by following the resources RelativePath field specified in the ResourceGroup.

This is a regular file path:

``e.g. A file at c:/build/folder1/hello.txt``

If build is thought of as the base path would get a relative path in the form:

``folder1/hello.txt``

LOCAL_CDN
---------

Reading/Writing resources using ``LOCAL_CDN`` type will read/write files by following the resources Location field specified in the ResourceGroup.

The location field is generated form checksums of the relative path and its data.

``[First two chars of hashed name]/[Hashed Name]_[Data Checksum]``

Hashed Name is generated from the Relative Path entry using Fowler-Noll-Vo hash function.

Data checksum is an md5 of the resource data

``e.g. cc/cc00d34500ea7a31_f32613169b3491afb5d029828d269ff3``

Files created this way mean that if a files content changes the filename also changes, even if the files relative path stayed the same.

REMOTE_CDN
----------

Reading/Writing file resources using ``REMOTE_CDN`` do slightly different things, however in the future this may change if uploading is added to resources.

Reading resources using ``REMOTE_CDN`` will download the resource following the same naming rules as ``LOCAL_CDN``.

Writing resources using ``REMOTE_CDN`` will save the resource in the same form as ``LOCAL_CDN`` but also carries out a compression. Resources are then ready to upload to a CDN.