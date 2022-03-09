/*
 * Shamir.cpp
 *
 */

#ifndef PROTOCOLS_SHAMIR_HPP_
#define PROTOCOLS_SHAMIR_HPP_

#include "Shamir.h"
#include "ShamirInput.h"
#include "ShamirShare.h"
#include "Machines/ShamirMachine.h"
#include "Tools/benchmarking.h"

template<class T>
typename T::open_type::Scalar Shamir<T>::get_rec_factor(int i, int n)
{
    return get_rec_factor(i, n, 0, n);
}

template<class T>
typename T::open_type::Scalar Shamir<T>::get_rec_factor(int i, int n_total,
        int start, int n_points)
{
    U res = 1;
    for (int j = 0; j < n_points; j++)
    {
        int other = positive_modulo(start + j, n_total);
        if (i != other)
            res *= U(other + 1) / (U(other + 1) - U(i + 1));
    }
    return res;
}

template<class T>
Shamir<T>::Shamir(Player& P, int t) :
        resharing(0), random_input(0), P(P)
{
    if (not P.is_encrypted())
        insecure("unencrypted communication");
    if (t > 0)
        threshold = t;
    else
        threshold = ShamirMachine::s().threshold;
    n_mul_players = 2 * threshold + 1;
}

template<class T>
Shamir<T>::~Shamir()
{
    if (resharing != 0)
        delete resharing;
    if (random_input != 0)
        delete random_input;
}

template<class T>
Shamir<T> Shamir<T>::branch()
{
    return P;
}

template<class T>
int Shamir<T>::get_n_relevant_players()
{
    return threshold + 1;
}

template<class T>
void Shamir<T>::reset()
{
    os.reset(P);

    if (resharing == 0)
    {
        resharing = new ShamirInput<T>(0, P);
    }

    for (int i = 0; i < P.num_players(); i++)
        resharing->reset(i);
}

template<class T>
void Shamir<T>::init_mul(SubProcessor<T>* proc)
{
    (void) proc;
    init_mul();
}

template<class T>
void Shamir<T>::init_mul()
{
    reset();
    if (rec_factor == 0 and P.my_num() < n_mul_players)
        rec_factor = get_rec_factor(P.my_num(), n_mul_players);
}

template<class T>
typename T::clear Shamir<T>::prepare_mul(const T& x, const T& y, int n)
{
    (void) n;
    auto add_share = x * y * rec_factor;
    if (P.my_num() < n_mul_players)
        resharing->add_mine(add_share);
    return {};
}

template<class T>
void Shamir<T>::exchange()
{
    vector<bool> senders(P.num_players(), false);
    for (int i = 0; i < n_mul_players; i++)
        senders[i] = true;
    P.send_receive_all(senders, resharing->os, os);
}

template<class T>
void Shamir<T>::start_exchange()
{
    if (P.my_num() < n_mul_players)
        for (int offset = 1; offset < P.num_players(); offset++)
            P.send_relative(offset, resharing->os[P.get_player(offset)]);
}

template<class T>
void Shamir<T>::stop_exchange()
{
    for (int offset = 1; offset < P.num_players(); offset++)
    {
        int receive_from = P.get_player(-offset);
        if (receive_from < n_mul_players)
            P.receive_player(receive_from, os[receive_from]);
    }
}

template<class T>
T Shamir<T>::finalize_mul(int n)
{
    (void) n;
    return finalize(n_mul_players);
}

template<class T>
T Shamir<T>::finalize(int n_relevant_players)
{
    ShamirShare<U> res = U(0);
    if (P.my_num() < n_relevant_players)
        res = resharing->finalize_mine();
    for (int i = 0; i < n_relevant_players; i++)
        if (i != P.my_num())
        {
            T tmp;
            resharing->finalize_other(i, tmp, os[i]);
            res += tmp;
        }
    return res;
}

template<class T>
void Shamir<T>::init_dotprod(SubProcessor<T>* proc)
{
    init_mul(proc);
    dotprod_share = 0;
}

template<class T>
void Shamir<T>::prepare_dotprod(const T& x, const T& y)
{
    dotprod_share += x * y * rec_factor;
}

template<class T>
void Shamir<T>::next_dotprod()
{
    if (P.my_num() < n_mul_players)
        resharing->add_mine(dotprod_share);
    dotprod_share = 0;
}

template<class T>
T Shamir<T>::finalize_dotprod(int)
{
    return finalize_mul();
}

template<class T>
void Shamir<T>::buffer_random()
{
    this->random = get_randoms(secure_prng, threshold);
}

template<class T>
vector<vector<typename T::open_type>>& Shamir<T>::get_hyper(int t)
{
    auto& hyper = hypers[t];
    if (int(hyper.size()) != P.num_players() - t)
    {
        get_hyper(hyper, t, P.num_players());
    }
    return hyper;
}

template<class T>
string Shamir<T>::hyper_filename(int t, int n)
{
    return PREP_DIR "/Hyper-" + to_string(t) + "-" + to_string(n) + "-"
            + to_string(T::clear::pr());
}

template<class T>
void Shamir<T>::get_hyper(vector<vector<typename T::open_type> >& hyper,
        int t, int n)
{
    assert(hyper.empty());

    try
    {
        octetStream os;
        string filename = hyper_filename(t, n);
        ifstream in(filename);
#ifdef VERBOSE_HYPER
        cerr << "Trying to load hyper-invertable matrix from " << filename << endl;
#endif
        os.input(in);
        os.get(hyper);
        if (int(hyper.size()) != n - t)
            throw exception();
#ifdef VERBOSE_HYPER
        cerr << "Loaded hyper-invertable matrix from " << filename << endl;
#endif
        return;
    }
    catch (...)
    {
#ifdef VERBOSE_HYPER
        cerr << "Failed to load hyper-invertable" << endl;
#endif
    }

    map<int, U> inverses, dividends;
    for (int i = -n; i < n; i++)
        if (i != 0)
            inverses[i] = U(i).invert();
    for (int i = 0; i < 2 * n; i++)
        dividends[i] = i;
    for (int i = 0; i < n - t; i++)
    {
        hyper.push_back({});
        for (int j = 0; j < n; j++)
        {
            hyper.back().push_back({1});
            for (int k = 0; k < n; k++)
                if (k != j)
                    hyper.back().back() *= dividends.at(n + i - k)
                    * inverses.at(j - k);
        }
    }
}

template<class T>
vector<T> Shamir<T>::get_randoms(PRNG& G, int t)
{
    auto& hyper = get_hyper(t);
    if (random_input == 0)
        random_input = new ShamirInput<T>(0, P, threshold);
    auto& input = *random_input;
    input.reset_all(P);
    int buffer_size = OnlineOptions::singleton.batch_size;
    for (int i = 0; i < buffer_size; i += hyper.size())
        input.add_mine(G.get<U>());
    input.exchange();
    vector<U> inputs;
    vector<T> random;
    random.reserve(buffer_size + hyper.size());
    for (int i = 0; i < buffer_size; i += hyper.size())
    {
        inputs.clear();
        for (int j = 0; j < P.num_players(); j++)
            inputs.push_back(input.finalize(j));
        for (size_t j = 0; j < hyper.size(); j++)
        {
            random.push_back({});
            for (int k = 0; k < P.num_players(); k++)
                random.back() += hyper[j][k] * inputs[k];
        }
    }
    return random;
}

#endif
