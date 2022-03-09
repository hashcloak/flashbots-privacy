/*
 * ReplicatedPrivateOutput.cpp
 *
 */

#include "ReplicatedPrivateOutput.h"
#include "Processor/Processor.h"
#include "Math/FixedVec.h"
#include "Math/Integer.h"

template<class T>
inline ReplicatedPrivateOutput<T>::ReplicatedPrivateOutput(
        SubProcessor<T>& proc) :
        proc(proc)
{
}

template<class T>
void ReplicatedPrivateOutput<T>::start(int player, int target,
        int source)
{
    (void)player, (void)target, (void)source;
    throw runtime_error("not implemented, use PrivateOutput");
}

template<class T>
void ReplicatedPrivateOutput<T>::stop(int player, int source)
{
    (void)player, (void)source;
}
