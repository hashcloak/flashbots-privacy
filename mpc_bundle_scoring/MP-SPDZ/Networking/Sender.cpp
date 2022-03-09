/*
 * Sender.cpp
 *
 */

#include "Sender.h"
#include "ssl_sockets.h"

template<class T>
void* Sender<T>::run_thread(void* sender)
{
    ((Sender<T>*)sender)->run();
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    OPENSSL_thread_stop();
#endif
    return 0;
}

template<class T>
Sender<T>::Sender(T socket) : socket(socket), thread(0)
{
    start();
}

template<class T>
Sender<T>::~Sender()
{
    stop();
}

template<class T>
void Sender<T>::start()
{
    pthread_create(&thread, 0, run_thread, this);
}

template<class T>
void Sender<T>::stop()
{
    in.stop();
    pthread_join(thread, 0);
}

template<class T>
void Sender<T>::run()
{
    const octetStream* os = 0;
    while (in.pop(os))
    {
//        timer.start();
        os->Send(socket);
//        timer.stop();
        out.push(os);
    }
}

template<class T>
void Sender<T>::request(const octetStream& os)
{
    in.push(&os);
}

template<class T>
void Sender<T>::wait(const octetStream& os)
{
    const octetStream* queued = 0;
    out.pop(queued);
    if (queued != &os)
      throw not_implemented();
}

template class Sender<int>;
template class Sender<ssl_socket*>;
