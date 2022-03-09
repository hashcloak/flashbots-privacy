/*
 * TripleMachine.h
 *
 */

#ifndef OT_TRIPLEMACHINE_H_
#define OT_TRIPLEMACHINE_H_

#include "Math/gf2n.h"
#include "Math/gfp.h"
#include "Math/Z2k.h"
#include "OT/OTTripleSetup.h"
#include "OT/MascotParams.h"

class GeneratorThread;

class TripleMachine : public OfflineMachineBase, public MascotParams
{
    Names N[2];
    int nConnections;

    gf2n mac_key2;
    gfpvar1 mac_keyp;
    Z2<128> mac_keyz;

    bigint prime;

public:
    int nloops;
    bool bonding;
    int z2k, z2s;

    TripleMachine(int argc, const char** argv);

    template<class T>
    GeneratorThread* new_generator(OTTripleSetup& setup, int i,
            typename T::mac_key_type mac_key);

    void run();
};

#endif /* OT_TRIPLEMACHINE_H_ */
