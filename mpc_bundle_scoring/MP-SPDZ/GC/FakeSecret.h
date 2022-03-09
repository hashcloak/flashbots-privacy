/*
 * Secret.h
 *
 */

#ifndef GC_FAKESECRET_H_
#define GC_FAKESECRET_H_

#include "GC/Clear.h"
#include "GC/Memory.h"
#include "GC/Access.h"
#include "GC/ArgTuples.h"

#include "Math/gf2nlong.h"
#include "Tools/SwitchableOutput.h"

#include "Processor/DummyProtocol.h"
#include "Protocols/FakePrep.h"
#include "Protocols/FakeMC.h"
#include "Protocols/FakeProtocol.h"
#include "Protocols/FakeInput.h"
#include "Protocols/ShareInterface.h"

#include <random>
#include <fstream>

class ProcessorBase;

namespace GC
{

template <class T>
class Processor;
template <class T>
class Machine;

class FakeSecret : public ShareInterface, public BitVec
{
public:
    typedef FakeSecret DynamicType;
    typedef Memory<FakeSecret> DynamicMemory;

    typedef BitVec mac_key_type;
    typedef BitVec clear;
    typedef BitVec open_type;

    typedef FakeSecret part_type;
    typedef FakeSecret small_type;
    typedef NoShare bit_type;

    typedef FakePrep<FakeSecret> LivePrep;
    typedef FakeMC<FakeSecret> MC;
    typedef MC MAC_Check;
    typedef MC Direct_MC;
    typedef FakeProtocol<FakeSecret> Protocol;
    typedef FakeInput<FakeSecret> Input;

    typedef SwitchableOutput out_type;

    static string type_string() { return "fake secret"; }
    static string type_short() { return "emulB"; }
    static string phase_name() { return "Faking"; }

    static const int default_length = 64;

    static const bool is_real = true;

    static const bool actual_inputs = true;

    static const true_type invertible;
    static const true_type characteristic_two;

    static int threshold(int) { return 0; }

    static MC* new_mc(mac_key_type key) { return new MC(key); }

    static void store_clear_in_dynamic(Memory<DynamicType>& mem,
    		const vector<GC::ClearWriteAccess>& accesses);

    static void load(vector< ReadAccess<FakeSecret> >& accesses, const Memory<FakeSecret>& mem);
    static void store(Memory<FakeSecret>& mem, vector< WriteAccess<FakeSecret> >& accesses);

    template <class T>
    static void andrs(T& processor, const vector<int>& args)
    { processor.andrs(args); }
    static void ands(GC::Processor<FakeSecret>& processor, const vector<int>& regs);
    template <class T>
    static void xors(GC::Processor<T>& processor, const vector<int>& regs)
    { processor.xors(regs); }
    template <class T>
    static void inputb(T& processor, const vector<int>& args)
    { processor.input(args); }
    template <class T>
    static void inputb(T& processor, ArithmeticProcessor&, const vector<int>& args)
    { processor.input(args); }
    static void inputbvec(Processor<FakeSecret>& processor,
            ProcessorBase& input_processor, const vector<int>& args);
    template <class T>
    static void reveal_inst(T& processor, const vector<int>& args)
    { processor.reveal(args); }

    static void trans(Processor<FakeSecret>& processor, int n_inputs,
            const vector<int>& args);

    template <class T>
    static void convcbit(Integer& dest, const Clear& source, T&) { dest = source; }

    template<class U>
    static void convcbit2s(GC::Processor<U>&, const BaseInstruction&)
    { throw runtime_error("convcbit2s not implemented"); }

    static FakeSecret input(GC::Processor<FakeSecret>& processor, const InputArgs& args);
    static FakeSecret input(int from, word input, int n_bits);

    static FakeSecret constant(clear value, int = 0, mac_key_type = {}, int = -1)
    { return value; }

    FakeSecret() {}
    template <class T>
    FakeSecret(T other) : BitVec(other) {}

    __uint128_t operator>>(const FakeSecret& other) const { return a >> other.a; }
    __uint128_t operator<<(const FakeSecret& other) const { return a << other.a; }

    __uint128_t operator^=(const FakeSecret& other) { return a ^= other.a; }

    void load_clear(int n, const Integer& x);
    template <class T>
    void load(int n, const Memory<T>& mem, size_t address) { load(n, mem[address]); }
    template <class T>
    void store(Memory<T>& mem, size_t address) { mem[address] = *this; }

    void bitcom(Memory<FakeSecret>& S, const vector<int>& regs);
    void bitdec(Memory<FakeSecret>& S, const vector<int>& regs) const;

    template <class T>
    void xor_(int n, const FakeSecret& x, const T& y)
    { *this = BitVec(x.a ^ y.a).mask(n); }
    void and_(int n, const FakeSecret& x, const FakeSecret& y, bool repeat);
    void andrs(int n, const FakeSecret& x, const FakeSecret& y)
    { *this = BitVec(x.a * (y.a & 1)).mask(n); }

    void xor_bit(int i, FakeSecret bit) { *this ^= bit << i; }

    void invert(int n, const FakeSecret& x) { *this = BitVec(~x.a).mask(n); }

    void random_bit() { a = random() % 2; }

    void reveal(int n_bits, Clear& x) { (void) n_bits; x = a; }

    FakeSecret invert() const { throw not_implemented(); }

    void input(istream&, bool) { throw not_implemented(); }

    bool operator<(FakeSecret) const { return false; }

    void my_input(Input& inputter, BitVec value, int n_bits);
    void other_input(Input& inputter, int from, int n_bits = 1);
    void finalize_input(Input& inputter, int from, int n_bits);
};

} /* namespace GC */

#endif /* GC_FAKESECRET_H_ */
