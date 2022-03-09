/*
 * PrivateOutput.cpp
 *
 */

#include "PrivateOutput.h"
#include "Processor.h"

template<class T>
void PrivateOutput<T>::start(int player, int target, int source)
{
    proc.get_S_ref(target) = start(player, proc.get_S_ref(source));
}

template<class T>
T PrivateOutput<T>::start(int player, const T& source)
{
    assert (player < proc.P.num_players());
    open_type mask;
    T res;
    proc.DataF.get_input(res, mask, player);
    res += source;

    if (player == proc.P.my_num())
        masks.push_back(mask);

    return res;
}

template<class T>
void PrivateOutput<T>::stop(int player, int dest, int source)
{
    auto& value = proc.get_C_ref(dest);
    value = stop(player, proc.get_C_ref(source));
    if (proc.Proc)
        value.output(proc.Proc->private_output, false);
}

template<class T>
typename T::clear PrivateOutput<T>::stop(int player, const typename T::clear& source)
{
    typename T::clear value;
    if (player == proc.P.my_num())
    {
        value = source - masks.front();
        masks.pop_front();
    }
    return value;
}
