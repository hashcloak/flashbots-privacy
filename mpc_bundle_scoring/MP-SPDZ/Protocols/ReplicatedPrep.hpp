/*
 * ReplicatedPrep.cpp
 *
 */

#ifndef PROTOCOlS_REPLICATEDPREP_HPP_
#define PROTOCOlS_REPLICATEDPREP_HPP_

#include "ReplicatedPrep.h"
#include "DabitSacrifice.h"
#include "Spdz2kPrep.h"

#include "GC/BitAdder.h"
#include "Processor/OnlineOptions.h"
#include "Protocols/Rep3Share.h"

#include "MaliciousRingPrep.hpp"
#include "ShuffleSacrifice.hpp"
#include "GC/ShareThread.hpp"
#include "GC/BitAdder.hpp"

class InScope
{
    bool& variable;
    bool backup;

public:
    InScope(bool& variable, bool value) :
            variable(variable)
    {
        backup = variable;
        variable = value;
    }
    ~InScope()
    {
        variable = backup;
    }
};

template<class T>
BufferPrep<T>::BufferPrep(DataPositions& usage) :
        Preprocessing<T>(usage), n_bit_rounds(0),
		proc(0),
        buffer_size(OnlineOptions::singleton.batch_size)
{
}

template<class T>
BufferPrep<T>::~BufferPrep()
{
    string type_string = T::type_string();

#ifdef VERBOSE
    if (n_bit_rounds > 0)
        cerr << n_bit_rounds << " rounds of random " << type_string
                << " bit generation" << endl;
#endif

    if (OnlineOptions::singleton.verbose)
    {
        this->print_left("triples", triples.size() * T::default_length,
                type_string);

#define X(KIND) \
    this->print_left(#KIND, KIND.size(), type_string);
        X(squares)
        X(inverses)
        X(bits)
        X(dabits)
#undef X

        for (auto& x : this->edabits)
        {
            this->print_left_edabits(x.second.size(), x.second[0].size(),
                    x.first.first, x.first.second);
        }
    }
}

template<class T>
BitPrep<T>::BitPrep(SubProcessor<T>* proc, DataPositions& usage) :
        BufferPrep<T>(usage), base_player(0), protocol(0)
{
    this->proc = proc;
}

template<class T>
RingPrep<T>::RingPrep(SubProcessor<T>* proc, DataPositions& usage) :
        BufferPrep<T>(usage), BitPrep<T>(proc, usage), bit_part_proc(0)
{
}

template<class T>
RingPrep<T>::~RingPrep()
{
    if (bit_part_proc)
        delete bit_part_proc;
}

template<class T>
void BitPrep<T>::set_protocol(typename T::Protocol& protocol)
{
    this->protocol = new typename T::Protocol(protocol.branch());
    auto proc = this->proc;
    if (proc and proc->Proc)
        this->base_player = proc->Proc->thread_num;
}

template<class T>
BitPrep<T>::~BitPrep()
{
    if (protocol)
    {
        protocol->check();
        delete protocol;
    }
}

template<class T>
void BufferPrep<T>::clear()
{
    triples.clear();
    inverses.clear();
    bits.clear();
    squares.clear();
    inputs.clear();
}

template<class T>
void ReplicatedRingPrep<T>::buffer_triples()
{
    assert(this->protocol != 0);
    // independent instance to avoid conflicts
    typename T::Protocol protocol(this->protocol->branch());
    generate_triples(this->triples, OnlineOptions::singleton.batch_size,
            &protocol);
}

template<class T, class U>
void generate_triples(vector<array<T, 3>>& triples, int n_triples,
        U* protocol, int n_bits = -1)
{
    protocol->init_mul();
    generate_triples_initialized(triples, n_triples, protocol, n_bits);
}

template<class T, class U>
void generate_triples_initialized(vector<array<T, 3>>& triples, int n_triples,
        U* protocol, int n_bits = -1)
{
    triples.resize(n_triples);
    for (size_t i = 0; i < triples.size(); i++)
    {
        auto& triple = triples[i];
        triple[0] = protocol->get_random();
        triple[1] = protocol->get_random();
        protocol->prepare_mul(triple[0], triple[1], n_bits);
    }
    protocol->exchange();
    for (size_t i = 0; i < triples.size(); i++)
        triples[i][2] = protocol->finalize_mul(n_bits);
}

template<class T>
void BufferPrep<T>::get_three_no_count(Dtype dtype, T& a, T& b, T& c)
{
    if (dtype != DATA_TRIPLE)
        throw not_implemented();

    if (triples.empty())
    {
        buffer_triples();
        assert(not triples.empty());
    }

    a = triples.back()[0];
    b = triples.back()[1];
    c = triples.back()[2];
    triples.pop_back();
}

template<class T>
void BitPrep<T>::buffer_squares()
{
    auto proc = this->proc;
    auto buffer_size = this->buffer_size;
    assert(proc != 0);
    vector<T> a_plus_b(buffer_size), as(buffer_size), cs(buffer_size);
    T b;
    for (int i = 0; i < buffer_size; i++)
    {
        this->get_three_no_count(DATA_TRIPLE, as[i], b, cs[i]);
        a_plus_b[i] = as[i] + b;
    }
    vector<typename T::open_type> opened(buffer_size);
    proc->MC.POpen(opened, a_plus_b, proc->P);
    for (int i = 0; i < buffer_size; i++)
        this->squares.push_back({{as[i], as[i] * opened[i] - cs[i]}});
}

template<class T>
void ReplicatedRingPrep<T>::buffer_squares()
{
    generate_squares(this->squares, this->buffer_size,
            this->protocol, this->proc);
}

template<class T, class U>
void generate_squares(vector<array<T, 2>>& squares, int n_squares,
        U* protocol, SubProcessor<T>* proc)
{
    assert(protocol != 0);
    squares.resize(n_squares);
    protocol->init_mul(proc);
    for (size_t i = 0; i < squares.size(); i++)
    {
        auto& square = squares[i];
        square[0] = protocol->get_random();
        protocol->prepare_mul(square[0], square[0]);
    }
    protocol->exchange();
    for (size_t i = 0; i < squares.size(); i++)
        squares[i][1] = protocol->finalize_mul();
}

template<class T>
void BufferPrep<T>::buffer_inverses()
{
    buffer_inverses<0>(T::clear::invertible);
}

template<class T>
template<int>
void BufferPrep<T>::buffer_inverses(true_type)
{
    assert(proc != 0);
    auto& P = proc->P;
    auto& MC = proc->MC;
    auto& prep = *this;
    int buffer_size = OnlineOptions::singleton.batch_size;
    vector<array<T, 3>> triples(buffer_size);
    vector<T> c;
    for (int i = 0; i < buffer_size; i++)
    {
        prep.get_three_no_count(DATA_TRIPLE, triples[i][0], triples[i][1],
                triples[i][2]);
        c.push_back(triples[i][2]);
    }
    vector<typename T::open_type> c_open;
    MC.POpen(c_open, c, P);
    for (size_t i = 0; i < c.size(); i++)
        if (c_open[i] != 0)
            inverses.push_back({{triples[i][0], triples[i][1] / c_open[i]}});
    triples.clear();
    if (inverses.empty())
        throw runtime_error("products were all zero");
    MC.Check(P);
}

template<class T>
void BufferPrep<T>::get_two_no_count(Dtype dtype, T& a, T& b)
{
    switch (dtype)
    {
    case DATA_SQUARE:
    {
        if (squares.empty())
            buffer_squares();

        a = squares.back()[0];
        b = squares.back()[1];
        squares.pop_back();
        return;
    }
    case DATA_INVERSE:
    {
        while (inverses.empty())
            buffer_inverses();

        a = inverses.back()[0];
        b = inverses.back()[1];
        inverses.pop_back();
        return;
    }
    default:
        throw not_implemented();
    }
}

template<class T>
void XOR(vector<T>& res, vector<T>& x, vector<T>& y,
		typename T::Protocol& prot, SubProcessor<T>* proc)
{
    assert(x.size() == y.size());
    int buffer_size = x.size();
    res.resize(buffer_size);

    if (T::clear::field_type() == DATA_GF2N)
    {
        for (int i = 0; i < buffer_size; i++)
            res[i] = x[i] + y[i];
        return;
    }

    prot.init_mul(proc);
    for (int i = 0; i < buffer_size; i++)
        prot.prepare_mul(x[i], y[i]);
    prot.exchange();
    typename T::open_type two = typename T::open_type(1) + typename T::open_type(1);
    for (int i = 0; i < buffer_size; i++)
        res[i] = x[i] + y[i] - prot.finalize_mul() * two;
}

template<class T>
void buffer_bits_from_squares(RingPrep<T>& prep)
{
    auto proc = prep.get_proc();
    assert(proc != 0);
    auto& bits = prep.get_bits();
    vector<array<T, 2>> squares(prep.buffer_size);
    vector<T> s;
    for (int i = 0; i < prep.buffer_size; i++)
    {
        prep.get_two(DATA_SQUARE, squares[i][0], squares[i][1]);
        s.push_back(squares[i][1]);
    }
    vector<typename T::clear> open;
    proc->MC.POpen(open, s, proc->P);
    auto one = T::constant(1, proc->P.my_num(), proc->MC.get_alphai());
    for (size_t i = 0; i < s.size(); i++)
        if (open[i] != 0)
            bits.push_back((squares[i][0] / open[i].sqrRoot() + one) / 2);
    squares.clear();
    if (bits.empty())
        throw runtime_error("squares were all zero");
}

template<class T>
template<int>
void ReplicatedPrep<T>::buffer_bits(true_type)
{
    if (this->protocol->get_n_relevant_players() > 10
            or OnlineOptions::singleton.bits_from_squares)
        buffer_bits_from_squares(*this);
    else
        ReplicatedRingPrep<T>::buffer_bits();
}

template<class T>
void BitPrep<T>::buffer_bits_without_check()
{
    SeededPRNG G;
    buffer_ring_bits_without_check(this->bits, G, this->buffer_size);
}

template<class T>
void MaliciousRingPrep<T>::buffer_personal_dabits(int input_player)
{
    buffer_personal_dabits<0>(input_player, T::clear::characteristic_two,
            T::clear::prime_field);
}

template<class T>
template<int>
void MaliciousRingPrep<T>::buffer_personal_dabits(int, true_type, false_type)
{
    throw runtime_error("only implemented for integer-like domains");
}

template<class T>
template<int>
void MaliciousRingPrep<T>::buffer_personal_dabits(int input_player, false_type,
        false_type)
{
    assert(this->proc != 0);
    vector<dabit<T>> check_dabits;
    DabitSacrifice<T> dabit_sacrifice;
    this->buffer_personal_dabits_without_check<0>(input_player, check_dabits,
            dabit_sacrifice.minimum_n_inputs());
    dabit_sacrifice.sacrifice_and_check_bits(
            this->personal_dabits[input_player], check_dabits, *this->proc, 0);
}

template<class T>
template<int>
void MaliciousRingPrep<T>::buffer_personal_dabits(int input_player, false_type,
        true_type)
{
    if (T::clear::length() >= 60)
        buffer_personal_dabits<0>(input_player, false_type(), false_type());
    else
    {
        assert(this->proc != 0);
        vector<dabit<T>> check_dabits;
        DabitShuffleSacrifice<T> shuffle_sacrifice;
        this->buffer_personal_dabits_without_check<0>(input_player, check_dabits,
                shuffle_sacrifice.minimum_n_inputs());
        shuffle_sacrifice.dabit_sacrifice(this->personal_dabits[input_player],
                check_dabits, *this->proc, 0);
    }
}

template<class T>
template<int>
void MaliciousRingPrep<T>::buffer_personal_dabits_without_check(
        int input_player, vector<dabit<T>>& to_check, int buffer_size)
{
    assert(this->proc != 0);
    auto& P = this->proc->P;
    auto &party = GC::ShareThread<typename T::bit_type>::s();
    typedef typename T::bit_type::part_type BT;
    SubProcessor<BT> bit_proc(party.MC->get_part_MC(),
            this->proc->bit_prep, this->proc->P);
    typename T::Input input(*this->proc, this->proc->MC);
    typename BT::Input bit_input(bit_proc, bit_proc.MC);
    input.reset_all(P);
    bit_input.reset_all(P);
    SeededPRNG G;
    if (input_player == P.my_num())
    {
        for (int i = 0; i < buffer_size; i++)
        {
            auto bit = G.get_bit();
            bit_input.add_mine(bit, 1);
            input.add_mine(bit);
        }
    }
    else
        for (int i = 0; i < buffer_size; i++)
        {
            bit_input.add_other(input_player);
            input.add_other(input_player);
        }
    input.exchange();
    bit_input.exchange();
    for (int i = 0; i < buffer_size; i++)
        to_check.push_back({input.finalize(input_player),
                bit_input.finalize(input_player, 1)});
}

template<class T>
template<int>
void RingPrep<T>::buffer_personal_edabits_without_check(int n_bits,
        vector<T>& sums, vector<vector<BT> >& bits, SubProcessor<BT>& proc,
        int input_player, int begin, int end)
{
#ifdef VERBOSE_EDA
    fprintf(stderr, "generate personal edaBits %d to %d\n", begin, end);
#endif
    InScope in_scope(this->do_count, false);
    assert(this->proc != 0);
    auto& P = proc.P;
    typename T::Input input(*this->proc, this->proc->MC);
    typename BT::Input bit_input(proc, proc.MC);
    input.reset_all(P);
    bit_input.reset_all(P);
    SeededPRNG G;
    assert(begin % BT::default_length == 0);
    int buffer_size = end - begin;
    int n_chunks = DIV_CEIL(buffer_size, BT::default_length);
    if (input_player == P.my_num())
    {
        for (int i = 0; i < n_chunks; i++)
        {
            typename T::clear tmp[BT::default_length];
            for (int j = 0; j < n_bits; j++)
            {
                auto bits = G.get<typename BT::clear>();
                bit_input.add_mine(bits, BT::default_length);
                for (int k = 0; k < BT::default_length; k++)
                    tmp[k] += T::clear::power_of_two(bits.get_bit(k), j);
            }
            for (int k = 0; k < BT::default_length; k++)
                input.add_mine(tmp[k], n_bits);
        }
    }
    else
        for (int i = 0; i < n_chunks; i++)
        {
            for (int j = 0; j < n_bits; j++)
                bit_input.add_other(input_player);
            for (int i = 0; i < BT::default_length; i++)
                input.add_other(input_player);
        }
    input.exchange();
    bit_input.exchange();
    for (int i = 0; i < buffer_size; i++)
        sums[begin + i] = input.finalize(input_player);
    assert(bits.size() == size_t(n_bits));
    for (auto& x : bits)
        assert(x.size() >= size_t(end / BT::default_length));
    for (int i = 0; i < n_chunks; i++)
    {
        for (int j = 0; j < n_bits; j++)
            bits[j][begin / BT::default_length + i] =
                    bit_input.finalize(input_player, BT::default_length);
    }
}

template<class T>
void MaliciousRingPrep<T>::buffer_personal_edabits(int n_bits, vector<T>& wholes,
        vector<vector<BT> >& parts, SubProcessor<BT>& proc, int input_player,
        bool strict, ThreadQueues* queues)
{
#ifdef VERBOSE_EDA
    cerr << "Generate personal edaBits of length " << n_bits
            << " to sacrifice" << endl;
    Timer timer;
    timer.start();
#endif
    EdabitShuffleSacrifice<T> shuffle_sacrifice;
    int buffer_size = shuffle_sacrifice.minimum_n_inputs();
    vector<T> sums(buffer_size);
    vector<vector<BT>> bits(n_bits, vector<BT>(DIV_CEIL(buffer_size, BT::default_length)));
    if (queues)
    {
        ThreadJob job(n_bits, &sums, &bits, input_player);
        int start = queues->distribute(job, buffer_size, 0, BT::default_length);
        this->template buffer_personal_edabits_without_check<0>(n_bits, sums,
                bits, proc, input_player, start, buffer_size);
        queues->wrap_up(job);
    }
    else
        this->template buffer_personal_edabits_without_check<0>(n_bits, sums,
                bits, proc, input_player, 0, buffer_size);
#ifdef VERBOSE_EDA
    cerr << "Done with generating personal edaBits after " << timer.elapsed()
            << " seconds" << endl;
#endif
    vector<edabit<T>> edabits;
    shuffle_sacrifice.edabit_sacrifice(edabits, sums, bits, n_bits, *this->proc,
            strict, input_player, queues);
    wholes.clear();
    parts.clear();
    parts.resize(n_bits);
    for (size_t j = 0; j < edabits.size(); j++)
    {
        auto& x = edabits[j];
        wholes.push_back(x.first);
        for (int i = 0; i < n_bits; i++)
        {
            if (j % BT::default_length == 0)
                parts[i].push_back({});
            parts[i].back() ^= BT(x.second[i]) << (j % BT::default_length);
        }
    }
}

template<class T>
void buffer_bits_from_players(vector<vector<T>>& player_bits,
        PRNG& G, SubProcessor<T>& proc, int base_player,
        int buffer_size, int n_bits)
{
    auto& protocol = proc.protocol;
    auto& P = protocol.P;
    int n_relevant_players = protocol.get_n_relevant_players();
    player_bits.resize(n_relevant_players, vector<T>(buffer_size));
    auto& input = proc.input;
    input.reset_all(P);
    for (int i = 0; i < n_relevant_players; i++)
    {
        int input_player = (base_player + i) % P.num_players();
        if (input_player == P.my_num())
        {
            for (int i = 0; i < buffer_size; i++)
            {
                typename T::clear tmp;
                for (int j = 0; j < n_bits; j++)
                    tmp += typename T::clear(G.get_bit()) << j;
                input.add_mine(tmp, n_bits);
            }
        }
        else
            for (int i = 0; i < buffer_size; i++)
                input.add_other(input_player);
    }
    input.exchange();
    for (int i = 0; i < n_relevant_players; i++)
        for (auto& x : player_bits[i])
            x = input.finalize((base_player + i) % P.num_players(), n_bits);
#if !defined(__clang__) && (__GNUC__ == 6)
    // mitigate compiler bug
    Bundle<octetStream> bundle(P);
    P.unchecked_broadcast(bundle);
#endif
#ifdef DEBUG_BIT_SACRIFICE
    typename T::MAC_Check MC;
    for (int i = 0; i < n_relevant_players; i++)
        for (auto& x : player_bits[i])
            assert((MC.open(x, P) == 0) or (MC.open(x, P) == 1));
#endif
}

template<class T>
void BitPrep<T>::buffer_ring_bits_without_check(vector<T>& bits, PRNG& G,
        int buffer_size)
{
    auto proc = this->proc;
    assert(protocol != 0);
    assert(proc != 0);
    int n_relevant_players = protocol->get_n_relevant_players();
    vector<vector<T>> player_bits;
    auto stat = proc->P.comm_stats;
    buffer_bits_from_players(player_bits, G, *proc, this->base_player,
            buffer_size, 1);
    auto& prot = *protocol;
    XOR(bits, player_bits[0], player_bits[1], prot, proc);
    for (int i = 2; i < n_relevant_players; i++)
        XOR(bits, bits, player_bits[i], prot, proc);
    this->base_player++;
    (void) stat;
#ifdef VERBOSE_PREP
    cerr << "bit generation" << endl;
    (proc->P.comm_stats - stat).print(true);
#endif
}

template<class T>
void RingPrep<T>::buffer_dabits_without_check(vector<dabit<T>>& dabits,
        int buffer_size, ThreadQueues* queues)
{
    if (buffer_size < 0)
        buffer_size = OnlineOptions::singleton.batch_size;
    int old_size = dabits.size();
    dabits.resize(dabits.size() + buffer_size);
    if (queues)
    {
        ThreadJob job(&dabits);
        int start = queues->distribute(job, buffer_size, old_size);
        this->buffer_dabits_without_check(dabits,
                start, dabits.size());
        queues->wrap_up(job);
    }
    else
        buffer_dabits_without_check(dabits, old_size, dabits.size());
}

template<class T>
void RingPrep<T>::buffer_dabits_without_check(vector<dabit<T>>& dabits,
        size_t begin, size_t end)
{
    auto proc = this->proc;
    assert(proc != 0);
    buffer_dabits_without_check<0>(dabits, begin, end, proc->bit_prep);
}

template<class T>
template<int>
void RingPrep<T>::buffer_dabits_without_check(vector<dabit<T>>& dabits,
        size_t begin, size_t end,
        Preprocessing<typename T::bit_type::part_type>& bit_prep)
{
#ifdef VERBOSE_DABIT
    fprintf(stderr, "generate daBits %lu to %lu\n", begin, end);
#endif

    size_t buffer_size = end - begin;
    auto proc = this->proc;
    assert(this->protocol != 0);
    assert(proc != 0);
    SeededPRNG G;
    PRNG G2 = G;
    typedef typename T::bit_type::part_type bit_type;
    vector<vector<bit_type>> player_bits;
    auto& party = GC::ShareThread<typename T::bit_type>::s();
    SubProcessor<bit_type> bit_proc(party.MC->get_part_MC(),
            bit_prep, proc->P);
    buffer_bits_from_players(player_bits, G, bit_proc, this->base_player,
            buffer_size, 1);
    vector<T> int_bits;
    this->buffer_ring_bits_without_check(int_bits, G2, buffer_size);
    for (auto& pb : player_bits)
        assert(pb.size() == int_bits.size());
    for (size_t i = 0; i < int_bits.size(); i++)
    {
        bit_type bit = player_bits[0][i];
        for (int j = 1; j < this->protocol->get_n_relevant_players(); j++)
            bit ^= player_bits[j][i];
        dabits[begin + i] = {int_bits[i], bit};
    }
}

template<class T>
template<int>
void RingPrep<T>::buffer_edabits_without_check(int n_bits, vector<T>& sums,
        vector<vector<typename T::bit_type::part_type>>& bits, int buffer_size,
        ThreadQueues* queues)
{
    RunningTimer timer;
    int dl = T::bit_type::part_type::default_length;
    int rounded = DIV_CEIL(buffer_size, dl) * dl;
    sums.resize(rounded);
    bits.resize(rounded / dl);
    if (queues)
    {
        ThreadJob job(n_bits, &sums, &bits);
        int start = queues->distribute(job, rounded, 0, dl);
        buffer_edabits_without_check<0>(n_bits, sums, bits, start, rounded);
        queues->wrap_up(job);
    }
    else
        buffer_edabits_without_check<0>(n_bits, sums, bits, 0, rounded);
    sums.resize(buffer_size);
#ifdef VERBOSE_EDA
    cerr << "Done with unchecked edaBit generation after " << timer.elapsed()
            << " seconds" << endl;
#endif
}

template<class T>
template<int>
void RingPrep<T>::buffer_edabits_without_check(int n_bits, vector<T>& sums,
        vector<vector<typename T::bit_type::part_type>>& bits, int begin,
        int end)
{
    typedef typename T::bit_type::part_type bit_type;
    int dl = bit_type::default_length;
    assert(begin % dl == 0);
    assert(end % dl == 0);
    int buffer_size = end - begin;
    auto proc = this->proc;
    assert(this->protocol != 0);
    assert(proc != 0);
    auto &party = GC::ShareThread<typename T::bit_type>::s();
    if (bit_part_proc == 0)
        bit_part_proc = new SubProcessor<bit_type>(party.MC->get_part_MC(),
                proc->bit_prep, proc->P);
    auto& bit_proc = *bit_part_proc;
    int n_relevant = this->protocol->get_n_relevant_players();
    vector<vector<T>> player_ints(n_relevant, vector<T>(buffer_size));
    vector<vector<vector<bit_type>>> parts(n_relevant,
            vector<vector<bit_type>>(n_bits, vector<bit_type>(buffer_size / dl)));
    for (int i = 0; i < n_relevant; i++)
        buffer_personal_edabits_without_check<0>(n_bits, player_ints[i], parts[i],
                bit_proc, i, 0, buffer_size);
    vector<vector<vector<bit_type>>> player_bits(n_bits,
            vector<vector<bit_type>>(n_relevant));
    for (int i = 0; i < n_bits; i++)
        for (int j = 0; j < n_relevant; j++)
            player_bits[i][j] = parts[j][i];
    BitAdder().add(bits, player_bits, begin / dl, end / dl, bit_proc,
            bit_type::default_length, 0);
    for (int i = 0; i < buffer_size; i++)
    {
        T sum;
        for (auto& ints : player_ints)
            sum += ints[i];
        sums[begin + i] = sum;
    }
}

template<class T>
template<int>
void RingPrep<T>::buffer_edabits_without_check(int n_bits, vector<edabitvec<T>>& edabits,
        int buffer_size)
{
    auto stat = this->proc->P.comm_stats;
    typedef typename T::bit_type::part_type bit_type;
    vector<vector<bit_type>> bits;
    vector<T> sums;
    buffer_edabits_without_check<0>(n_bits, sums, bits, buffer_size);
    this->push_edabits(edabits, sums, bits, buffer_size);
    (void) stat;
#ifdef VERBOSE_PREP
    cerr << "edaBit generation" << endl;
    (proc->P.comm_stats - stat).print(true);
#endif
}

template<class T>
void BufferPrep<T>::push_edabits(vector<edabitvec<T>>& edabits,
        const vector<T>& sums, const vector<vector<typename T::bit_type::part_type>>& bits,
        int buffer_size)
{
    int unit = T::bit_type::part_type::default_length;
    edabits.reserve(edabits.size() + DIV_CEIL(buffer_size, unit));
    for (int i = 0; i < buffer_size; i++)
    {
        if (i % unit ==  0)
            edabits.push_back(bits.at(i / unit));
        edabits.back().push_a(sums[i]);
    }
}

template<class T>
template<int>
void RingPrep<T>::buffer_sedabits_from_edabits(int n_bits, false_type)
{
    assert(this->proc != 0);
    size_t buffer_size = OnlineOptions::singleton.batch_size;
    auto& loose = this->edabits[{false, n_bits}];
    while (loose.size() < size_t(DIV_CEIL(buffer_size, edabitvec<T>::MAX_SIZE)))
        this->buffer_edabits(false, n_bits);
    sanitize<0>(loose, n_bits);
    for (auto& x : loose)
    {
        this->edabits[{true, n_bits}].push_back(x);
    }
    loose.clear();
}

template<class T>
template<int>
void RingPrep<T>::sanitize(vector<edabit<T>>& edabits, int n_bits,
        int player, ThreadQueues* queues)
{
    if (queues)
    {
        SanitizeJob job(&edabits, n_bits, player);
        int start = queues->distribute(job, edabits.size());
        sanitize<0>(edabits, n_bits, player, start, edabits.size());
        queues->wrap_up(job);
    }
    else
        sanitize<0>(edabits, n_bits, player, 0, edabits.size());
}

template<class T>
template<int>
void RingPrep<T>::sanitize(vector<edabit<T>>& edabits, int n_bits, int player,
        int begin, int end)
{
#ifdef VERBOSE_EDA
    fprintf(stderr, "sanitize edaBits %d to %d in %d\n", begin, end,
        BaseMachine::thread_num);
#endif

    vector<T> dabits;
    typedef typename T::bit_type::part_type::small_type BT;
    vector<BT> to_open;
    for (int i = begin; i < end; i++)
    {
        auto& x = edabits[i];
        for (size_t j = n_bits; j < x.second.size(); j++)
        {
            T a;
            typename T::bit_type b;
            if (player < 0)
                this->get_dabit_no_count(a, b);
            else
                this->get_personal_dabit(player, a, b);
            dabits.push_back(a);
            to_open.push_back(x.second[j] + BT(b));
        }
    }
    vector<typename BT::open_type> opened;
    auto& MCB = *BT::new_mc(
            GC::ShareThread<typename T::bit_type>::s().MC->get_alphai());
    MCB.POpen(opened, to_open, this->proc->P);
    auto dit = dabits.begin();
    auto oit = opened.begin();
    for (int i = begin; i < end; i++)
    {
        auto& x = edabits[i];
        auto& whole = x.first;
        for (size_t j = n_bits; j < x.second.size(); j++)
        {
            auto& mask = *dit++;
            int masked = (*oit++).get();
            auto overflow = mask
                    + T::constant(masked, this->proc->P.my_num(),
                            this->proc->MC.get_alphai())
                    - mask * typename T::clear(masked * 2);
            whole -= overflow << j;
        }
        x.second.resize(n_bits);
    }
    MCB.Check(this->proc->P);
    delete &MCB;
}

template<class T>
template<int>
void RingPrep<T>::sanitize(vector<edabitvec<T>>& edabits, int n_bits)
{
    vector<T> dabits;
    typedef typename T::bit_type::part_type BT;
    vector<BT> to_open;
    for (auto& x : edabits)
    {
        for (size_t j = n_bits; j < x.b.size(); j++)
        {
            BT bits;
            for (size_t i = 0; i < x.size(); i++)
            {
                T a;
                typename T::bit_type b;
                this->get_dabit_no_count(a, b);
                dabits.push_back(a);
                bits ^= BT(b) << i;
            }
            to_open.push_back(x.b[j] + bits);
        }
    }
    vector<typename BT::open_type> opened;
    auto& MCB = *BT::new_mc(
            GC::ShareThread<typename T::bit_type>::s().MC->get_alphai());
    MCB.POpen(opened, to_open, this->proc->P);
    auto dit = dabits.begin();
    auto oit = opened.begin();
    for (auto& x : edabits)
    {
        for (size_t j = n_bits; j < x.b.size(); j++)
        {
            auto masked = (*oit++);
            for (size_t i = 0; i < x.size(); i++)
            {
                int masked_bit = masked.get_bit(i);
                auto& mask = *dit++;
                auto overflow = mask
                        + T::constant(masked_bit, this->proc->P.my_num(),
                                this->proc->MC.get_alphai())
                        - mask * typename T::clear(masked_bit * 2);
                x.a[i] -= overflow << j;
            }
        }
        x.b.resize(n_bits);
    }
    MCB.Check(this->proc->P);
    delete &MCB;
}

template<>
inline
void SemiHonestRingPrep<Rep3Share<gf2n>>::buffer_bits()
{
    assert(protocol != 0);
    bits_from_random(bits, *protocol);
}

template<class T>
void bits_from_random(vector<T>& bits, typename T::Protocol& protocol)
{
    while (bits.size() < (size_t)OnlineOptions::singleton.batch_size)
    {
        Rep3Share<gf2n> share = protocol.get_random();
        for (int j = 0; j < gf2n::degree(); j++)
        {
            bits.push_back(share & 1);
            share >>= 1;
        }
    }
}

template<class T>
template<int>
void ReplicatedPrep<T>::buffer_bits(false_type)
{
    ReplicatedRingPrep<T>::buffer_bits();
}

template<class T>
void ReplicatedPrep<T>::buffer_bits()
{
    assert(this->protocol != 0);
    buffer_bits<0>(T::clear::prime_field);
}

template<class T>
void BufferPrep<T>::get_one_no_count(Dtype dtype, T& a)
{
    if (dtype != DATA_BIT)
        throw not_implemented();

    while (bits.empty())
    {
        InScope in_scope(this->do_count, false);
        buffer_bits();
        n_bit_rounds++;
    }

    a = bits.back();
    bits.pop_back();
}

template<class T>
void BufferPrep<T>::get_input_no_count(T& a, typename T::open_type& x, int i)
{
    (void) a, (void) x, (void) i;
    if (inputs.size() <= (size_t)i)
        inputs.resize(i + 1);
    if (inputs.at(i).empty())
        buffer_inputs(i);
    a = inputs[i].back().share;
    x = inputs[i].back().value;
    inputs[i].pop_back();
}

template<class T>
void BufferPrep<T>::get_dabit_no_count(T& a, typename T::bit_type& b)
{
    if (dabits.empty())
    {
        InScope in_scope(this->do_count, false);
        ThreadQueues* queues = 0;
        buffer_dabits(queues);
    }
    a = dabits.back().first;
    b = dabits.back().second;
    dabits.pop_back();
}

template<class T>
void BufferPrep<T>::get_personal_dabit(int player, T& a, typename T::bit_type& b)
{
    auto& buffer = personal_dabits[player];
    if (buffer.empty())
    {
        InScope in_scope(this->do_count, false);
        buffer_personal_dabits(player);
    }
    a = buffer.back().first;
    b = buffer.back().second;
    buffer.pop_back();
}

template<class T>
void Preprocessing<T>::get_dabit(T& a, typename T::bit_type& b)
{
    get_dabit_no_count(a, b);
    this->count(DATA_DABIT);
}

template<class T>
template<int>
edabitvec<T> Preprocessing<T>::get_edabitvec(bool strict, int n_bits)
{
    auto& buffer = this->edabits[{strict, n_bits}];
    if (buffer.empty())
    {
        InScope in_scope(this->do_count, false);
        buffer_edabits_with_queues(strict, n_bits);
    }
    auto res = buffer.back();
    buffer.pop_back();
    return res;
}

template<class T>
template<int>
void Preprocessing<T>::get_edabit_no_count(bool strict, int n_bits, edabit<T>& a)
{
    auto& my_edabit = my_edabits[{strict, n_bits}];
    if (my_edabit.empty())
    {
        my_edabit = this->template get_edabitvec<0>(strict, n_bits);
    }
    a = my_edabit.next();
}

template<class T>
void BufferPrep<T>::buffer_edabits_with_queues(bool strict, int n_bits)
{
    ThreadQueues* queues = 0;
    if (BaseMachine::thread_num == 0)
        queues = &BaseMachine::s().queues;
    buffer_edabits(strict, n_bits, queues);
}

template<class T>
template<int>
void Preprocessing<T>::get_edabits(bool strict, size_t size, T* a,
        vector<typename T::bit_type>& Sb, const vector<int>& regs, false_type)
{
    int n_bits = regs.size();
    auto& buffer = edabits[{strict, n_bits}];
    edabit<T> eb;
    size_t unit = T::bit_type::default_length;
    for (int k = 0; k < DIV_CEIL(size, unit); k++)
    {
        if (not buffer.empty() and buffer.back().size() == unit and (k + 1) * unit <= size)
        {
            for (int j = 0; j < n_bits; j++)
                Sb[regs[j] + k] = buffer.back().get_b(j);
            for (size_t j = 0; j < unit; j++)
                a[k * unit + j] = buffer.back().get_a(j);
            buffer.pop_back();
        }
        else
        {
            for (size_t i = k * unit; i < min(size, (k + 1) * unit); i++)
            {
                this->template get_edabit_no_count<0>(strict, n_bits, eb);
                a[i] = eb.first;
                for (int j = 0; j < n_bits; j++)
                {
                    if (i % unit == 0)
                        Sb[regs[j] + i / unit] = {};
                    Sb[regs[j] + i / unit].xor_bit(i % unit, eb.second[j]);
                }
            }
        }
    }

    for (size_t i = 0; i < size; i++)
        this->usage.count_edabit(strict, n_bits);
}

template<class T>
void BufferPrep<T>::buffer_edabits(bool strict, int n_bits,
        ThreadQueues* queues)
{
    if (strict)
        buffer_sedabits(n_bits, queues);
    else
        buffer_edabits(n_bits, queues);
}

template<class T>
inline void BufferPrep<T>::buffer_inputs(int player)
{
    (void) player;
    throw not_implemented();
}

template<class T>
void BufferPrep<T>::buffer_inputs_as_usual(int player, SubProcessor<T>* proc)
{
    assert(proc != 0);
    auto& P = proc->P;
    this->inputs.resize(P.num_players());
    typename T::Input input(proc, P);
    input.reset(player);
    auto buffer_size = OnlineOptions::singleton.batch_size;
    if (P.my_num() == player)
    {
        SeededPRNG G;
        for (int i = 0; i < buffer_size; i++)
        {
            typename T::clear r;
            r.randomize(G);
            input.add_mine(r);
            this->inputs[player].push_back({input.finalize_mine(), r});
        }
        input.send_mine();
    }
    else
    {
        octetStream os;
        P.receive_player(player, os);
        T share;
        for (int i = 0; i < buffer_size; i++)
        {
            input.finalize_other(player, share, os);
            this->inputs[player].push_back({share, 0});
        }
    }
}

template<class T>
void BufferPrep<T>::get_no_count(vector<T>& S, DataTag tag,
        const vector<int>& regs, int vector_size)
{
    (void) S, (void) tag, (void) regs, (void) vector_size;
    throw not_implemented();
}

template<class T>
void BufferPrep<T>::shrink_to_fit()
{
    triples.shrink_to_fit();
}

template<class T>
T BufferPrep<T>::get_random()
{
    try
    {
        if (proc != 0)
            return proc->protocol.get_random();
        else
            throw not_implemented();
    }
    catch (not_implemented&)
    {
        return Preprocessing<T>::get_random();
    }
}

#endif
