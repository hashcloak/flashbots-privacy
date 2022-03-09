/*
 * ShareInterface.h
 *
 */

#ifndef PROTOCOLS_SHAREINTERFACE_H_
#define PROTOCOLS_SHAREINTERFACE_H_

#include <vector>
#include <string>
#include <stdexcept>
using namespace std;

#include "Tools/Exceptions.h"

class Player;
class Instruction;
class ValueInterface;

namespace GC
{
class NoShare;
}

class ShareInterface
{
public:
    typedef GC::NoShare part_type;
    typedef GC::NoShare bit_type;

    static const bool needs_ot = false;
    static const bool expensive = false;
    static const bool expensive_triples = false;

    static const bool has_trunc_pr = false;
    static const bool has_split = false;

    static const false_type triple_matmul;

    static const int default_length = 1;

    static string type_short() { return "undef"; }

    template<class T, class U>
    static void split(vector<U>, vector<int>, int, T*, int,
            typename U::Protocol&)
    { throw runtime_error("split not implemented"); }

    template<class T>
    static void shrsi(T&, const Instruction&)
    { throw runtime_error("shrsi not implemented"); }

    static bool get_rec_factor(int, int) { return false; }

    template<class T>
    static void read_or_generate_mac_key(const string&, const Player&, T&) {}

    template<class T, class U>
    static void generate_mac_key(T&, U&) {}
};

#endif /* PROTOCOLS_SHAREINTERFACE_H_ */
