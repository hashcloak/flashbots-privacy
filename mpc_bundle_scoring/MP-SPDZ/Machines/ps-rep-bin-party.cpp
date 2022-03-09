/*
 * abfllnow-party.cpp
 *
 */

#include "GC/PostSacriBin.h"
#include "GC/PostSacriSecret.h"

#include "GC/ShareParty.hpp"

int main(int argc, const char** argv)
{
    GC::simple_binary_main<GC::PostSacriSecret>(argc, argv);
}
