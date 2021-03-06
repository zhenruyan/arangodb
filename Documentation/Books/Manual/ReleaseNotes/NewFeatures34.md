Features and Improvements
=========================


Streaming AQL Cursors
------------------

It is now possible to create AQL query cursors with the new *stream* option.
Specify *true* and the query will be executed in a **streaming** fashion. The query result is
not stored on the server, but calculated on the fly. *Beware*: long-running queries will
need to hold the collection locks for as long as the query cursor exists.
When set to *false* the query will be executed right away in its entirety. 
In that case query results are either returned right away (if the resultset is small enough),
or stored on the arangod instance and accessible via the cursor API. It is advisable
to *only* use this option on short-running queries *or* without exclusive locks (write locks on MMFiles).
Please note that the query options `cache`, `count` and `fullCount` will not work on streaming
queries. Additionally query statistics, warnings and profiling data will only be available
after the query is finished. 
The default value is *false*