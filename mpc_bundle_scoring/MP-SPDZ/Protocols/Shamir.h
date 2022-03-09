/*
 * Shamir.h
 *
 */

#ifndef PROTOCOLS_SHAMIR_H_
#define PROTOCOLS_SHAMIR_H_

#include <vector>
using namespace std;

#include "Replicated.h"

template<class T> class SubProcessor;
template<class T> class ShamirMC;
template<class T> class ShamirShare;
template<class T> class ShamirInput;
template<class T> class IndirectShamirMC;

class Player;

/**
 * Shamir secret sharing-based protocol with resharing
 */
template<class T>
class Shamir : public ProtocolBase<T>
{
    typedef typename T::open_type::Scalar U;

    octetStreams os;
    vector<U> reconstruction;
    U rec_factor;
    ShamirInput<T>* resharing;
    ShamirInput<T>* random_input;

    SeededPRNG secure_prng;

    map<int, vector<vector<typename T::open_type>>> hypers;

    typename T::open_type dotprod_share;

    void buffer_random();

    int threshold;
    int n_mul_players;

public:
    static const bool uses_triples = false;

    Player& P;

    static U get_rec_factor(int i, int n);
    static U get_rec_factor(int i, int n_total, int start, int threshold);

    Shamir(Player& P, int threshold = 0);
    ~Shamir();

    Shamir branch();

    int get_n_relevant_players();

    void reset();

    void init_mul();
    void init_mul(SubProcessor<T>* proc);

    template<class V>
    void init_mul(V*)
    {
        init_mul();
    }
    template<class V, class W>
    void init_mul(const V&, const W&)
    {
        init_mul();
    }

    typename T::clear prepare_mul(const T& x, const T& y, int n = -1);

    void exchange();
    void start_exchange();
    void stop_exchange();

    T finalize_mul(int n = -1);

    T finalize(int n_input_players);

    void init_dotprod(SubProcessor<T>* proc = 0);
    void prepare_dotprod(const T& x, const T& y);
    void next_dotprod();
    T finalize_dotprod(int length);

    vector<T> get_randoms(PRNG& G, int t);

    vector<vector<typename T::open_type>>& get_hyper(int t);
    static void get_hyper(vector<vector<typename T::open_type>>& hyper, int t, int n);
    static string hyper_filename(int t, int n);
};

#endif /* PROTOCOLS_SHAMIR_H_ */
