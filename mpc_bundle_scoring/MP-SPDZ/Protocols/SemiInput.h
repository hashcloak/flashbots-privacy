/*
 * SemiInput.h
 *
 */

#ifndef PROTOCOLS_SEMIINPUT_H_
#define PROTOCOLS_SEMIINPUT_H_

#include "ShamirInput.h"

template<class T> class SemiMC;

/**
 * Additive secret sharing input protocol
 */
template<class T>
class SemiInput : public IndividualInput<T>
{
    SeededPRNG secure_prng;

public:
    SemiInput(SubProcessor<T>& proc, SemiMC<T>& MC) :
            IndividualInput<T>(proc)
    {
        (void) MC;
    }

    SemiInput(SubProcessor<T>* proc, Player& P) :
            IndividualInput<T>(proc, P)
    {
    }

    SemiInput(typename T::MAC_Check& MC, Preprocessing<T>& prep, Player& P) :
            SemiInput(P)
    {
        (void) MC, (void) prep;
    }

    SemiInput(Player& P) :
            IndividualInput<T>(0, P)
    {
    }

    void add_mine(const typename T::clear& input, int n_bits = -1);
};

#endif /* PROTOCOLS_SEMIINPUT_H_ */
