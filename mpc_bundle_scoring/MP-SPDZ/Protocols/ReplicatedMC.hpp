/*
 * ReplicatedMC.cpp
 *
 */

#ifndef PROTOCOLS_REPLICATEDMC_HPP_
#define PROTOCOLS_REPLICATEDMC_HPP_

#include "ReplicatedMC.h"

template<class T>
void ReplicatedMC<T>::POpen(vector<typename T::open_type>& values,
        const vector<T>& S, const Player& P)
{
    prepare(S);
    P.pass_around(to_send, o, -1);
    finalize(values, S);
}

template<class T>
void ReplicatedMC<T>::POpen_Begin(vector<typename T::open_type>&,
        const vector<T>& S, const Player& P)
{
    prepare(S);
    P.send_relative(-1, to_send);
}

template<class T>
void ReplicatedMC<T>::prepare(const vector<T>& S)
{
    assert(T::length == 2);
    o.reset_write_head();
    to_send.reset_write_head();
    to_send.reserve(S.size() * T::value_type::size());
    for (auto& x : S)
        x[0].pack(to_send);
}

template<class T>
void ReplicatedMC<T>::exchange(const Player& P)
{
    prepare(this->secrets);
    P.pass_around(to_send, o, -1);
}

template<class T>
void ReplicatedMC<T>::POpen_End(vector<typename T::open_type>& values,
        const vector<T>& S, const Player& P)
{
    P.receive_relative(1, o);
    finalize(values, S);
}

template<class T>
void ReplicatedMC<T>::finalize(vector<typename T::open_type>& values,
        const vector<T>& S)
{
    values.resize(S.size());
    for (size_t i = 0; i < S.size(); i++)
    {
        typename T::open_type tmp;
        tmp.unpack(o);
        values[i] = S[i].sum() + tmp;
    }
}

template<class T>
typename T::open_type ReplicatedMC<T>::finalize_open()
{
    auto a = this->secrets.next().sum();
    return a + o.get<typename T::open_type>();
}

#endif
