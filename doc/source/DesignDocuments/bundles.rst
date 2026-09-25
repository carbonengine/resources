Bundles
#######

To efficiently transfer file data across a network, resources combines and splits file data into evenly sized 'chunks'. This collection of 'chunks' is what is referred to as a 'Bundle'.

The two data split methods
--------------------------
resources offers two main appraoches to the splitting of the file data:

1. Split on compressed size
2. Split on uncompressed size

By default resources will split on the compressed size, but this can be changed via argument both through CLI and lib.

Each setting will give vastly different results and it is important to understand the implications of both.

Split on compressed size
------------------------
This is the default setting and will create the most efficent bundles in terms of size at the cost of increased processing time during creation.

When chunking files that will ultimately be compressed on the network, it makes the most sense to use the compressed size of the files as they are added to the chunk.
Files may have vastly different sizes between compressed and uncompressed size. So if splitting on uncompressed size it is possible to create chunks that would eventually
compress to a very small size. This scenario is very likely for patch files for example. This could lead to tiny chunk files that have very small file sizes.

This process is sequential by nature so calculation time is expensive as it leaves little room for parallelism. This means the process can take considerably longer than the alternative (split on uncompressed).

Split on uncompressed size
--------------------------
Splitting on uncompressed size may be preferred if bundle creation time is a concern. The process will complete in a far shorter time but the caveat is that more chunks will be created and these will be less size efficient.

This operation doesn't need to calculate compression as part of chunking and so can defer the compression calculation to a parallel job to achieve much better performance.

This approach is best used for files which have much less compression oportunity than patch files. Although it will be less efficient, it may be acceptible compared to the time tradeoff.

It is also possible to completely skip compression calculation during bundling. This method gives the ultimate processing speed and may make sense depending on the how the data will be processed after this step.