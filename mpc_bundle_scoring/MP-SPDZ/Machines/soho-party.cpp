/*
 * soho-party.cpp
 *
 */

#include "Protocols/SohoShare.h"
#include "Math/gfp.h"
#include "Math/gf2n.h"
#include "FHE/P2Data.h"
#include "Tools/ezOptionParser.h"
#include "GC/SemiSecret.h"
#include "GC/SemiPrep.h"

#include "Processor/FieldMachine.hpp"
#include "Protocols/HemiPrep.hpp"
#include "Processor/Data_Files.hpp"
#include "Processor/Instruction.hpp"
#include "Processor/Machine.hpp"
#include "Protocols/SohoPrep.hpp"
#include "Protocols/SemiInput.hpp"
#include "Protocols/MAC_Check_Base.hpp"
#include "Protocols/MAC_Check.hpp"
#include "Protocols/SemiMC.hpp"
#include "Protocols/Beaver.hpp"
#include "GC/ShareSecret.hpp"
#include "GC/SemiHonestRepPrep.h"
#include "Math/gfp.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    DishonestMajorityFieldMachine<SohoShare, SohoShare, gf2n_short>(argc, argv,
            opt);
}
