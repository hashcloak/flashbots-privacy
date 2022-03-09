/*
 * MAC_Check_Base.hpp
 *
 */

#ifndef PROTOCOLS_MAC_CHECK_BASE_HPP_
#define PROTOCOLS_MAC_CHECK_BASE_HPP_

#include "MAC_Check_Base.h"

template<class T>
void MAC_Check_Base<T>::POpen_Begin(vector<typename T::open_type>&,
        const vector<T>& S, const Player& P)
{
    init_open(P, S.size());
    for (auto& secret : S)
        prepare_open(secret);
    exchange(P);
}

template<class T>
void MAC_Check_Base<T>::POpen_End(vector<typename T::open_type>& values,
        const vector<T>& S, const Player&)
{
    values.clear();
    values.reserve(S.size());
    for (size_t i = 0; i < S.size(); i++)
        values.push_back(finalize_open());
}

template<class T>
void MAC_Check_Base<T>::POpen(vector<typename T::open_type>& values,const vector<T>& S,const Player& P)
{
    MAC_Check_Base<T>::POpen_Begin(values, S, P);
    MAC_Check_Base<T>::POpen_End(values, S, P);
}

template<class T>
typename T::open_type MAC_Check_Base<T>::POpen(const T& secret, const Player& P)
{
    vector<typename T::open_type> opened;
    POpen(opened, {secret}, P);
    return opened[0];
}

template<class T>
void MAC_Check_Base<T>::init_open(const Player&, int n)
{
    secrets.clear();
    secrets.reserve(n);
    values.clear();
    values.reserve(n);
}

template<class T>
void MAC_Check_Base<T>::prepare_open(const T& secret)
{
    secrets.push_back(secret);
}

template<class T>
typename T::open_type MAC_Check_Base<T>::finalize_open()
{
    return values.next();
}

template<class T>
void MAC_Check_Base<T>::CheckFor(const typename T::open_type& value,
        const vector<T>& shares, const Player& P)
{
    vector<typename T::open_type> opened;
    POpen(opened, shares, P);
    for (auto& check : opened)
        if (value != typename T::clear(check))
        {
            cout << check << " != " << value << endl;
            throw Offline_Check_Error("CheckFor");
        }
    Check(P);
}

#endif /* PROTOCOLS_MAC_CHECK_BASE_HPP_ */
