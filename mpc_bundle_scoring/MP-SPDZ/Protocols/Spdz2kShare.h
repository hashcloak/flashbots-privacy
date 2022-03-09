/*
 * Spdz2kShare.h
 *
 */

#ifndef PROTOCOLS_SPDZ2KSHARE_H_
#define PROTOCOLS_SPDZ2KSHARE_H_

#include "Math/Z2k.h"
#include "Protocols/Share.h"
#include "Protocols/MAC_Check.h"
#include "Processor/DummyProtocol.h"
#include "OT/Rectangle.h"

template<int K, int S> class Spdz2kMultiplier;
template<class T> class Spdz2kTripleGenerator;

namespace GC
{
template<int S> class TinySecret;
}

template<int K, int S>
class Spdz2kShare : public Share<Z2<K + S>>
{
public:
    typedef Z2<K + S> tmp_type;
    typedef Share<tmp_type> super;

//    typedef Integer clear;
    typedef SignedZ2<K> clear;

    typedef Z2<S> mac_key_type;
    typedef Z2<K + S> open_type;

    typedef Spdz2kShare prep_type;
    typedef Spdz2kShare input_check_type;
    typedef Spdz2kMultiplier<K, S> Multiplier;
    typedef Spdz2kTripleGenerator<Spdz2kShare> TripleGenerator;
    typedef Z2<K + 2 * S> sacri_type;
    typedef Z2kRectangle<TAU(K, S), K + S> Rectangle;

    typedef MAC_Check_Z2k<Z2<K + S>, Z2<S>, open_type, Spdz2kShare> MAC_Check;
    typedef MAC_Check Direct_MC;
    typedef ::Input<Spdz2kShare> Input;
    typedef ::PrivateOutput<Spdz2kShare> PrivateOutput;
    typedef SPDZ<Spdz2kShare> Protocol;
    typedef Spdz2kPrep<Spdz2kShare> LivePrep;

#ifndef NO_MIXED_CIRCUITS
#ifdef SPDZ2K_BIT
    typedef GC::TinySecret<S> bit_type;
#else
    typedef GC::TinierSecret<gf2n_short> bit_type;
#endif
#endif

    const static int k = K;
    const static int s = S;

    static string type_string() { return "SPDZ2^(" + to_string(K) + "+" + to_string(S) + ")"; }
    static string type_short() { return "Z" + to_string(K) + "," + to_string(S); }

    Spdz2kShare() {}
    template<class T, class V>
    Spdz2kShare(const Share_<T, V>& x) : super(x) {}
    template<class T, class V>
    Spdz2kShare(const T& share, const V& mac) : super(share, mac) {}
};


template<int K, int S>
Spdz2kShare<K, S> operator*(const typename Spdz2kShare<K,S>::open_type& x, Spdz2kShare<K, S>& y)
{
    return typename Spdz2kShare<K,S>::tmp_type(x) * typename Spdz2kShare<K,S>::super(y);
}

#endif /* PROTOCOLS_SPDZ2KSHARE_H_ */
