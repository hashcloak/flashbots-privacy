/*
 * Client.h
 *
 */

#ifndef EXTERNALIO_CLIENT_H_
#define EXTERNALIO_CLIENT_H_

#include "Networking/ssl_sockets.h"

class Client
{
    vector<int> plain_sockets;
    ssl_ctx ctx;
    ssl_service io_service;

public:
    vector<ssl_socket*> sockets;
    octetStream specification;

    Client(const vector<string>& hostnames, int port_base, int my_client_id);
    ~Client();

    template<class T>
    void send_private_inputs(const vector<T>& values);

    template<class T>
    vector<T> receive_outputs(int n);
};

#endif /* EXTERNALIO_CLIENT_H_ */
