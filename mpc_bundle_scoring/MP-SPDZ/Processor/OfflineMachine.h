/*
 * DishonestMajorityOfflineMachine.h
 *
 */

#ifndef PROCESSOR_OFFLINEMACHINE_H_
#define PROCESSOR_OFFLINEMACHINE_H_

#include "OnlineMachine.h"
#include "Data_Files.h"
#include "BaseMachine.h"
#include "Networking/CryptoPlayer.h"

template<class W>
class OfflineMachine : public W
{
    DataPositions usage;
    BaseMachine machine;
    Names& playerNames;
    Player& P;

    template<class T>
    void generate();

public:
    template<class V>
    OfflineMachine(int argc, const char** argv,
            ez::ezOptionParser& opt, OnlineOptions& online_opts, V,
            int nplayers = 0);
    ~OfflineMachine();

    template<class T, class U>
    int run();
};

#endif /* PROCESSOR_OFFLINEMACHINE_H_ */
