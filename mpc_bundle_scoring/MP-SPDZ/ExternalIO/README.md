The ExternalIO directory contains an example of managing I/O between external client processes and SPDZ parties running SPDZ engines. These instructions assume that SPDZ has been built as per the [project readme](../README.md).

## Working Examples

[bankers-bonus-client.cpp](./bankers-bonus-client.cpp) acts as a
client to [bankers_bonus.mpc](../Programs/Source/bankers_bonus.mpc)
and demonstrates sending input and receiving output as described by
[Damgård et al.](https://eprint.iacr.org/2015/1006) The computation
allows up to eight clients to input a number and computes the client
with the largest input. You can run it as follows from the main
directory:
```
make bankers-bonus-client.x
./compile.py bankers_bonus 1
Scripts/setup-ssl.sh <nparties>
Scripts/setup-clients.sh 3
Scripts/<protocol>.sh bankers_bonus-1 &
./bankers-bonus-client.x 0 <nparties> 100 0 &
./bankers-bonus-client.x 1 <nparties> 200 0 &
./bankers-bonus-client.x 2 <nparties> 50 1
```
This should output that the winning id is 1. Note that the ids have to
be incremental, and the client with the highest id has to input 1 as
the last argument while the others have to input 0 there. Furthermore,
`<nparties>` refers to the number of parties running the computation
not the number of clients, and `<protocol>` can be the name of
protocol script. The setup scripts generate the necessary SSL
certificates and keys. Therefore, if you run the computation on
different hosts, you will have to distribute the `*.pem` files.

## I/O MPC Instructions

### Connection Setup

**listen**(*int port_num*)

Setup a socket server to listen for client connections. Runs in own thread so once created clients will be able to connect in the background.

*port_num* - the port number to listen on.

**acceptclientconnection**(*regint client_socket_id*, *int port_num*)

Picks the first available client socket connection. Blocks if none available.

*client_socket_id* - an identifier used to refer to the client socket.

*port_num* - the port number identifies the socket server to accept connections on.

### Data Exchange

Only the sint methods are documented here, equivalent methods are available for the other data types **cfix**, **cint** and **regint**. See implementation details in [types.py](../Compiler/types.py).

*[sint inputs]* **sint.read_from_socket**(*regint client_socket_id*, *int number_of_inputs*)

Read a share of an input from a client, blocking on the client send.

*client_socket_id* - an identifier used to refer to the client socket.

*number_of_inputs* - the number of inputs expected

*[inputs]* - returned list of shares of private input.

**sint.write_to_socket**(*regint client_socket_id*, *[sint values]*, *int message_type*)

Write shares of values including macs to an external client.

*client_socket_id* - an identifier used to refer to the client socket.

*[values]* - list of shares of values to send to client.

*message_type* - optional integer which will be sent in first 4 bytes of message, to indicate message type to client.

See also sint.write_shares_to_socket where macs can be explicitly included or excluded from the message.

*[sint inputs]* **sint.receive_from_client**(*int number_of_inputs*, *regint client_socket_id*, *int message_type*)

Receive shares of private inputs from a client, blocking on client send. This is an abstraction which first sends shares of random values to the client and then receives masked input from the client, using the input protocol introduced in [Confidential Benchmarking based on Multiparty Computation. Damgard et al.](http://eprint.iacr.org/2015/1006.pdf)

*number_of_inputs* - the number of inputs expected

*client_socket_id* - an identifier used to refer to the client socket.

*message_type* - optional integer which will be sent in first 4 bytes of message, to indicate message type to client.

*[inputs]* - returned list of shares of private input.
