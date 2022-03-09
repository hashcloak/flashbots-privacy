/*
 * ExecutionStats.cpp
 *
 */

#include "ExecutionStats.h"
#include "Processor/instructions.h"
#include "GC/instructions.h"

#include <iostream>
#include <iomanip>
#include <set>

ExecutionStats& ExecutionStats::operator+=(const ExecutionStats& other)
{
    for (auto it : other)
        (*this)[it.first] += it.second;
    return *this;
}

void ExecutionStats::print()
{
    cerr << "Instruction statistics:" << endl;
    set<pair<size_t, int>> sorted_stats;
    for (auto& x : *this)
    {
        sorted_stats.insert({x.second, x.first});
    }
    for (auto& x : sorted_stats)
    {
        auto opcode = x.second;
        auto calls = x.first;
        cerr << "\t";
        int n_fill = 15;
        switch (opcode)
        {
#define X(NAME, PRE, CODE) case NAME: cerr << #NAME; n_fill -= strlen(#NAME); break;
        ARITHMETIC_INSTRUCTIONS
#undef X
#define X(NAME, CODE) case NAME: cerr << #NAME; n_fill -= strlen(#NAME); break;
        COMBI_INSTRUCTIONS
#undef X
        default:
            cerr << hex << setw(5) << showbase << left << opcode;
            n_fill -= 5;
            cerr << setw(0);
        }
        for (int i = 0; i < n_fill; i++)
            cerr << " ";
        cerr << dec << calls << endl;
    }
}
