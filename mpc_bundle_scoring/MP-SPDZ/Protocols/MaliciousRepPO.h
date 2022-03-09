/*
 * MaliciousRepPO.h
 *
 */

#ifndef PROTOCOLS_MALICIOUSREPPO_H_
#define PROTOCOLS_MALICIOUSREPPO_H_

#include "Networking/Player.h"

template<class T>
class MaliciousRepPO
{
    Player& P;
    octetStream to_send;
    octetStream to_receive[2];

public:
    MaliciousRepPO(Player& P);

    void prepare_sending(const T& secret, int player);
    void send(int player);
    void receive();
    typename T::clear finalize(const T& secret);
};

#endif /* PROTOCOLS_MALICIOUSREPPO_H_ */
