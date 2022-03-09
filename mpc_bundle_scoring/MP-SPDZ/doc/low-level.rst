Low-Level Interface
===================

In the following we will explain the basic of the C++ interface by
walking trough :file:`Utils/paper-example.cpp`.

.. default-domain:: cpp

.. code-block:: cpp

    template<class T>
    void run(char** argv, int prime_length);

MP-SPDZ heavily uses templating to allow to reuse code between
different protocols. :cpp:func:`run` is a simple example of this.  The
entire virtual machine in the :file:`Processor` directory is built on
the same principle. The central type is a type representing a share in
a particular type.

.. code-block:: cpp

    // bit length of prime
    const int prime_length = 128;

    // compute number of 64-bit words needed
    const int n_limbs = (prime_length + 63) / 64;

Computation modulo a prime requires to fix the number of limbs (64-bit
words) at compile time. This allows for optimal memory usage and
computation.

.. code-block:: cpp

    if (protocol == "MASCOT")
        run<Share<gfp_<0, n_limbs>>>(argv, prime_length);
    else if (protocol == "CowGear")
        run<CowGearShare<gfp_<0, n_limbs>>>(argv, prime_length);

Share types for computation module a prime (and in
:math:`\mathrm{GF}(2^n)`) generally take one parameter for the
computation domain. :class:`gfp_` in turn takes two parameters, a
counter and the number of limbs. The counter allows to use several
instances with different parameters. It can be chosen freely, but the
convention is to use 0 for the online phase and 1 for the offline
phase where required.

.. code-block:: cpp

    else if (protocol == "SPDZ2k")
        run<Spdz2kShare<64, 64>>(argv, 0);

Share types for computation modulo a power of two simply take the
exponent as parameter, and some take an additional security parameter.

.. code-block:: cpp

    int my_number = atoi(argv[1]);
    int n_parties = atoi(argv[2]);
    int port_base = 9999;
    Names N(my_number, n_parties, "localhost", port_base);

All implemented protocols require point-to-point connections between
all parties. :class:`Names` objects represent a setup of hostnames and
IPs used to set up the actual
connections. The chosen initialization provides a way where
every party connects to party 0 on a specified location (localhost in
this case), which then broadcasts the locations of all parties. The
base port number is used to derive the port numbers for the parties to
listen on (base + party number). See the the :class:`Names` class for
other possibilities such as a text file containing hostname and port
number for each party.

.. code-block:: cpp

    CryptoPlayer P(N);

The networking setup is used to set up the actual
connections. :class:`CryptoPlayer` uses encrypted connection while
:class:`PlainPlayer` does not. If you use several instances (for
several threads for example), you must use an integer identifier as
the second parameter, which must differ from any other by at least the
number of parties.

.. code-block:: cpp

    // initialize fields
    T::clear::init_default(prime_length);

We have to use a specific prime for computation modulo a prime. This
deterministically generates one of the desired length if
necessary. For computation modulo a power of two, this does not do
anything.

.. code-block:: cpp

    T::clear::next::init_default(prime_length, false);

For computation modulo a prime, it is more efficient to use Montgomery
representation, which is not compatible with the MASCOT offline phase
however. This line initializes another field instance for MASCOT
without using Montgomery representation.

.. code-block:: cpp

    // must initialize MAC key for security of some protocols
    typename T::mac_key_type mac_key;
    T::read_or_generate_mac_key("", P, mac_key);

Some protocols use an information-theoretic tag that is constant
throughout the protocol. This codes reads it from storage if available
or generates a fresh one otherwise.

.. code-block:: cpp

    // global OT setup
    BaseMachine machine;
    if (T::needs_ot)
        machine.ot_setups.push_back({P});

Many protocols for a dishonest majority use oblivious transfer. This
block runs a few instances to seed the oblivious transfer
extension. The resulting setup only works for one thread. For several
threads, you need to add sufficiently many instances to
:member:`ot_setups` and set :member:`BaseMachine::thread_num`
(thread-local) to a different consecutive number in every thread.

.. code-block:: cpp

    // keeps tracks of preprocessing usage (triples etc)
    DataPositions usage;
    usage.set_num_players(P.num_players());

To help keeping track of the required preprocessing, it is necessary
to initialize preprocessing instances with a :class:`DataPositions`
variable that will store the usage.

.. code-block:: cpp

    // initialize binary computation
    T::bit_type::mac_key_type::init_field();
    typename T::bit_type::mac_key_type binary_mac_key;
    T::bit_type::part_type::read_or_generate_mac_key("", P, binary_mac_key);
    GC::ShareThread<typename T::bit_type> thread(N,
            OnlineOptions::singleton, P, binary_mac_key, usage);

While this example only uses arithmetic computation, you need to
initialize binary computation as well unless you use the compile-time
option ``NO_MIXED_CIRCUITS``.

.. code-block:: cpp

    // output protocol
    typename T::MAC_Check output(mac_key);

Some output protocols use the MAC key to check the correctness.

.. code-block:: cpp

    // various preprocessing
    typename T::LivePrep preprocessing(0, usage);
    SubProcessor<T> processor(output, preprocessing, P);

In this example we use live preprocessing, but it is also possible to
read preprocessing data from disk by using :class:`Sub_Data_Files<T>`
instead. You can use a live preprocessing instances to generate
preprocessing data independently, but many protocols require that a
:class:`SubProcessor<T>` instance has been created as well. The latter
essentially glues an instance of the output and the preprocessing
protocol together, which is necessary for Beaver-based multiplication
protocols.

.. code-block:: cpp

    // input protocol
    typename T::Input input(processor, output);

Some input protocols depend on preprocessing and an output protocol,
which is reflect in the standard constructor. Other constructors are
available depending on the protocol.

.. code-block:: cpp

    // multiplication protocol
    typename T::Protocol protocol(P);

This instantiates a multiplication protocol. :var:`P` is required
because some protocols start by exchanging keys for pseudo-random
secret sharing.

.. code-block:: cpp

    int n = 1000;
    vector<T> a(n), b(n);
    T c;
    typename T::clear result;

Remember that :type:`T` stands for a share in the protocol. The
derived type :type:`T::clear` stands for the cleartext domain. Share
types support linear operations such as addition, subtraction, and
multiplication with a constant. Use :func:`T::constant` to convert a
constant to a share type.

.. code-block:: cpp

    input.reset_all(P);
    for (int i = 0; i < n; i++)
        input.add_from_all(i);
    input.exchange();
    for (int i = 0; i < n; i++)
    {
        a[i] = input.finalize(0);
        b[i] = input.finalize(1);
    }

The interface for all protocols proceeds in four stages:

1. Initialization. This is required to initialize and reset data
   structures in consecutive use.
2. Local data preparation
3. Communication
4. Output extraction

This blueprint allows for a minimal number of communication rounds.

.. code-block:: cpp

    protocol.init_dotprod(&processor);
    for (int i = 0; i < n; i++)
        protocol.prepare_dotprod(a[i], b[i]);
    protocol.next_dotprod();
    protocol.exchange();
    c = protocol.finalize_dotprod(n);

The initialization of the multiplication sets the preprocessing and
output instances to use in Beaver multiplication. :func:`next_dotprod`
separates dot products in the data preparation phase.

.. code-block:: cpp

    output.init_open(P);
    output.prepare_open(c);
    output.exchange(P);
    result = output.finalize_open();

    cout << "result: " << result << endl;
    output.Check(P);

The output protocol follows the same blueprint except that it is
necessary to call the checking in order to verify the outputs.

.. code-block:: cpp

    T::LivePrep::teardown();

This frees the memory used for global key material when using homomorphic
encryption. Otherwise, this does not do anything.


Domain Types
------------

.. list-table::
   :widths: 20 80

   *
     - ``gfp_<X, L>``
     - Computation modulo a prime. ``L`` is the number of 64-bit
       limbs, that is, it covers primes of bit length
       :math:`64(L-1)+1` to :math:`64L`. The type has to be
       initialized using :cpp:func:`init_field` or
       :cpp:func:`init_default`. The latter picks a prime given a bit length.
   *
     - ``SignedZ2<K>`` / ``Z2<K>``
     - Computation modulo :math:`2^K`. This is not a field.
   *
     - ``gf2n_short`` / ``gf2n_long`` / ``gf2n_<T>``
     - :math:`GF(2^n)`. ``T`` denotes a type that is used to store the
       values. It must support a variety of integer
       operations. The type has to be initialized using
       :cpp:func:`init_field`. The choice of degrees is limited. At
       the time of writing, 4, 8, 28, 40, 63, and 128 are supported if the
       storage type is large enough.

Share Types
------------

.. list-table::
   :widths: 20 80
   :header-rows: 1

   *
     - Type
     - Protocol
   *
     - ``AtlasShare<T>``
     - Semi-honest version of `ATLAS
       <https://eprint.iacr.org/2021/833>`_ (Section 4.2). ``T`` must
       represent a field.
   *
     - ``ChaiGearShare<T>``
     - `HighGear <https://eprint.iacr.org/2017/1230>`_ with covert key
       setup. ``T`` must be ``gfp_<X, L>`` or ``gf2n_short``.
   *
     - ``CowGearShare<T>``
     - `LowGear <https://eprint.iacr.org/2017/1230>`_ with covert key
       setup. ``T`` must be ``gfp_<X, L>`` or ``gf2n_short``.
   *
     - ``HemiShare<T>``
     - Semi-honest protocol with Beaver multiplication based on
       semi-homomorphic encryption. ``T`` must be ``gfp_<X, L>`` or
       ``gf2n_short``.
   *
     - ``HighGearShare<T>``
     - `HighGear <https://eprint.iacr.org/2017/1230>`_. ``T`` must be
       ``gfp_<X, L>`` or ``gf2n_short``.
   *
     - ``LowGearShare<T>``
     - `LowGear <https://eprint.iacr.org/2017/1230>`_. ``T`` must be
       ``gfp_<X, L>`` or ``gf2n_short``.
   *
     - ``MaliciousShamirShare<T>``
     - Shamir secret sharing with Beaver multiplication and sacrifice.
       ``T`` must represent a field.
   *
     - ``MamaShare<T, N>``
     - `MASCOT <https://eprint.iacr.org/2016/505>`_ with multiple
       MACs. ``T`` must represent a field, ``N`` is the number of MACs.
   *
     - ``PostSacriRepFieldShare<T>``
     - `Post-sacrifice <https://eprint.iacr.org/2017/816>`_ protocol
       using three-party replicated secret sharing with ``T``
       representing a field.
   *
     - ``PostSacriRepRingShare<K, S>``
     - `Post-sacrifice protocol <https://eprint.iacr.org/2019/164>`_
       using replicated three-party secret sharing modulo :math:`2^K`
       with security parameter ``S``.
   *
     - ``Rep3Share2<K>``
     - `Three-party semi-honest protocol
       <https://eprint.iacr.org/2016/768>`_ using replicated secret
       sharing modulo :math:`2^K`.
   *
     - ``Rep4Share<T>``
     - `Four-party malicious protocol
       <https://eprint.iacr.org/2020/1330>`_ using replicated secret
       sharing over a field.
   *
     - ``Rep4Share2<K>``
     - `Four-party malicious protocol
       <https://eprint.iacr.org/2020/1330>`_ using replicated secret
       sharing modulo :math:`2^K`.
   *
     - ``SemiShare2<K>``
     - Semi-honest dishonest-majority protocol using Beaver
       multiplication based on oblivious transfer modulo :math:`2^K`.
   *
     - ``SemiShare<T>``
     - Semi-honest dishonest-majority protocol using Beaver
       multiplication based on oblivious transfer in a field.
   *
     - ``ShamirShare<T>``
     - `Semi-honest protocol <https://eprint.iacr.org/2000/037>`_
       based on Shamir's secret sharing. ``T`` must represent a field.
   *
     - ``Share<T>``
     - `MASCOT <https://eprint.iacr.org/2016/505>`_. ``T`` must
       represent a field.
   *
     - ``SohoShare<T>``
     - Semi-honest protocol with Beaver multiplication based on
       somewhat homomorphic encryption. ``T`` must be ``gfp_<X, L>``
       or ``gf2n_short``.
   *
     - ``Spdz2kShare<K, S>``
     - `SPDZ2k <https://eprint.iacr.org/2018/482>`_ computing modulo
       :math:`2^K` with security parameter ``S``.
   *
     - ``SpdzWiseShare<K, S>``
     - `SPDZ-wise <https://eprint.iacr.org/2019/1298>`_ computing
       modulo :math:`2^K` with security parameter ``S``.
   *
     - ``SpdzWiseShare<T>``
     - `SPDZ-wise <https://eprint.iacr.org/2018/570>`_. ``T`` must be
       ``MaliciousShamirShare`` or ``MaliciousRep3Share``.


Protocol Interfaces
-------------------

.. doxygenclass:: ProtocolBase
   :members:

.. doxygenclass:: InputBase
   :members:

.. doxygenclass:: MAC_Check_Base
   :members:

.. doxygenclass:: Preprocessing
   :members:

.. doxygenclass:: BufferPrep
   :members:
