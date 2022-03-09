/*
 * MalicousRepParty.h
 *
 */

#ifndef GC_SHARETHREAD_H_
#define GC_SHARETHREAD_H_

#include "Thread.h"
#include "MaliciousRepSecret.h"
#include "RepPrep.h"
#include "SemiHonestRepPrep.h"
#include "Processor/Data_Files.h"
#include "Protocols/ReplicatedInput.h"

#include <array>

namespace GC
{

template<class T>
class ShareThread
{
    static thread_local ShareThread<T>* singleton;

public:
    static ShareThread& s();

    Player* P;
    typename T::MC* MC;
    typename T::Protocol* protocol;

    Preprocessing<T>& DataF;

    ShareThread(Preprocessing<T>& prep);
    ShareThread(Preprocessing<T>& prep, Player& P,
            typename T::mac_key_type mac_key);
    virtual ~ShareThread();

    virtual typename T::MC* new_mc(typename T::mac_key_type mac_key)
    { return T::new_mc(mac_key); }

    void pre_run(Player& P, typename T::mac_key_type mac_key);
    void post_run();

    void and_(Processor<T>& processor, const vector<int>& args, bool repeat);
    void xors(Processor<T>& processor, const vector<int>& args);
};

template<class T>
class StandaloneShareThread : public ShareThread<T>, public Thread<T>
{
public:
    DataPositions usage;

    StandaloneShareThread(int i, ThreadMaster<T>& master);
    ~StandaloneShareThread();

    void pre_run();
    void post_run() { ShareThread<T>::post_run(); }

    NamedCommStats comm_stats()
    { return Thread<T>::comm_stats() + this->DataF.comm_stats(); }
};

template<class T>
thread_local ShareThread<T>* ShareThread<T>::singleton = 0;

template<class T>
inline ShareThread<T>& ShareThread<T>::s()
{
    if (singleton and T::is_real)
        return *singleton;
    else
        throw runtime_error("no singleton");
}

} /* namespace GC */

#endif /* GC_SHARETHREAD_H_ */
