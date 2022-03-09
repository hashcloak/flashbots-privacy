/*
 * NoShare.h
 *
 */

#ifndef GC_NOSHARE_H_
#define GC_NOSHARE_H_

#include "Processor/DummyProtocol.h"
#include "Protocols/ShareInterface.h"

class InputArgs;
class ArithmeticProcessor;
class BlackHole;
class SwitchableOutput;

namespace GC
{

template<class T> class Processor;
class Clear;

class NoValue : public ValueInterface
{
public:
    typedef NoValue Scalar;
    typedef NoValue next;

    const static int n_bits = 0;
    const static int MAX_N_BITS = 0;

    static bool allows(Dtype)
    {
        return false;
    }

    static int size()
    {
        return 0;
    }

    static int length()
    {
        return 0;
    }

    static string type_string()
    {
        return "no";
    }

    static string type_short()
    {
        return "no";
    }

    static DataFieldType field_type()
    {
        throw not_implemented();
    }

    static void fail()
    {
        throw runtime_error("VM does not support binary circuits");
    }

    NoValue() {}
    NoValue(int) { fail(); }

    void assign(const char*) { fail(); }

    int get() const { fail(); return 0; }

    int operator<<(int) const { fail(); return 0; }
    void operator+=(int) { fail(); }

    bool operator!=(NoValue) const { fail(); return 0; }

    bool operator==(int) { fail(); return false; }

    bool get_bit(int) { fail(); return 0; }

    void randomize(PRNG&) { fail(); }

    void invert() { fail(); }

    void mask(int) { fail(); }

    void input(istream&, bool) { fail(); }
    void output(ostream&, bool) { fail(); }
};

inline ostream& operator<<(ostream& o, NoValue)
{
    return o;
}

class NoShare : public ShareInterface
{
public:
    typedef DummyMC<NoShare> MC;
    typedef DummyProtocol<NoShare> Protocol;
    typedef NotImplementedInput<NoShare> Input;
    typedef DummyLivePrep<NoShare> LivePrep;
    typedef DummyMC<NoShare> MAC_Check;

    typedef NoValue open_type;
    typedef NoValue clear;
    typedef NoValue mac_key_type;

    typedef NoShare small_type;

    typedef SwitchableOutput out_type;

    static const bool is_real = false;

    static MC* new_mc(mac_key_type)
    {
        return new MC;
    }

    static string type_string()
    {
        return "no";
    }

    static void specification(octetStream&)
    {
        fail();
    }

    static int size()
    {
        return 0;
    }

    static void fail()
    {
        NoValue::fail();
    }

    static void inputb(Processor<NoShare>&, const ArithmeticProcessor&, const vector<int>&) { fail(); }
    static void inputbvec(Processor<NoShare>&, const ArithmeticProcessor&, const vector<int>&) { fail(); }
    static void reveal_inst(Processor<NoShare>&, const vector<int>&) { fail(); }
    static void xors(Processor<NoShare>&, const vector<int>&) { fail(); }
    static void ands(Processor<NoShare>&, const vector<int>&) { fail(); }
    static void andrs(Processor<NoShare>&, const vector<int>&) { fail(); }

    static void trans(Processor<NoShare>&, Integer, const vector<int>&) { fail(); }

    static NoShare constant(const GC::Clear&, int, mac_key_type, int = -1) { fail(); return {}; }

    NoShare() {}

    NoShare(int) { fail(); }

    void load_clear(Integer, Integer) { fail(); }
    void random_bit() { fail(); }
    void bitdec(vector<NoShare>&, const vector<int>&) const { fail(); }
    void bitcom(vector<NoShare>&, const vector<int>&) const { fail(); }

    void assign(const char*) { fail(); }

    NoShare operator&(const Clear&) const { fail(); return {}; }

    NoShare operator<<(int) const { fail(); return {}; }
    void operator^=(NoShare) { fail(); }

    NoShare operator+(const NoShare&) const { fail(); return {}; }
    NoShare operator-(const NoShare&) const { fail(); return {}; }
    NoShare operator*(const NoValue&) const { fail(); return {}; }

    NoShare operator&(int) const { fail(); return {}; }
    NoShare operator>>(int) const { fail(); return {}; }

    NoShare& operator+=(const NoShare&) { fail(); return *this; }

    NoShare get_bit(int) const { fail(); return {}; }

    void xor_bit(int, NoShare) const { fail(); }

    void invert(int, NoShare) { fail(); }

    NoShare mask(int) const { fail(); return {}; }
    void mask(NoShare, int) const { fail(); }

    void input(istream&, bool) { fail(); }
    void output(ostream&, bool) { fail(); }
};

} /* namespace GC */

#endif /* GC_NOSHARE_H_ */
