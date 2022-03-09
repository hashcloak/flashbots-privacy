/*
 * SemiPrep.h
 *
 */

#ifndef GC_SEMIPREP_H_
#define GC_SEMIPREP_H_

#include "Protocols/ReplicatedPrep.h"
#include "OT/MascotParams.h"
#include "SemiSecret.h"
#include "ShiftableTripleBuffer.h"

template<class T> class Beaver;

namespace GC
{

template<class T> class ShareThread;

class SemiPrep : public BufferPrep<SemiSecret>, ShiftableTripleBuffer<SemiSecret>
{
    SemiSecret::TripleGenerator* triple_generator;
    MascotParams params;

    SeededPRNG secure_prng;

public:
    SemiPrep(DataPositions& usage, bool = true);
    ~SemiPrep();

    void set_protocol(Beaver<SemiSecret>& protocol);

    void buffer_triples();
    void buffer_bits();

    void buffer_squares() { throw not_implemented(); }
    void buffer_inverses() { throw not_implemented(); }

    void get(Dtype type, SemiSecret* data)
    {
        BufferPrep<SemiSecret>::get(type, data);
    }

    array<SemiSecret, 3> get_triple_no_count(int n_bits)
    {
        return ShiftableTripleBuffer<SemiSecret>::get_triple_no_count(n_bits);
    }

    void buffer_personal_triples(vector<array<SemiSecret, 3>>&, size_t, size_t)
    {
        throw not_implemented();
    }

    NamedCommStats comm_stats();
};

} /* namespace GC */

#endif /* GC_SEMIPREP_H_ */
