/*
 * tinier-party.cpp
 *
 */

#include "GC/TinierSecret.h"
#include "GC/ShareParty.h"
#include "GC/TinyMC.h"
#include "GC/VectorInput.h"

#include "GC/ShareParty.hpp"
#include "GC/ShareSecret.hpp"
#include "GC/Machine.hpp"
#include "GC/Processor.hpp"
#include "GC/Program.hpp"
#include "GC/Thread.hpp"
#include "GC/ThreadMaster.hpp"
#include "GC/Secret.hpp"
#include "GC/TinyPrep.hpp"
#include "GC/TinierSharePrep.hpp"
#include "GC/CcdPrep.hpp"

#include "Processor/Instruction.hpp"
#include "Protocols/MAC_Check.hpp"
#include "Protocols/MAC_Check_Base.hpp"
#include "Protocols/Beaver.hpp"
#include "Protocols/MascotPrep.hpp"

int main(int argc, const char** argv)
{
    gf2n_short::init_field(40);
    GC::simple_binary_main<GC::TinierSecret<gf2n_short>>(argc, argv, 1000);
}
