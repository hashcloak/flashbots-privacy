/*
 * PrivateOutput.h
 *
 */

#ifndef PROCESSOR_PRIVATEOUTPUT_H_
#define PROCESSOR_PRIVATEOUTPUT_H_

#include <deque>
using namespace std;

template<class T> class SubProcessor;

template<class T>
class PrivateOutput
{
    typedef typename T::open_type open_type;

    SubProcessor<T>& proc;
    deque<open_type> masks;

public:
    PrivateOutput(SubProcessor<T>& proc) : proc(proc) { };

    void start(int player, int target, int source);
    void stop(int player, int dest, int source);

    T start(int player, const T& source);
    typename T::clear stop(int player, const typename T::clear& masked);
};

#endif /* PROCESSOR_PRIVATEOUTPUT_H_ */
