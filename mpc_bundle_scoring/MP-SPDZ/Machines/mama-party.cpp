/*
 * mama-party.cpp
 *
 */

#include "Protocols/MamaShare.h"

#include "Protocols/MamaPrep.hpp"
#include "Protocols/MascotPrep.hpp"
#include "Processor/FieldMachine.hpp"
#include "SPDZ.hpp"
#include "Math/gfp.hpp"

#ifndef N_MAMA_MACS
#define N_MAMA_MACS 3
#endif

template<class T>
using MamaShare_ = MamaShare<T, N_MAMA_MACS>;

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    DishonestMajorityFieldMachine<MamaShare_, Share>(argc, argv, opt);
}
