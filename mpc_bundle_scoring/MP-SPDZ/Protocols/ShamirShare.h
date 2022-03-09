/*
 * ShamirShare.h
 *
 */

#ifndef PROTOCOLS_SHAMIRSHARE_H_
#define PROTOCOLS_SHAMIRSHARE_H_

#include "Protocols/Shamir.h"
#include "Protocols/ShamirInput.h"
#include "Machines/ShamirMachine.h"
#include "ShareInterface.h"

template<class T> class ReplicatedPrep;
template<class T> class ReplicatedRingPrep;

namespace GC
{
template<class T> class CcdSecret;
}

template<class T>
class ShamirShare : public T, public ShareInterface
{
public:
    typedef T clear;
    typedef T open_type;
    typedef T mac_key_type;
    typedef void sacri_type;
    typedef GC::NoShare mac_type;
    typedef GC::NoShare mac_share_type;

    typedef Shamir<ShamirShare> Protocol;
    typedef IndirectShamirMC<ShamirShare> MAC_Check;
    typedef ShamirMC<ShamirShare> Direct_MC;
    typedef ShamirInput<ShamirShare> Input;
    typedef ::PrivateOutput<ShamirShare> PrivateOutput;
    typedef ReplicatedPrep<ShamirShare> LivePrep;
    typedef ReplicatedRingPrep<ShamirShare> TriplePrep;
    typedef ShamirShare Honest;

#ifndef NO_MIXED_CIRCUITS
    typedef GC::CcdSecret<gf2n_<octet>> bit_type;
#endif

    const static bool needs_ot = false;
    const static bool dishonest_majority = false;
    const static bool variable_players = true;
    const static bool expensive = false;

    static string type_short()
    {
        auto res = "S" + string(1, clear::type_char());
        auto opts = ShamirOptions::singleton;
        if (opts.threshold != (opts.nparties - 1) / 2)
            res += "T" + to_string(opts.threshold);
        return res;
    }
    static string type_string()
    {
        return "Shamir " + T::type_string();
    }

    static int threshold(int)
    {
        return ShamirMachine::s().threshold;
    }

    static T get_rec_factor(int i, int n)
    {
        return Protocol::get_rec_factor(i, n);
    }

    static ShamirShare constant(T value, int my_num, const T& alphai = {})
    {
        return ShamirShare(value, my_num, alphai);
    }

    ShamirShare()
    {
    }
    template<class U>
    ShamirShare(const U& other)
    {
        T::operator=(other);
    }
    template<class U>
    ShamirShare(const U& other, int my_num, T alphai = {}) : ShamirShare(other)
    {
        (void) my_num, (void) alphai;
    }

    // Share<T> compatibility
    void assign(clear other, int my_num, const T& alphai)
    {
        (void)alphai, (void)my_num;
        *this = other;
    }
    void assign(const char* buffer)
    {
        T::assign(buffer);
    }

    void add(const ShamirShare& S, const clear aa, int my_num,
            const T& alphai)
    {
        (void) my_num, (void) alphai;
        *this = S + aa;
    }
    void sub(const ShamirShare& S, const clear& aa, int my_num,
            const T& alphai)
    {
        (void) my_num, (void) alphai;
        *this = S - aa;
    }
    void sub(const clear& aa, const ShamirShare& S, int my_num,
            const T& alphai)
    {
        (void) my_num, (void) alphai;
        *this = aa - S;
    }

    ShamirShare operator<<(int i)
    {
        return *this * (T(1) << i);
    }
    ShamirShare& operator<<=(int i)
    {
        *this = *this << i;
        return *this;
    }

    void force_to_bit()
    {
        throw not_implemented();
    }

    ShamirShare get_bit(int)
    {
        throw runtime_error("never call this");
    }

    void pack(octetStream& os, const T& rec_factor) const
    {
        (*this * rec_factor).pack(os);
    }
    void pack(octetStream& os) const
    {
        T::pack(os);
    }
    void unpack(octetStream& os, bool full = true)
    {
        assert(full);
        T::unpack(os);
    }
};

#endif /* PROTOCOLS_SHAMIRSHARE_H_ */
