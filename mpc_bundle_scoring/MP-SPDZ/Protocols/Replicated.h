/*
 * Replicated.h
 *
 */

#ifndef PROTOCOLS_REPLICATED_H_
#define PROTOCOLS_REPLICATED_H_

#include <assert.h>
#include <vector>
#include <array>
using namespace std;

#include "Tools/octetStream.h"
#include "Tools/random.h"
#include "Tools/PointerVector.h"
#include "Networking/Player.h"

template<class T> class SubProcessor;
template<class T> class ReplicatedMC;
template<class T> class ReplicatedInput;
template<class T> class ReplicatedPrivateOutput;
template<class T> class Share;
template<class T> class Rep3Share;
template<class T> class MAC_Check_Base;
template<class T> class Preprocessing;
class Instruction;

/**
 * Base class for replicated three-party protocols
 */
class ReplicatedBase
{
public:
    array<PRNG, 2> shared_prngs;

    Player& P;

    ReplicatedBase(Player& P);
    ReplicatedBase(Player& P, array<PRNG, 2>& prngs);

    ReplicatedBase branch();

    int get_n_relevant_players() { return P.num_players() - 1; }
};

/**
 * Abstract base class for multiplication protocols
 */
template <class T>
class ProtocolBase
{
    virtual void buffer_random() { throw not_implemented(); }

protected:
    vector<T> random;

    int trunc_pr_counter;
    int rounds, trunc_rounds;

public:
    typedef T share_type;

    int counter;

    ProtocolBase();
    virtual ~ProtocolBase();

    void muls(const vector<int>& reg, SubProcessor<T>& proc, typename T::MAC_Check& MC,
            int size);
    void mulrs(const vector<int>& reg, SubProcessor<T>& proc);

    void multiply(vector<T>& products, vector<pair<T, T>>& multiplicands,
            int begin, int end, SubProcessor<T>& proc);

    /// Single multiplication
    T mul(const T& x, const T& y);

    /// Initialize multiplication round
    virtual void init_mul(SubProcessor<T>* proc) = 0;
    /// Schedule multiplication of operand pair
    virtual typename T::clear prepare_mul(const T& x, const T& y, int n = -1) = 0;
    /// Run multiplication protocol
    virtual void exchange() = 0;
    /// Get next multiplication result
    virtual T finalize_mul(int n = -1) = 0;
    /// Store next multiplication result in ``res``
    virtual void finalize_mult(T& res, int n = -1);

    /// Initialize dot product round
    void init_dotprod(SubProcessor<T>* proc) { init_mul(proc); }
    /// Add operand pair to current dot product
    void prepare_dotprod(const T& x, const T& y) { prepare_mul(x, y); }
    /// Finish dot product
    void next_dotprod() {}
    /// Get next dot product result
    T finalize_dotprod(int length);

    virtual T get_random();

    virtual void trunc_pr(const vector<int>& regs, int size, SubProcessor<T>& proc)
    { (void) regs, (void) size; (void) proc; throw runtime_error("trunc_pr not implemented"); }

    virtual void randoms(T&, int) { throw runtime_error("randoms not implemented"); }
    virtual void randoms_inst(vector<T>&, const Instruction&);

    template<int = 0>
    void matmulsm(SubProcessor<T> & proc, CheckVector<T>& source,
            const Instruction& instruction, int a, int b)
    { proc.matmulsm(source, instruction, a, b); }

    template<int = 0>
    void conv2ds(SubProcessor<T>& proc, const Instruction& instruction)
    { proc.conv2ds(instruction); }

    virtual void start_exchange() { exchange(); }
    virtual void stop_exchange() {}

    virtual void check() {}

    virtual void cisc(SubProcessor<T>&, const Instruction&)
    { throw runtime_error("CISC instructions not implemented"); }
};

/**
 * Semi-honest replicated three-party protocol
 */
template <class T>
class Replicated : public ReplicatedBase, public ProtocolBase<T>
{
    array<octetStream, 2> os;
    PointerVector<typename T::clear> add_shares;
    typename T::clear dotprod_share;

public:
    typedef ReplicatedMC<T> MAC_Check;
    typedef ReplicatedInput<T> Input;

    static const bool uses_triples = false;

    Replicated(Player& P);
    Replicated(const ReplicatedBase& other);

    static void assign(T& share, const typename T::clear& value, int my_num)
    {
        assert(T::length == 2);
        share.assign_zero();
        if (my_num < 2)
            share[my_num] = value;
    }

    void init_mul(SubProcessor<T>* proc);
    void init_mul(Preprocessing<T>& prep, typename T::MAC_Check& MC);

    void init_mul();
    typename T::clear prepare_mul(const T& x, const T& y, int n = -1);
    void exchange();
    T finalize_mul(int n = -1);

    void prepare_reshare(const typename T::clear& share, int n = -1);

    void init_dotprod(SubProcessor<T>*) { init_mul(); }
    void init_dotprod();
    void prepare_dotprod(const T& x, const T& y);
    void next_dotprod();
    T finalize_dotprod(int length);

    template<class U>
    void trunc_pr(const vector<int>& regs, int size, U& proc);

    T get_random();
    void randoms(T& res, int n_bits);

    void start_exchange();
    void stop_exchange();
};

#endif /* PROTOCOLS_REPLICATED_H_ */
