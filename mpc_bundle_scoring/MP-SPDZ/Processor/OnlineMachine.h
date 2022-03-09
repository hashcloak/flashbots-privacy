/*
 * DishonestMajorityMachine.h
 *
 */

#ifndef PROCESSOR_ONLINEMACHINE_H_
#define PROCESSOR_ONLINEMACHINE_H_

#include "Processor/OnlineOptions.h"
#include "Math/gf2n.h"
#include "Networking/Player.h"

class OnlineMachine
{
protected:
    int argc;
    const char** argv;
    OnlineOptions& online_opts;

    int lg2, opening_sum, max_broadcast;

    Names playerNames;

    bool use_encryption, receive_threads;

    ez::ezOptionParser& opt;

    int nplayers;

public:
    template<class V = gf2n>
    OnlineMachine(int argc, const char** argv, ez::ezOptionParser& opt,
            OnlineOptions& online_opts, int nplayers = 0, V = {});

    void start_networking();

    template<class T, class U>
    int run();

    Player* new_player(const string& id_base);
};

class DishonestMajorityMachine : public OnlineMachine
{
public:
    template<class V>
    DishonestMajorityMachine(int argc, const char** argv,
            ez::ezOptionParser& opt, OnlineOptions& online_opts, V,
            int nplayers = 0);
    DishonestMajorityMachine(int argc, const char** argv,
            ez::ezOptionParser& opt, OnlineOptions& online_opts, int nplayers);
};

#endif /* PROCESSOR_ONLINEMACHINE_H_ */
