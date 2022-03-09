/*
 * SemiShare.h
 *
 */

#ifndef PROTOCOLS_SEMISHARE_H_
#define PROTOCOLS_SEMISHARE_H_

#include "Protocols/Beaver.h"
#include "Processor/DummyProtocol.h"
#include "ShareInterface.h"

#include <string>
using namespace std;

template<class T> class Input;
template<class T> class SemiMC;
template<class T> class DirectSemiMC;
template<class T> class SPDZ;
template<class T> class SemiPrep;
template<class T> class SemiInput;
template<class T> class PrivateOutput;
template<class T> class SemiMultiplier;
template<class T> class OTTripleGenerator;

namespace GC
{
class SemiSecret;
class NoValue;
}

template<class T>
class BasicSemiShare : public T
{
public:
    typedef T open_type;
    typedef T clear;

    typedef GC::NoValue mac_key_type;

    template<class U>
    BasicSemiShare(const U& other) : T(other)
    {
    }
};

template<class T>
class SemiShare : public T, public ShareInterface
{
    typedef T super;

public:
    typedef T mac_key_type;
    typedef T mac_type;
    typedef T open_type;
    typedef T clear;

    typedef SemiMC<SemiShare> MAC_Check;
    typedef DirectSemiMC<SemiShare> Direct_MC;
    typedef SemiInput<SemiShare> Input;
    typedef ::PrivateOutput<SemiShare> PrivateOutput;
    typedef SPDZ<SemiShare> Protocol;
    typedef SemiPrep<SemiShare> LivePrep;
    typedef LivePrep TriplePrep;

    typedef SemiShare<typename T::next> prep_type;
    typedef SemiMultiplier<SemiShare> Multiplier;
    typedef OTTripleGenerator<prep_type> TripleGenerator;
    typedef T sacri_type;
    typedef typename T::Square Rectangle;

    typedef GC::SemiSecret bit_type;

    const static bool needs_ot = true;
    const static bool dishonest_majority = true;
    const static bool variable_players = true;
    const static bool expensive = false;

    static string type_short() { return "D" + string(1, T::type_char()); }

    static int threshold(int nplayers)
    {
        return nplayers - 1;
    }

    static SemiShare constant(const clear& other, int my_num,
            const T& alphai = {}, int = -1)
    {
        return SemiShare(other, my_num, alphai);
    }

    SemiShare()
    {
    }
    template<class U>
    SemiShare(const U& other) : T(other)
    {
    }
    SemiShare(const clear& other, int my_num, const T& alphai = {})
    {
        (void) alphai;
        Protocol::assign(*this, other, my_num);
    }

    void assign(const char* buffer)
    {
        super::assign(buffer);
    }

    void pack(octetStream& os, bool full = true) const
    {
        (void)full;
        super::pack(os);
    }
    void unpack(octetStream& os, bool full = true)
    {
        (void)full;
        super::unpack(os);
    }

    void pack(octetStream& os, int n_bits) const
    {
        super::pack(os, n_bits);
    }
    void unpack(octetStream& os, int n_bits)
    {
        super::unpack(os, n_bits);
    }
};

#endif /* PROTOCOLS_SEMISHARE_H_ */
