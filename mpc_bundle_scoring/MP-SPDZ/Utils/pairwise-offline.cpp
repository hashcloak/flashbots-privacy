#include "FHEOffline/PairwiseMachine.h"
#include "Tools/callgrind.h"

int main(int argc, const char** argv)
{
    CALLGRIND_STOP_INSTRUMENTATION;
    PairwiseMachine machine(argc, argv);
    CALLGRIND_START_INSTRUMENTATION;
    machine.run();
}
