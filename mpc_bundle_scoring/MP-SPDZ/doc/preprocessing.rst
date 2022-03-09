Preprocessing
-------------

Many protocols in MP-SPDZ use preprocessing, that is, producing secret
shares that are independent of the actual data but help with the
computation. Due to the independence, this can be done in batches to
save communication rounds and even communication when using
homomorphic encryption that works with large vectors such as LWE-based
encryption.

Generally, preprocessing is done on demand and per computation
threads. On demand means that batches of preprocessing data are
computed whenever there is none in storage, and a computation thread
is a thread created by control flow instructions such as
:py:func:`~Compiler.library.for_range_multithread`.

The exceptions to the general rule are edaBit generation with
malicious security and AND triples with malicious security and honest
majority, both when use bucket size three. Bucket size three implies
batches of over a million to achieve 40-bit statistical security, and
in honest-majority binary computation the item size is 64, which makes
the actual batch size 64 million triples. In multithreaded programs,
the preprocessing is run centrally using the threads as helpers.

The batching means that the cost in terms of time and communication
jump whenever another batch is generated. Note that, while some
protocols are flexible with the batch size and can thus be controlled
using ``-b``, others mandate a batch size, which can be as large as a
million.
