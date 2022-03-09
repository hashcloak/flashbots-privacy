/*
 * ReplicatedPrivateOutput.h
 *
 */

#ifndef PROTOCOLS_REPLICATEDPRIVATEOUTPUT_H_
#define PROTOCOLS_REPLICATEDPRIVATEOUTPUT_H_

template<class T>
class SubProcessor;
template<class T>
class Share;

template <class T>
class ReplicatedPrivateOutput
{
    SubProcessor<T>& proc;

public:
    ReplicatedPrivateOutput(SubProcessor<T>& proc);

    void start(int player, int target, int source);
    void stop(int player, int source);
};

#endif /* PROTOCOLS_REPLICATEDPRIVATEOUTPUT_H_ */
