/*
 * spdz2k-party.cpp
 *
 */

#include "GC/TinierSecret.h"
#include "Processor/Machine.h"
#include "Processor/RingOptions.h"
#include "Protocols/Spdz2kShare.h"
#include "Math/gf2n.h"
#include "Networking/Server.h"

#include "Processor/OnlineMachine.hpp"
#include "Math/Z2k.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    opt.add(
        "64", // Default.
        0, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "SPDZ2k security parameter (default: 64)", // Help description.
        "-S", // Flag token.
        "--security" // Flag token.
    );
    opt.parse(argc, argv);
    int s;
    opt.get("-S")->getInt(s);
    opt.resetArgs();
    RingOptions ring_options(opt, argc, argv);
    int k = ring_options.R;
#ifdef VERBOSE
    cerr << "Using SPDZ2k with ring length " << k << " and security parameter "
            << s << endl;
#endif

#undef Z
#define Z(K, S) \
    if (s == S and k == K) \
        return spdz_main<Spdz2kShare<K, S>, Share<gf2n>>(argc, argv, opt);

    Z(64, 64)
    Z(64, 48)
    Z(72, 64)
    Z(72, 48)

    else
        throw runtime_error(
                "not compiled for k=" + to_string(k) + " and s=" + to_string(s));
}
