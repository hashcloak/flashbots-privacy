/*
 * Rep2kPrep.h
 *
 */

#ifndef PROTOCOLS_REPLICATEDPREP2K_H_
#define PROTOCOLS_REPLICATEDPREP2K_H_

#include "ReplicatedPrep.h"

/**
 * Preprocessing for three-party replicated secret sharing modulo a power of two
 */
template<class T>
class ReplicatedPrep2k : public virtual SemiHonestRingPrep<T>,
        public virtual ReplicatedRingPrep<T>
{
public:
    ReplicatedPrep2k(SubProcessor<T>* proc, DataPositions& usage) :
            BufferPrep<T>(usage), BitPrep<T>(proc, usage),
			RingPrep<T>(proc, usage),
            SemiHonestRingPrep<T>(proc, usage), ReplicatedRingPrep<T>(proc, usage)
    {
    }

    void buffer_bits() { this->buffer_bits_without_check(); }

    void get_dabit_no_count(T& a, typename T::bit_type& b)
    {
        this->get_one_no_count(DATA_BIT, a);
        b = a & 1;
    }
};

#endif /* PROTOCOLS_REPLICATEDPREP2K_H_ */
