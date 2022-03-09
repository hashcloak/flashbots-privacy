/*
 * BrainShare.h
 *
 */

#ifndef PROTOCOLS_BRAINSHARE_H_
#define PROTOCOLS_BRAINSHARE_H_

#include "Rep3Share.h"

template<class T> class HashMaliciousRepMC;
template<class T> class Beaver;
template<class T> class BrainPrep;

namespace GC
{
class MaliciousRepSecret;
}

template<int K, int S>
class BrainShare : public Rep3Share<SignedZ2<K>>
{
    typedef SignedZ2<K> T;
    typedef Rep3Share<T> super;

public:
    typedef T clear;

    typedef Beaver<BrainShare> Protocol;
    typedef HashMaliciousRepMC<BrainShare> MAC_Check;
    typedef MAC_Check Direct_MC;
    typedef ReplicatedInput<BrainShare> Input;
    typedef ::PrivateOutput<BrainShare> PrivateOutput;
    typedef BrainPrep<BrainShare> LivePrep;

    typedef GC::MaliciousRepSecret bit_type;

    const static int N_MASK_BITS = clear::N_BITS + S;
    const static int Z_BITS = 2 * (N_MASK_BITS) + 5 + S;

    BrainShare()
    {
    }
    template<class U>
    BrainShare(const FixedVec<U, 2>& other)
    {
        FixedVec<T, 2>::operator=(other);
    }
    template<class U>
    BrainShare(const U& other, int my_num, T alphai = {}) : super(other, my_num, alphai)
    {
    }
};

#endif /* PROTOCOLS_BRAINSHARE_H_ */
