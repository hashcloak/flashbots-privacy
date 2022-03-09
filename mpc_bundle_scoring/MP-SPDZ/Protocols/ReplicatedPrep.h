/*
 * ReplicatedPrep.h
 *
 */

#ifndef PROTOCOLS_REPLICATEDPREP_H_
#define PROTOCOLS_REPLICATEDPREP_H_

#include "Networking/Player.h"
#include "Processor/Data_Files.h"
#include "Processor/OnlineOptions.h"
#include "Processor/ThreadQueues.h"
#include "Protocols/ShuffleSacrifice.h"
#include "Protocols/MAC_Check_Base.h"
#include "Protocols/ShuffleSacrifice.h"
#include "edabit.h"

#include <array>

template<class T>
void bits_from_random(vector<T>& bits, typename T::Protocol& protocol);

namespace GC
{
template<class T> class ShareThread;
}

/**
 * Abstract base class for live preprocessing
 */
template<class T>
class BufferPrep : public Preprocessing<T>
{
    template<class U, class V> friend class Machine;

    template<int>
    void buffer_inverses(true_type);
    template<int>
    void buffer_inverses(false_type) { throw runtime_error("no inverses"); }

protected:
    vector<array<T, 3>> triples;
    vector<array<T, 2>> squares;
    vector<array<T, 2>> inverses;
    vector<T> bits;
    vector<vector<InputTuple<T>>> inputs;

    vector<dabit<T>> dabits;

    int n_bit_rounds;

    SubProcessor<T>* proc;

    virtual void buffer_triples() { throw runtime_error("no triples"); }
    virtual void buffer_squares() { throw runtime_error("no squares"); }
    virtual void buffer_inverses();
    virtual void buffer_bits() { throw runtime_error("no bits"); }
    virtual void buffer_inputs(int player);

    // don't call this if T::Input requires input tuples
    void buffer_inputs_as_usual(int player, SubProcessor<T>* proc);

    virtual void buffer_dabits(ThreadQueues* = 0) { throw runtime_error("no daBits"); }
    virtual void buffer_edabits(int, ThreadQueues*) { throw runtime_error("no edaBits"); }
    virtual void buffer_sedabits(int, ThreadQueues*) { throw runtime_error("no sedaBits"); }

    virtual void buffer_edabits(bool strict, int n_bits,
            ThreadQueues* queues = 0);
    virtual void buffer_edabits_with_queues(bool strict, int n_bits);

    map<int, vector<dabit<T>>> personal_dabits;
    void get_personal_dabit(int player, T& a, typename T::bit_type& b);
    virtual void buffer_personal_dabits(int)
    { throw runtime_error("no personal daBits"); }

    void push_edabits(vector<edabitvec<T>>& edabits,
            const vector<T>& sums, const vector<vector<typename T::bit_type::part_type>>& bits,
            int buffer_size);
public:
    typedef T share_type;

    int buffer_size;

    /// Key-independent setup if necessary (cryptosystem parameters)
    static void basic_setup(Player& P) { (void) P; }
    /// Generate keys if necessary
    static void setup(Player& P, typename T::mac_key_type alphai) { (void) P, (void) alphai; }
    /// Free memory of global cryptosystem parameters
    static void teardown() {}

    static void edabit_sacrifice_buckets(vector<edabit<T>>&, size_t, bool, int,
            SubProcessor<T>&, int, int, const void* = 0)
    {
        throw runtime_error("sacrifice not available");
    }

    BufferPrep(DataPositions& usage);
    virtual ~BufferPrep();

    void clear();

    void get_three_no_count(Dtype dtype, T& a, T& b, T& c);
    void get_two_no_count(Dtype dtype, T& a, T& b);
    void get_one_no_count(Dtype dtype, T& a);
    void get_input_no_count(T& a, typename T::open_type& x, int i);
    void get_no_count(vector<T>& S, DataTag tag, const vector<int>& regs,
            int vector_size);

    virtual void get_dabit_no_count(T& a, typename T::bit_type& b);

    /// Get fresh random value
    virtual T get_random();

    void push_triples(const vector<array<T, 3>>& triples)
    { this->triples.insert(this->triples.end(), triples.begin(), triples.end()); }
    void push_triple(const array<T, 3>& triple)
    { this->triples.push_back(triple); }

    void shrink_to_fit();

    void buffer_personal_triples(int, ThreadQueues*) {}
    void buffer_personal_triples(vector<array<T, 3>>&, int, int) {}

    SubProcessor<T>* get_proc() { return proc; }
    void set_proc(SubProcessor<T>* proc) { this->proc = proc; }
};

/**
 * Generic preprocessing protocols
 */
template<class T>
class BitPrep : public virtual BufferPrep<T>
{
protected:
    int base_player;

    typename T::Protocol* protocol;

    void buffer_ring_bits_without_check(vector<T>& bits, PRNG& G,
            int buffer_size);

public:
    BitPrep(SubProcessor<T>* proc, DataPositions& usage);
    ~BitPrep();

    void set_protocol(typename T::Protocol& protocol);

    /// Generate squares from triples
    void buffer_squares();

    /// Generate random bits from inputs without semi-honest security
    void buffer_bits_without_check();
};

/**
 * Generate (e)daBit protocols
 */
template<class T>
class RingPrep : public virtual BitPrep<T>
{
    typedef typename T::bit_type::part_type BT;

    SubProcessor<BT>* bit_part_proc;

protected:
    void buffer_dabits_without_check(vector<dabit<T>>& dabits,
            int buffer_size = -1, ThreadQueues* queues = 0);
    template<int>
    void buffer_edabits_without_check(int n_bits, vector<T>& sums,
            vector<vector<typename T::bit_type::part_type>>& bits, int buffer_size,
            ThreadQueues* queues = 0);
    template<int>
    void buffer_edabits_without_check(int n_bits, vector<edabitvec<T>>& edabits,
            int buffer_size);

    void buffer_sedabits_from_edabits(int n_bits)
    { this->template buffer_sedabits_from_edabits<0>(n_bits, T::clear::characteristic_two); }
    template<int>
    void buffer_sedabits_from_edabits(int n_bits, false_type);
    template<int>
    void buffer_sedabits_from_edabits(int, true_type)
    { throw not_implemented(); }

    template<int>
    void sanitize(vector<edabitvec<T>>& edabits, int n_bits);

public:
    RingPrep(SubProcessor<T>* proc, DataPositions& usage);
    virtual ~RingPrep();

    vector<T>& get_bits() { return this->bits; }

    /// Generate strict edabits from loose ones
    template<int>
    void sanitize(vector<edabit<T>>& edabits, int n_bits,
            int player = -1, ThreadQueues* queues = 0);
    template<int>
    void sanitize(vector<edabit<T>>& edabits, int n_bits, int player, int begin,
            int end);

    /// Generic daBit generation with semi-honest security
    void buffer_dabits_without_check(vector<dabit<T>>& dabits,
            size_t begin, size_t end);
    template<int>
    void buffer_dabits_without_check(vector<dabit<T>>& dabits,
            size_t begin, size_t end,
            Preprocessing<typename T::bit_type::part_type>& bit_prep);

    /// Generic edaBit generation with semi-honest security
    template<int>
    void buffer_edabits_without_check(int n_bits, vector<T>& sums,
            vector<vector<typename T::bit_type::part_type>>& bits, int begin,
            int end);

    template<int>
    void buffer_personal_edabits_without_check(int n_bits, vector<T>& sums,
            vector<vector<BT> >& bits, SubProcessor<BT>& proc, int input_player,
            int begin, int end);
};

/**
 * Semi-honest *bit preprocessing
 */
template<class T>
class SemiHonestRingPrep : public virtual RingPrep<T>
{
public:
    SemiHonestRingPrep(SubProcessor<T>* proc, DataPositions& usage) :
            BufferPrep<T>(usage), BitPrep<T>(proc, usage),
			RingPrep<T>(proc, usage)
    {
    }
    virtual ~SemiHonestRingPrep() {}

    virtual void buffer_bits() { this->buffer_bits_without_check(); }
    virtual void buffer_inputs(int player)
    { this->buffer_inputs_as_usual(player, this->proc); }

    virtual void buffer_dabits(ThreadQueues*)
    { this->buffer_dabits_without_check(this->dabits); }
    virtual void buffer_edabits(int n_bits, ThreadQueues*)
    { buffer_edabits<0>(n_bits, T::clear::characteristic_two); }
    template<int>
    void buffer_edabits(int n_bits, false_type)
    { this->template buffer_edabits_without_check<0>(n_bits,
            this->edabits[{false, n_bits}],
            OnlineOptions::singleton.batch_size); }
    template<int>
    void buffer_edabits(int, true_type)
    { throw not_implemented(); }
    virtual void buffer_sedabits(int n_bits, ThreadQueues*)
    { this->buffer_sedabits_from_edabits(n_bits); }
};

/**
 * daBit preprocessing with malicious security
 */
template<class T>
class MaliciousDabitOnlyPrep : public virtual RingPrep<T>
{
    template<int>
    void buffer_dabits(ThreadQueues* queues, true_type, false_type);
    template<int>
    void buffer_dabits(ThreadQueues* queues, false_type, false_type);
    template<int>
    void buffer_dabits(ThreadQueues* queues, false_type, true_type);

public:
    MaliciousDabitOnlyPrep(SubProcessor<T>* proc, DataPositions& usage) :
            BufferPrep<T>(usage), BitPrep<T>(proc, usage),
            RingPrep<T>(proc, usage)
    {
    }
    virtual ~MaliciousDabitOnlyPrep() {}

    virtual void buffer_dabits(ThreadQueues* queues);
};

/**
 * Random bit and edaBit preprocessing with malicious security
 */
template<class T>
class MaliciousRingPrep : public virtual MaliciousDabitOnlyPrep<T>
{
    typedef typename T::bit_type::part_type BT;

protected:
    void buffer_personal_edabits(int n_bits, vector<T>& sums,
            vector<vector<BT>>& bits, SubProcessor<BT>& proc, int input_player,
            bool strict, ThreadQueues* queues = 0);

    void buffer_edabits_from_personal(bool strict, int n_bits,
            ThreadQueues* queues);
    template<int>
    void buffer_edabits_from_personal(bool strict, int n_bits,
            ThreadQueues* queues, true_type);
    template<int>
    void buffer_edabits_from_personal(bool strict, int n_bits,
            ThreadQueues* queues, false_type);

    void buffer_personal_dabits(int input_player);
    template<int>
    void buffer_personal_dabits(int input_player, true_type, false_type);
    template<int>
    void buffer_personal_dabits(int input_player, false_type, false_type);
    template<int>
    void buffer_personal_dabits(int input_player, false_type, true_type);

    template<int>
    void buffer_personal_dabits_without_check(int input_player,
            vector<dabit<T>>& dabits, int buffer_size);

public:
    static void edabit_sacrifice_buckets(vector<edabit<T>>& to_check, size_t n_bits,
            bool strict, int player, SubProcessor<T>& proc, int begin, int end,
            const void* supply = 0)
    {
        EdabitShuffleSacrifice<T>().edabit_sacrifice_buckets(to_check, n_bits, strict,
                player, proc, begin, end, supply);
    }

    MaliciousRingPrep(SubProcessor<T>* proc, DataPositions& usage) :
            BufferPrep<T>(usage), BitPrep<T>(proc, usage),
            RingPrep<T>(proc, usage), MaliciousDabitOnlyPrep<T>(proc, usage)
    {
    }
    virtual ~MaliciousRingPrep() {}

    virtual void buffer_bits();
    virtual void buffer_edabits(bool strict, int n_bits, ThreadQueues* queues);
};

/**
 * Semi-honest preprocessing with honest majority (no (e)daBits)
 */
template<class T>
class ReplicatedRingPrep : public virtual BitPrep<T>
{
protected:
    void buffer_triples();
    void buffer_squares();

public:
    ReplicatedRingPrep(SubProcessor<T>* proc, DataPositions& usage) :
            BufferPrep<T>(usage), BitPrep<T>(proc, usage)
    {
    }

    virtual ~ReplicatedRingPrep() {}

    virtual void buffer_bits() { this->buffer_bits_without_check(); }
};

/**
 * Semi-honest preprocessing with honest majority (including (e)daBits)
 */
template<class T>
class ReplicatedPrep : public virtual ReplicatedRingPrep<T>,
        public virtual SemiHonestRingPrep<T>
{
    template<int>
    void buffer_bits(false_type);
    template<int>
    void buffer_bits(true_type);

public:
    ReplicatedPrep(SubProcessor<T>* proc, DataPositions& usage) :
            BufferPrep<T>(usage), BitPrep<T>(proc, usage),
            ReplicatedRingPrep<T>(proc, usage),
            RingPrep<T>(proc, usage),
            SemiHonestRingPrep<T>(proc, usage)
    {
    }

    ReplicatedPrep(DataPositions& usage, int = 0) :
            ReplicatedPrep(0, usage)
    {
    }

    template<class U>
    ReplicatedPrep(DataPositions& usage, GC::ShareThread<U>&, int = 0) :
            ReplicatedPrep(0, usage)
    {
    }

    void buffer_squares() { ReplicatedRingPrep<T>::buffer_squares(); }
    void buffer_bits();
};

#endif /* PROTOCOLS_REPLICATEDPREP_H_ */
