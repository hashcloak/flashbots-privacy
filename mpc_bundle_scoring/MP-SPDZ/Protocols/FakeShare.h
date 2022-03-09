/*
 * FakeShare.h
 *
 */

#ifndef PROTOCOLS_FAKESHARE_H_
#define PROTOCOLS_FAKESHARE_H_

#include "GC/FakeSecret.h"
#include "ShareInterface.h"
#include "FakeMC.h"
#include "FakeProtocol.h"
#include "FakePrep.h"
#include "FakeInput.h"

template<class T>
class FakeShare : public T, public ShareInterface
{
    typedef FakeShare This;

public:
    typedef T mac_key_type;
    typedef T open_type;
    typedef T clear;

    typedef FakePrep<This> LivePrep;
    typedef FakeMC<This> MAC_Check;
    typedef MAC_Check Direct_MC;
    typedef FakeInput<This> Input;
    typedef ::PrivateOutput<This> PrivateOutput;
    typedef FakeProtocol<This> Protocol;

    typedef GC::FakeSecret bit_type;

    static string type_short()
    {
        return "emul";
    }

    static int threshold(int)
    {
        return 0;
    }

    static T constant(T value, int = 0, T = 0)
    {
        return value;
    }

    FakeShare()
    {
    }

    template<class U>
    FakeShare(U other) :
            T(other)
    {
    }

    static void split(vector<bit_type>& dest, const vector<int>& regs,
            int n_bits, const This* source, int n_inputs,
            GC::FakeSecret::Protocol& protocol);
};

#endif /* PROTOCOLS_FAKESHARE_H_ */
