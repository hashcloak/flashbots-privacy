#!/usr/bin/env python3


#     ===== Compiler usage instructions =====
#
# ./compile.py input_file
#
# will compile Programs/Source/input_file.mpc onto
# Programs/Bytecode/input_file.bc and Programs/Schedules/input_file.sch
#
# (run with --help for more options)
#
# See the compiler documentation at https://mp-spdz.readthedocs.io
# for details on the Compiler package


from optparse import OptionParser
from Compiler.program import defaults
import Compiler

def main():
    usage = "usage: %prog [options] filename [args]"
    parser = OptionParser(usage=usage)
    parser.add_option("-n", "--nomerge",
                      action="store_false", dest="merge_opens",
                      default=defaults.merge_opens,
                      help="don't attempt to merge open instructions")
    parser.add_option("-o", "--output", dest="outfile",
                      help="specify output file")
    parser.add_option("-a", "--asm-output", dest="asmoutfile",
                      help="asm output file for debugging")
    parser.add_option("-g", "--galoissize", dest="galois",
                      default=defaults.galois,
                      help="bit length of Galois field")
    parser.add_option("-d", "--debug", action="store_true", dest="debug",
                      help="keep track of trace for debugging")
    parser.add_option("-c", "--comparison", dest="comparison", default="log",
                      help="comparison variant: log|plain|inv|sinv")
    parser.add_option("-M", "--preserve-mem-order", action="store_true",
                      dest="preserve_mem_order",
                      default=defaults.preserve_mem_order,
                      help="preserve order of memory instructions; possible efficiency loss")
    parser.add_option("-O", "--optimize-hard", action="store_true",
                      dest="optimize_hard", help="currently not in use")
    parser.add_option("-u", "--noreallocate", action="store_true", dest="noreallocate",
                      default=defaults.noreallocate, help="don't reallocate")
    parser.add_option("-m", "--max-parallel-open", dest="max_parallel_open",
                      default=defaults.max_parallel_open,
                      help="restrict number of parallel opens")
    parser.add_option("-D", "--dead-code-elimination", action="store_true",
                      dest="dead_code_elimination",
                      default=defaults.dead_code_elimination,
                      help="eliminate instructions with unused result")
    parser.add_option("-p", "--profile", action="store_true", dest="profile",
                      help="profile compilation")
    parser.add_option("-s", "--stop", action="store_true", dest="stop",
                      help="stop on register errors")
    parser.add_option("-R", "--ring", dest="ring", default=defaults.ring,
                      help="bit length of ring (default: 0 for field)")
    parser.add_option("-B", "--binary", dest="binary", default=defaults.binary,
                      help="bit length of sint in binary circuit (default: 0 for arithmetic)")
    parser.add_option("-F", "--field", dest="field", default=defaults.field,
                      help="bit length of sint modulo prime (default: 64)")
    parser.add_option("-P", "--prime", dest="prime", default=defaults.prime,
                      help="prime modulus (default: not specified)")
    parser.add_option("-I", "--insecure", action="store_true", dest="insecure",
                      help="activate insecure functionality for benchmarking")
    parser.add_option("-b", "--budget", dest="budget", default=defaults.budget,
                      help="set budget for optimized loop unrolling "
                      "(default: 100000)")
    parser.add_option("-X", "--mixed", action="store_true", dest="mixed",
                      help="mixing arithmetic and binary computation")
    parser.add_option("-Y", "--edabit", action="store_true", dest="edabit",
                      help="mixing arithmetic and binary computation using edaBits")
    parser.add_option("-Z", "--split", default=defaults.split, dest="split",
                      help="mixing arithmetic and binary computation "
                      "using direct conversion if supported "
                      "(number of parties as argument)")
    parser.add_option("-C", "--CISC", action="store_true", dest="cisc",
                      help="faster CISC compilation mode")
    parser.add_option("-K", "--keep-cisc", dest="keep_cisc",
                      help="don't translate CISC instructions")
    parser.add_option("-l", "--flow-optimization", action="store_true",
                      dest="flow_optimization", help="optimize control flow")
    parser.add_option("-v", "--verbose", action="store_true", dest="verbose",
                      help="more verbose output")
    options,args = parser.parse_args()
    if len(args) < 1:
        parser.print_help()
        return

    if options.optimize_hard:
        print('Note that -O/--optimize-hard currently has no effect')

    def compilation():
        prog = Compiler.run(args, options)

        if prog.public_input_file is not None:
            print('WARNING: %s is required to run the program' % \
                  prog.public_input_file.name)

    if options.profile:
        import cProfile
        p = cProfile.Profile().runctx('compilation()', globals(), locals())
        p.dump_stats(args[0] + '.prof')
        p.print_stats(2)
    else:
        compilation()

if __name__ == '__main__':
    main()
