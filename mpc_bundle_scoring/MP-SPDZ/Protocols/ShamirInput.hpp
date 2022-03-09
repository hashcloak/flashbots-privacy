/*
 * ShamirInput.cpp
 *
 */

#ifndef PROTOCOLS_SHAMIRINPUT_HPP_
#define PROTOCOLS_SHAMIRINPUT_HPP_

#include "ShamirInput.h"
#include "Machines/ShamirMachine.h"

#include "Protocols/ReplicatedInput.hpp"

template<class U>
void IndividualInput<U>::reset(int player)
{
    if (player == P.my_num())
    {
        this->shares.clear();
        this->i_share = 0;
        os.reset(P);
    }
}

template<class T>
vector<vector<typename T::open_type>> ShamirInput<T>::get_vandermonde(
        size_t t, size_t n)
{
    vector<vector<typename T::open_type>> vandermonde(n);

    for (int i = 0; i < int(n); i++)
        if (vandermonde[i].size() < t)
        {
            vandermonde[i].resize(t);
            typename T::open_type x = 1;
            for (size_t j = 0; j < t; j++)
            {
                x *= (i + 1);
                vandermonde[i][j] = x;
            }
        }

    return vandermonde;
}

template<class T>
void ShamirInput<T>::add_mine(const typename T::open_type& input, int n_bits)
{
    (void) n_bits;
    auto& P = this->P;
    int n = P.num_players();
    int t = threshold;

    if (vandermonde.empty())
        vandermonde = get_vandermonde(t, n);

    randomness.resize(t);
    for (auto& x : randomness)
        x.randomize(secure_prng);

    for (int i = 0; i < n; i++)
    {
        typename T::open_type x = input;
        for (int j = 0; j < t; j++)
            x += randomness[j] * vandermonde[i][j];
        if (i == P.my_num())
            this->shares.push_back(x);
        else
            x.pack(this->os[i]);
    }
}

template<class U>
void IndividualInput<U>::add_other(int player, int)
{
    (void) player;
}

template<class U>
void IndividualInput<U>::send_mine()
{
    for (int i = 0; i < P.num_players(); i++)
        if (i != P.my_num())
            P.send_to(i, os[i]);
}

template<class T>
void IndividualInput<T>::exchange()
{
    P.send_receive_all(os, InputBase<T>::os);
}

template<class T>
void IndividualInput<T>::finalize_other(int player, T& target, octetStream& o,
        int n_bits)
{
    (void) player;
    target.unpack(o, n_bits);
}

#endif
