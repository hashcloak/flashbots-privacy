/*
 * MalRepRingPrep.cpp
 *
 */

#ifndef PROTOCOlS_MALREPRINGPREP_HPP_
#define PROTOCOlS_MALREPRINGPREP_HPP_

#include "MalRepRingPrep.h"
#include "MaliciousRepPrep.h"
#include "MalRepRingOptions.h"
#include "ShuffleSacrifice.h"
#include "Processor/OnlineOptions.h"

#include "ShuffleSacrifice.hpp"

template<class T>
MalRepRingPrep<T>::MalRepRingPrep(SubProcessor<T>*, DataPositions& usage) :
        BufferPrep<T>(usage)
{
}

template<class T>
RingOnlyBitsFromSquaresPrep<T>::RingOnlyBitsFromSquaresPrep(SubProcessor<T>*,
        DataPositions& usage) :
        BufferPrep<T>(usage)
{
}

template<class T>
SimplerMalRepRingPrep<T>::SimplerMalRepRingPrep(SubProcessor<T>* proc,
        DataPositions& usage) :
        BufferPrep<T>(usage), MalRepRingPrep<T>(proc, usage),
        RingOnlyBitsFromSquaresPrep<T>(proc, usage)
{
}

template<class T>
MalRepRingPrepWithBits<T>::MalRepRingPrepWithBits(SubProcessor<T>* proc,
        DataPositions& usage) :
        BufferPrep<T>(usage), BitPrep<T>(proc, usage),
        RingPrep<T>(proc, usage),
        MaliciousDabitOnlyPrep<T>(proc, usage),
        MaliciousRingPrep<T>(proc, usage), MalRepRingPrep<T>(proc, usage),
        RingOnlyBitsFromSquaresPrep<T>(proc, usage),
        SimplerMalRepRingPrep<T>(proc, usage)
{
}

template<class T>
void MalRepRingPrep<T>::buffer_triples()
{
    if (MalRepRingOptions::singleton.shuffle)
        shuffle_buffer_triples();
    else
        simple_buffer_triples();
}

template<class T>
void MalRepRingPrep<T>::buffer_squares()
{
    typedef typename T::prep_type prep_type;
    DataPositions _;
    MaliciousRepPrep<prep_type> prep(_);
    assert(this->proc != 0);
    prep.init_honest(this->proc->P);
    prep.buffer_size = this->buffer_size;
    prep.buffer_squares();
    for (auto& x : prep.squares)
        this->squares.push_back({{x[0], x[1]}});
    prep.squares.clear();
}

template<class T>
void MalRepRingPrep<T>::simple_buffer_triples()
{
    typedef typename T::prep_type prep_type;
    DataPositions _;
    MaliciousRepPrep<prep_type> prep(_);
    assert(this->proc != 0);
    prep.init_honest(this->proc->P);
    prep.buffer_size = this->buffer_size;
    prep.buffer_triples();
    for (auto& x : prep.triples)
        this->triples.push_back({{x[0], x[1], x[2]}});
    prep.triples.clear();
}

template<class T>
void MalRepRingPrep<T>::shuffle_buffer_triples()
{
    assert(T::SECURITY <= 40);
    assert(this->proc != 0);
    typename T::MAC_Check MC;
    shuffle_triple_generation(this->triples, this->proc->P, MC);
}

template<class T>
void shuffle_triple_generation(vector<array<T, 3>>& triples, Player& P,
        typename T::MAC_Check& MC, int n_bits = -1, ThreadQueues* queues = 0)
{
    RunningTimer timer;
    TripleShuffleSacrifice<T> sacrifice;
    vector<array<T, 3>> check_triples;
    int buffer_size = sacrifice.minimum_n_inputs(OnlineOptions::singleton.batch_size);

    // optimistic triple generation
    Replicated<T> protocol(P);
    generate_triples(check_triples, buffer_size, &protocol, n_bits);

#ifdef VERBOSE_SHUFFLE
    double gen_time = timer.elapsed();
    cerr << "Triple generation took " << gen_time << " seconds" << endl;
#endif

    sacrifice.triple_sacrifice(triples, check_triples, P, MC, queues);

#ifdef VERBOSE_SHUFFLE
    cerr << "Triple sacrifice took " << timer.elapsed() - gen_time << " seconds" << endl;
    cerr << "Total shuffle triple generation took " << timer.elapsed() << " seconds" << endl;
#endif
}

template<class U>
void ShuffleSacrifice::shuffle(vector<U>& check_triples, Player& P)
{
    int buffer_size = check_triples.size();

    // shuffle
    GlobalPRNG G(P);
    for (int i = 0; i < buffer_size; i++)
    {
        int remaining = buffer_size - i;
        int pos = G.get_uint(remaining);
        swap(check_triples[i], check_triples[i + pos]);
    }
}

template<class T>
TripleShuffleSacrifice<T>::TripleShuffleSacrifice()
{
}

template<class T>
TripleShuffleSacrifice<T>::TripleShuffleSacrifice(int B, int C) :
        ShuffleSacrifice(B, C)
{
}

template<class T>
void TripleShuffleSacrifice<T>::triple_sacrifice(vector<array<T, 3>>& triples,
        vector<array<T, 3>>& check_triples, Player& P,
        typename T::MAC_Check& MC, ThreadQueues* queues)
{
    int buffer_size = check_triples.size();
    size_t N = (buffer_size - C) / B;

    shuffle(check_triples, P);

    // opening C triples
    vector<T> shares;
    for (int i = 0; i < C; i++)
    {
        for (int j = 0; j < 3; j++)
            shares.push_back(check_triples.back()[j]);
        check_triples.pop_back();
    }
    vector<typename T::open_type> opened;
    MC.POpen(opened, shares, P);
    for (int i = 0; i < C; i++)
        if (typename T::clear(opened[3 * i] * opened[3 * i + 1]) != opened[3 * i + 2])
            throw Offline_Check_Error("shuffle opening");

    // triples might be same as check_triples
    if (triples.size() < N)
        triples.resize(N);

    if (queues)
    {
        TripleSacrificeJob job(&triples, &check_triples);
        int start = queues->distribute(job, N);
        triple_sacrifice(triples, check_triples, P, MC, start, N);
        queues->wrap_up(job);
    }
    else
        triple_sacrifice(triples, check_triples, P, MC, 0, N);

    triples.resize(N);
}

template<class T>
void TripleShuffleSacrifice<T>::triple_sacrifice(vector<array<T, 3>>& triples,
        vector<array<T, 3>>& check_triples, Player& P,
        typename T::MAC_Check& MC, int begin, int end)
{
#ifdef VERBOSE_SHUFFLE
    cerr << "sacrificing triples " << begin << " to " << end << endl;
#endif
    // sacrifice buckets
    vector<T> masked;
    int buffer_size = check_triples.size();
    int N = buffer_size / B;
    int size = end - begin;
    masked.reserve(2 * size);
    assert(size_t(end * B) <= check_triples.size());
    for (int i = begin; i < end; i++)
    {
        T& a = check_triples[i][0];
        T& b = check_triples[i][1];
        for (int j = 1; j < B; j++)
        {
            T& f = check_triples[i + N * j][0];
            T& g = check_triples[i + N * j][1];
            masked.push_back(a - f);
            masked.push_back(b - g);
        }
    }
    vector<typename T::open_type> opened;
    MC.POpen(opened, masked, P);
    auto it = opened.begin();
    vector<T> checks;
    checks.reserve(2 * size);
    for (int i = begin; i < end; i++)
    {
        T& b = check_triples[i][1];
        T& c = check_triples[i][2];
        for (int j = 1; j < B; j++)
        {
            T& f = check_triples[i + N * j][0];
            T& h = check_triples[i + N * j][2];
            typename T::open_type& rho = *(it++);
            typename T::open_type& sigma = *(it++);
            checks.push_back(c - h - b * rho - f * sigma);
        }
        triples[i] = check_triples[i];
    }
    MC.CheckFor(0, checks, P);
}

template<class T>
void RingOnlyBitsFromSquaresPrep<T>::buffer_bits()
{
    auto proc = this->proc;
    assert(proc != 0);
    typedef typename T::SquareToBitShare BitShare;
    typename BitShare::MAC_Check MC;
    DataPositions usage;
    typename BitShare::SquarePrep prep(0, usage);
    SubProcessor<BitShare> bit_proc(MC, prep, proc->P);
    prep.set_proc(&bit_proc);
    bits_from_square_in_ring(this->bits, this->buffer_size, &prep);
}

template<class T>
void MaliciousRingPrep<T>::buffer_edabits(bool strict, int n_bits,
        ThreadQueues* queues)
{
    RunningTimer timer;
#ifndef NONPERSONAL_EDA
    this->buffer_edabits_from_personal(strict, n_bits, queues);
#else
    assert(this->proc != 0);
    ShuffleSacrifice<T> shuffle_sacrifice;
    typedef typename T::bit_type::part_type bit_type;
    vector<vector<bit_type>> bits;
    vector<T> sums;
    this->buffer_edabits_without_check(n_bits, sums, bits,
            shuffle_sacrifice.minimum_n_inputs(), queues);
    vector<edabit<T>>& checked = this->edabits[{strict, n_bits}];
    shuffle_sacrifice.edabit_sacrifice(checked, sums, bits,
            n_bits, *this->proc, strict, -1, queues);
    if (strict)
        this->sanitize(checked, n_bits, -1, queues);
#endif
#ifdef VERBOSE_EDA
    cerr << "Total edaBit generation took " << timer.elapsed() << " seconds" << endl;
#endif
}

template<class T>
void MalRepRingPrep<T>::buffer_inputs(int player)
{
    this->buffer_inputs_as_usual(player, this->proc);
}

#endif
