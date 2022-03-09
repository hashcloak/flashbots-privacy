/*
 * SemiMC.cpp
 *
 */

#ifndef PROTOCOLS_SEMIMC_HPP_
#define PROTOCOLS_SEMIMC_HPP_

#include "SemiMC.h"

#include "MAC_Check.hpp"

template<class T>
void SemiMC<T>::prepare_open(const T& secret)
{
    this->values.push_back(secret);
}

template<class T>
void SemiMC<T>::exchange(const Player& P)
{
    this->run(this->values, P);
}

template<class T>
void DirectSemiMC<T>::POpen_(vector<typename T::open_type>& values,
        const vector<T>& S, const PlayerBase& P)
{
    this->values.clear();
    this->values.reserve(S.size());
    for (auto& secret : S)
        this->prepare_open(secret);
    this->exchange_(P);
    values = this->values;
}

template<class T>
void DirectSemiMC<T>::exchange_(const PlayerBase& P)
{
    Bundle<octetStream> oss(P);
    oss.mine.reserve(this->values.size());
    for (auto& x : this->values)
        x.pack(oss.mine);
    P.unchecked_broadcast(oss);
    direct_add_openings<typename T::open_type>(this->values, P, oss);
}

template<class T>
void DirectSemiMC<T>::POpen_Begin(vector<typename T::open_type>& values,
        const vector<T>& S, const Player& P)
{
    values.clear();
    values.insert(values.begin(), S.begin(), S.end());
    octetStream os;
    for (auto& x : values)
        x.pack(os);
    P.send_all(os);
}

template<class T>
void DirectSemiMC<T>::POpen_End(vector<typename T::open_type>& values,
        const vector<T>&, const Player& P)
{
    Bundle<octetStream> oss(P);
    P.receive_all(oss);
    direct_add_openings<typename T::open_type>(values, P, oss);
}

#endif
