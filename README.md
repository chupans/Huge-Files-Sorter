Huge-Files-Sorter
=================

Test task for Artek

Task: Sort in ascending order binary file containing unsigned 32-bit integers, 
size of the file is more than addressable memory.

Provided with binary file path via argument it sorts it using 2x file size disk space.
Currently implemented only windows version.
Requires Boost library to be compiled.
Best speed results can be achieved if compiled by 64 bit compiler, but 32 bit compiler will do the work too.

In general, algorithm can be described by following steps:
1. Prepare buffers: 1 if compiled by 32bit compiler or equal to amount of cores of central processor if used 64bit
2. Read chunks from binary files to buffers
3. Sort those buffers as asynchronously
4. Save each buffer into temp file.
5. If binary file contains unread values goto 2.
6. Merge all temp files into one result file.
