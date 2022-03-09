/*
 * RingOptions.cpp
 *
 */

#include "RingOptions.h"
#include "BaseMachine.h"

#include <iostream>
using namespace std;

RingOptions::RingOptions(ez::ezOptionParser& opt, int argc, const char** argv,
        bool security)
{
    opt.add(
        "64", // Default.
        0, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Number of integer bits (default: 64)", // Help description.
        "-R", // Flag token.
        "--ring" // Flag token.
    );
    if (security)
        opt.add(
            "40", // Default.
            0, // Required?
            1, // Number of args expected.
            0, // Delimiter if expecting multiple args.
            "Security parameter (default: 40)", // Help description.
            "-S", // Flag token.
            "--security" // Flag token.
        );
    opt.parse(argc, argv);
    opt.get("-R")->getInt(R);
    if (security)
        opt.get("-S")->getInt(S);
    else
        S = -1;
    R_is_set = opt.isSet("-R");
    opt.resetArgs();
    if (R_is_set)
        cerr << "Trying to run " << R << "-bit computation" << endl;
    if (security)
        cerr << "Using security parameter " << S << endl;
}

int RingOptions::ring_size_from_opts_or_schedule(string progname)
{
    if (R_is_set)
        return R;
    int r = BaseMachine::ring_size_from_schedule(progname);
    if (r == 0)
        r = R;
    cerr << "Trying to run " << r << "-bit computation" << endl;
    return r;
}
