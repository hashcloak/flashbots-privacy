/*
 * Rep3Share.h
 *
 */

#ifndef PROTOCOLS_REP3SHARE_H_
#define PROTOCOLS_REP3SHARE_H_

#include "Math/FixedVec.h"
#include "Math/Integer.h"
#include "Protocols/Replicated.h"
#include "GC/ShareSecret.h"
#include "ShareInterface.h"

template<class T> class ReplicatedPrep;
template<class T> class ReplicatedRingPrep;
template<class T> class PrivateOutput;

template<class T, int L>
class RepShare : public FixedVec<T, L>, public ShareInterface
{
    typedef RepShare This;
    typedef FixedVec<T, L> super;

public:
    typedef T clear;
    typedef T open_type;
    typedef T mac_type;
    typedef T mac_key_type;

    const static bool needs_ot = false;
    const static bool dishonest_majority = false;
    const static bool expensive = false;

    static int threshold(int)
    {
        return 1;
    }

    static void specification(octetStream& os)
    {
        T::specification(os);
    }

    RepShare()
    {
    }
    template<class U>
    RepShare(const U& other) :
            super(other)
    {
    }

    void pack(octetStream& os, T) const
    {
        pack(os, false);
    }
    void pack(octetStream& os, bool full = true) const
    {
        if (full)
            FixedVec<T, L>::pack(os);
        else
            (*this)[0].pack(os);
    }
    void unpack(octetStream& os, bool full = true)
    {
        assert(full);
        FixedVec<T, L>::unpack(os);
    }
};

template<class T>
class Rep3Share : public RepShare<T, 2>
{
    typedef RepShare<T, 2> super;

public:
    typedef T clear;

    typedef Replicated<Rep3Share> Protocol;
    typedef ReplicatedMC<Rep3Share> MAC_Check;
    typedef MAC_Check Direct_MC;
    typedef ReplicatedInput<Rep3Share> Input;
    typedef ::PrivateOutput<Rep3Share> PrivateOutput;
    typedef ReplicatedPrep<Rep3Share> LivePrep;
    typedef ReplicatedRingPrep<Rep3Share> TriplePrep;
    typedef Rep3Share Honest;

    typedef Rep3Share Scalar;

    typedef GC::SemiHonestRepSecret bit_type;

    const static bool needs_ot = false;
    const static bool dishonest_majority = false;
    const static bool expensive = false;
    const static bool variable_players = false;

    static string type_short()
    {
        return "R" + string(1, clear::type_char());
    }
    static string type_string()
    {
        return "replicated " + T::type_string();
    }
    static char type_char()
    {
        return T::type_char();
    }

    static Rep3Share constant(T value, int my_num, const T& alphai = {})
    {
        return Rep3Share(value, my_num, alphai);
    }

    Rep3Share()
    {
    }
    template<class U>
    Rep3Share(const U& other) :
            super(other)
    {
    }

    Rep3Share(T value, int my_num, const T& alphai = {})
    {
        (void) alphai;
        Replicated<Rep3Share>::assign(*this, value, my_num);
    }

    void assign(const char* buffer)
    {
        FixedVec<T, 2>::assign(buffer);
    }

    clear local_mul(const Rep3Share& other) const
    {
        auto a = (*this)[0].lazy_mul(other.lazy_sum());
        auto b = (*this)[1].lazy_mul(other[0]);
        return a.lazy_add(b);
    }
};

#endif /* PROTOCOLS_REP3SHARE_H_ */
