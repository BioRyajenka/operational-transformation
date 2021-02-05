Operation Transformation
====
This repository contains highly-efficient implementation
of [Operation Transformation algorithm](https://en.wikipedia.org/wiki/Operational_transformation), 
which was originally developed in Jupyter and later was used in Google Wave.

Current algorithm is tested using simulation of client-server
interoperation in online document editing.

Some metrics
----
OT algorithm has many variations and implementations, here is mine.
I believe that asymptotically this is the most optimal variation both
in memory and CPU consumption.
It has linear time on processing and on memory usage for each operation.

### operations/sec ###
Maximum operations throughput for each client doesn't depend on the document size. 

For 20 clients and server, all running on my laptop (AMD Ryzen 9 4900HS 3.00 GHz), it is about
`832.140 ops/sec` on average. For 2000 clients and server, it is about `8.610 ops/sec`. 
Note that this measurements are given for "simple" history engine (see below). 

### memory consumption ###
Server needs 24 byte for each symbol: 4 byte for id, 4 byte for value and 16 bytes for
references to the next and to the previous symbols. So, for document consisting of 10^7 
numbers, server will consume `~240mb` of RAM.

Client needs same 24 bytes plus around 40 bytes for metadata. So, client memory consumption 
will be around `~640mb` for 10^7-sized document.

Note that during simulation many additional data need to be stored for each client. 
Additional structure ("magic_list.h" in code) is needed to generate random positions in document.
This structure will also take about 24 bytes for each symbol, so, during simulation it will also
require additional `~240mb` for each client.  

History engines
----
There are two "history engines" implemented: *simple* history engine and *jumping* history engine.
They have the following characteristics:
* *simple* engine allows to quickly add operations to history (**O(1)** time complexity), but it is slow 
  on history retrieval (**O(n)** for each retrieval, where **n** is an amount of data missed by the
  client from his last sync with server)
* on the other hand, *jumping* history engine is slower on operations addition (**O(nlogn)** time), 
but it is much faster on retrieving operations from history: **O(logn)** time for retrieval.
  

How to run the code
----
You can use cmake to run the simulation and tests. *src/testing/client_server_one_threaded_tests.cpp* is an entrypoint for simulation.

See CMakeLists.txt files for additional information.
