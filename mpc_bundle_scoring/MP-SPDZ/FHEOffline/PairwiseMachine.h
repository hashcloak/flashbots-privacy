/*
 * PairwiseMachine.h
 *
 */

#ifndef FHEOFFLINE_PAIRWISEMACHINE_H_
#define FHEOFFLINE_PAIRWISEMACHINE_H_

#include "FHEOffline/PairwiseGenerator.h"
#include "FHEOffline/SimpleMachine.h"
#include "FHEOffline/PairwiseSetup.h"

class PairwiseMachine : public MachineBase
{
public:
    PairwiseSetup<FFT_Data> setup_p;
    PairwiseSetup<P2Data> setup_2;
    Player& P;

    vector<FHE_PK> other_pks;
    FHE_PK& pk;
    FHE_SK sk;
    vector<Ciphertext> enc_alphas;

    PairwiseMachine(Player& P);
    PairwiseMachine(int argc, const char** argv);

    void init();

    template <class FD>
    void setup_keys();

    template <class T>
    void set_mac_key(T alphai);

    template <class FD>
    PairwiseSetup<FD>& setup();

    void pack(octetStream& os) const;
    void unpack(octetStream& os);

    void check(Player& P) const;
};

#endif /* FHEOFFLINE_PAIRWISEMACHINE_H_ */
