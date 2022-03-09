/*
 * MalRepRingShare.h
 *
 */

#ifndef PROTOCOLS_MALREPRINGSHARE_H_
#define PROTOCOLS_MALREPRINGSHARE_H_

#include "Math/Z2k.h"
#include "Protocols/MaliciousRep3Share.h"

template<class T> class MalRepRingPrepWithBits;
template<class T> class MalRepRingPrep;

template<int K, int S>
class MalRepRingShare : public MaliciousRep3Share<SignedZ2<K>>
{
    typedef SignedZ2<K> T;
    typedef MaliciousRep3Share<T> super;

public:
    const static int BIT_LENGTH = K;
    const static int SECURITY = S;

    typedef Beaver<MalRepRingShare> Protocol;
    typedef HashMaliciousRepMC<MalRepRingShare> MAC_Check;
    typedef MAC_Check Direct_MC;
    typedef ReplicatedInput<MalRepRingShare> Input;
    typedef ::PrivateOutput<MalRepRingShare> PrivateOutput;
    typedef MalRepRingPrepWithBits<MalRepRingShare> LivePrep;
    typedef MaliciousRep3Share<Z2<K + S>> prep_type;
    typedef Z2<S> random_type;
    typedef MalRepRingShare<K + 2, S> SquareToBitShare;
    typedef MalRepRingPrep<MalRepRingShare> SquarePrep;

    static string type_short()
    {
        return "RR";
    }

    MalRepRingShare()
    {
    }
    MalRepRingShare(const T& other, int my_num, T alphai = {}) :
            super(other, my_num, alphai)
    {
    }
    template<class U>
    MalRepRingShare(const U& other) : super(other)
    {
    }
};

#endif /* PROTOCOLS_MALREPRINGSHARE_H_ */
