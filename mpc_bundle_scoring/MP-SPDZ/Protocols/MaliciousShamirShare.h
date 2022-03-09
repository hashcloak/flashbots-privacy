/*
 * MaliciousShamirShare.h
 *
 */

#ifndef PROTOCOLS_MALICIOUSSHAMIRSHARE_H_
#define PROTOCOLS_MALICIOUSSHAMIRSHARE_H_

#include "ShamirShare.h"
#include "Protocols/Beaver.h"
#include "Protocols/MaliciousShamirMC.h"

template<class T> class MaliciousRepPrepWithBits;
template<class T> class MaliciousRepPrep;
template<class T> class MaliciousShamirPO;

namespace GC
{
template<class T> class MaliciousCcdSecret;
}

template<class T>
class MaliciousShamirShare : public ShamirShare<T>
{
    typedef ShamirShare<T> super;

public:
    typedef Beaver<MaliciousShamirShare<T>> Protocol;
    typedef MaliciousShamirMC<MaliciousShamirShare> MAC_Check;
    typedef MAC_Check Direct_MC;
    typedef ShamirInput<MaliciousShamirShare> Input;
    typedef ::PrivateOutput<MaliciousShamirShare> PrivateOutput;
    typedef MaliciousShamirPO<MaliciousShamirShare> PO;
    typedef ShamirShare<T> Honest;
    typedef MaliciousRepPrepWithBits<MaliciousShamirShare> LivePrep;
    typedef MaliciousRepPrep<MaliciousShamirShare> TriplePrep;
    typedef T random_type;

#ifndef NO_MIXED_CIRCUITS
    typedef GC::MaliciousCcdSecret<gf2n_short> bit_type;
#endif

    static string type_short()
    {
        return "M" + super::type_short();
    }

    MaliciousShamirShare()
    {
    }
    template<class U>
    MaliciousShamirShare(const U& other, int my_num = 0, T alphai = {}) : super(other)
    {
        (void) my_num, (void) alphai;
    }
};

#endif /* PROTOCOLS_MALICIOUSSHAMIRSHARE_H_ */
