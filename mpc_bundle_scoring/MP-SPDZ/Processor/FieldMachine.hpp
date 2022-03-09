/*
 * ReplicatedFieldMachine.hpp
 *
 */

#ifndef PROCESSOR_FIELDMACHINE_HPP_
#define PROCESSOR_FIELDMACHINE_HPP_

#include "FieldMachine.h"
#include "HonestMajorityMachine.h"
#include "Math/gfp.h"
#include "OnlineMachine.hpp"


template<template<class U> class T, class V>
HonestMajorityFieldMachine<T, V>::HonestMajorityFieldMachine(int argc,
        const char **argv)
{
    ez::ezOptionParser opt;
    HonestMajorityFieldMachine<T>(argc, argv, opt);
}

template<template<class U> class T, class V>
HonestMajorityFieldMachine<T, V>::HonestMajorityFieldMachine(int argc,
        const char **argv, ez::ezOptionParser& opt, int nplayers)
{
    OnlineOptions online_opts(opt, argc, argv, 0, true, true);
    FieldMachine<T, T, V>(argc, argv, opt, online_opts,
            nplayers);
}

template<template<class U> class T, template<class U> class V, class W, class X>
FieldMachine<T, V, W, X>::FieldMachine(int argc, const char** argv,
        ez::ezOptionParser& opt, OnlineOptions& online_opts, int nplayers)
{
    W machine(argc, argv, opt, online_opts, X(), nplayers);
    int n_limbs = online_opts.prime_limbs();
    switch (n_limbs)
    {
#undef X
#define X(L) \
    case L: \
        machine.template run<T<gfp_<0, L>>, V<X>>(); \
        break;
#ifndef FEWER_PRIMES
    X(1) X(2) X(3) X(4)
#endif
#if GFP_MOD_SZ > 4 or defined(FEWER_PRIMES)
    X(GFP_MOD_SZ)
#endif
#undef X
    default:
        cerr << "Not compiled for " << online_opts.prime_length() << "-bit primes" << endl;
        cerr << "Compile with -DGFP_MOD_SZ=" << n_limbs << endl;
        exit(1);
    }
}

#endif /* PROCESSOR_FIELDMACHINE_HPP_ */
