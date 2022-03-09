/*
 * Beaver.cpp
 *
 */

#ifndef PROTOCOLS_BEAVER_HPP_
#define PROTOCOLS_BEAVER_HPP_

#include "Beaver.h"

#include "Replicated.hpp"

#include <array>

template<class T>
Player& Beaver<T>::branch()
{
    return P;
}

template<class T>
void Beaver<T>::init_mul(SubProcessor<T>* proc)
{
    assert(proc != 0);
    init_mul(proc->DataF, proc->MC);
}

template<class T>
void Beaver<T>::init_mul(Preprocessing<T>& prep, typename T::MAC_Check& MC)
{
    this->prep = &prep;
    this->MC = &MC;
    shares.clear();
    opened.clear();
    triples.clear();
}

template<class T>
typename T::clear Beaver<T>::prepare_mul(const T& x, const T& y, int n)
{
    (void) n;
    triples.push_back({{}});
    auto& triple = triples.back();
    triple = prep->get_triple(n);
    shares.push_back(x - triple[0]);
    shares.push_back(y - triple[1]);
    return 0;
}

template<class T>
void Beaver<T>::exchange()
{
    MC->POpen(opened, shares, P);
    it = opened.begin();
    triple = triples.begin();
}

template<class T>
void Beaver<T>::start_exchange()
{
    MC->POpen_Begin(opened, shares, P);
}

template<class T>
void Beaver<T>::stop_exchange()
{
    MC->POpen_End(opened, shares, P);
    it = opened.begin();
    triple = triples.begin();
}

template<class T>
T Beaver<T>::finalize_mul(int n)
{
    (void) n;
    typename T::open_type masked[2];
    T& tmp = (*triple)[2];
    for (int k = 0; k < 2; k++)
    {
        masked[k] = *it++;
    }
    tmp += (masked[0] * (*triple)[1]);
    tmp += ((*triple)[0] * masked[1]);
    tmp += T::constant(masked[0] * masked[1], P.my_num(), MC->get_alphai());
    triple++;
    return tmp;
}

#endif
