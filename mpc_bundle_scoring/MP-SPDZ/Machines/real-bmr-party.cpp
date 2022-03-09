/*
 * real-bmr-party.cpp
 *
 */

#define NO_MIXED_CIRCUITS

#include "BMR/RealProgramParty.hpp"
#include "Machines/SPDZ.hpp"

int main(int argc, const char** argv)
{
	RealProgramParty<Share<gf2n_long>>(argc, argv);
}
