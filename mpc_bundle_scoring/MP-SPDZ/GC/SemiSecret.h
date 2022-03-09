/*
 * SemiSecret.h
 *
 */

#ifndef GC_SEMISECRET_H_
#define GC_SEMISECRET_H_

#include "Protocols/SemiMC.h"
#include "Protocols/SemiShare.h"
#include "Processor/DummyProtocol.h"
#include "ShareSecret.h"

template<class T> class Beaver;

namespace GC
{

class SemiPrep;

class SemiSecret : public SemiShare<BitVec>, public ShareSecret<SemiSecret>
{
public:
    typedef Memory<SemiSecret> DynamicMemory;

    typedef SemiMC<SemiSecret> MC;
    typedef DirectSemiMC<SemiSecret> Direct_MC;
    typedef Beaver<SemiSecret> Protocol;
    typedef MC MAC_Check;
    typedef SemiPrep LivePrep;
    typedef SemiInput<SemiSecret> Input;

    typedef SemiSecret part_type;
    typedef SemiSecret small_type;

    static const int default_length = sizeof(BitVec) * 8;

    static string type_string() { return "binary secret"; }
    static string phase_name() { return "Binary computation"; }

    static MC* new_mc(mac_key_type);

    template<class T>
    static void generate_mac_key(mac_key_type, T)
    {
    }

    static void trans(Processor<SemiSecret>& processor, int n_outputs,
            const vector<int>& args);

    SemiSecret()
    {
    }
    SemiSecret(long other) :
            SemiShare<BitVec>(other)
    {
    }
    SemiSecret(const IntBase& other) :
            SemiShare<BitVec>(other)
    {
    }
    template<int K>
    SemiSecret(const Z2<K>& other) :
            SemiShare<BitVec>(other)
    {
    }

    void load_clear(int n, const Integer& x);

    void bitcom(Memory<SemiSecret>& S, const vector<int>& regs);
    void bitdec(Memory<SemiSecret>& S, const vector<int>& regs) const;

    void xor_(int n, const SemiSecret& x, const SemiSecret& y)
    { *this = BitVec(x ^ y).mask(n); }

    void xor_bit(int i, const SemiSecret& bit)
    { *this ^= bit << i; }

    void reveal(size_t n_bits, Clear& x);

    SemiSecret lsb()
    { return *this & 1; }
};

} /* namespace GC */

#endif /* GC_SEMISECRET_H_ */
