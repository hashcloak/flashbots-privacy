/*
 * atlas-party.cpp
 *
 */

#include "Protocols/AtlasShare.h"
#include "Protocols/AtlasPrep.h"
#include "GC/AtlasSecret.h"

#include "ShamirMachine.hpp"
#include "Protocols/Atlas.hpp"

int main(int argc, const char** argv)
{
    ShamirMachineSpec<AtlasShare>(argc, argv);
}
