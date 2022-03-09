/*
 * MamPrep.cpp
 *
 */

#include "MamaPrep.h"

#include "SemiMC.hpp"

template<class T>
MamaPrep<T>::MamaPrep(SubProcessor<T>* proc, DataPositions& usage) :
        BufferPrep<T>(usage), BitPrep<T>(proc, usage),
        RingPrep<T>(proc, usage),
        MaliciousDabitOnlyPrep<T>(proc, usage),
        OTPrep<T>(proc, usage),
        MaliciousRingPrep<T>(proc, usage)
{
    this->params.amplify = true;
    this->params.generateMACs = true;
    this->params.check = false;
}

template<class T>
void MamaPrep<T>::buffer_triples()
{
    int mac_security = T::N_MACS * T::clear::length();

    if (mac_security < 40)
    {
        cerr << T::N_MACS << " MACs are not enough for 40-bit security with "
                << T::clear::length() << "-bit primes." << endl;
        cerr << "Compile with -DN_MAMA_MACS="
                << DIV_CEIL(40, T::clear::length())
                << " or remove this check in " << __FILE__ << endl;
        exit(1);
    }

    auto& triple_generator = this->triple_generator;
    assert(triple_generator != 0);
    assert(this->proc != 0);
    this->params.generateBits = false;
    vector<array<T, 3>> triples;
    TripleShuffleSacrifice<T> sacrifice;
    size_t required = OnlineOptions::singleton.batch_size;

    // prefer shuffling if not loosing much security and bucket size is smaller
    bool use_shuffling = mac_security <= 42
            and OnlineOptions::singleton.bucket_size < T::N_MACS;
    if (use_shuffling)
        required = sacrifice.minimum_n_inputs();

    while (triples.size() < required)
    {
        triple_generator->generateTriples();
        triple_generator->unlock();
        for (auto& x : triple_generator->uncheckedTriples)
        {
            triples.push_back({});
            for (int k = 0; k < 3; k++)
                triples.back()[k] = x.byIndex(k, 0);
        }
        cerr << "Got " << triple_generator->uncheckedTriples.size()
                << " triples" << endl;
    }

    if (use_shuffling)
        sacrifice.triple_sacrifice(triples, triples, this->proc->P,
                this->proc->MC);
    else
    {
        auto& proc = this->proc;
        auto& P = proc->P;
        const unsigned n_sacrifice = T::N_MACS - 1;
        vector<array<array<T, 3>, n_sacrifice>> check_triples;
        while (n_sacrifice <= triples.size())
        {
            check_triples.push_back({});
            for (unsigned i = 0; i < n_sacrifice; i++)
            {
                check_triples.back()[i] = triples.back();
                triples.pop_back();
            }
        }
        auto t = GlobalPRNG(P).get<typename T::clear>();
        vector<T> masked;
        PointerVector<typename T::clear> opened;
        for (auto& x : check_triples)
            for (unsigned i = 1; i < n_sacrifice; i++)
            {
                masked.push_back(t * x[0][0] - x[i][0]);
                masked.push_back(x[0][1] - x[i][1]);
            }
        proc->MC.POpen(opened, masked, P);
        vector<T> checks;
        for (auto& x : check_triples)
        {
            triples.push_back(x[0]);
            for (unsigned i = 1; i < n_sacrifice; i++)
            {
                auto rho = opened.next();
                auto sigma = opened.next();
                checks.push_back(
                        t * x[0][2] - x[i][2] - x[i][1] * rho - x[i][0] * sigma
                                - T::constant(sigma * rho, P.my_num(),
                                        proc->MC.get_alphai()));
            }
        }
        proc->MC.CheckFor(0, checks, P);
    }

    this->triples = triples;
}
