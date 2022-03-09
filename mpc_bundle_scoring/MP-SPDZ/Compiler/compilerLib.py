from Compiler.program import Program
from .GC import types as GC_types

import sys
import re, tempfile, os


def run(args, options):
    """ Compile a file and output a Program object.
    
    If options.merge_opens is set to True, will attempt to merge any
    parallelisable open instructions. """
    
    prog = Program(args, options)
    VARS['program'] = prog
    if options.binary:
        VARS['sint'] = GC_types.sbitintvec.get_type(int(options.binary))
        VARS['sfix'] = GC_types.sbitfixvec
        for i in 'cint', 'cfix', 'cgf2n', 'sintbit', 'sgf2n', 'sgf2nint', \
            'sgf2nuint', 'sgf2nuint32', 'sgf2nfloat', 'sfloat', 'cfloat', \
            'squant':
            del VARS[i]
    
    print('Compiling file', prog.infile)
    f = open(prog.infile, 'rb')

    changed = False
    if options.flow_optimization:
        output = []
        if_stack = []
        for line in open(prog.infile):
            if if_stack and not re.match(if_stack[-1][0], line):
                if_stack.pop()
            m = re.match(
                '(\s*)for +([a-zA-Z_]+) +in +range\(([0-9a-zA-Z_]+)\):',
                line)
            if m:
                output.append('%s@for_range_opt(%s)\n' % (m.group(1),
                                                          m.group(3)))
                output.append('%sdef _(%s):\n' % (m.group(1), m.group(2)))
                changed = True
                continue
            m = re.match('(\s*)if(\W.*):', line)
            if m:
                if_stack.append((m.group(1), len(output)))
                output.append('%s@if_(%s)\n' % (m.group(1), m.group(2)))
                output.append('%sdef _():\n' % (m.group(1)))
                changed = True
                continue
            m = re.match('(\s*)elif\s+', line)
            if m:
                raise CompilerError('elif not supported')
            if if_stack:
                m = re.match('%selse:' % if_stack[-1][0], line)
                if m:
                    start = if_stack[-1][1]
                    ws = if_stack[-1][0]
                    output[start] = re.sub(r'^%s@if_\(' % ws, r'%s@if_e(' % ws,
                                           output[start])
                    output.append('%s@else_\n' % ws)
                    output.append('%sdef _():\n' % ws)
                    continue
            output.append(line)
        if changed:
            infile = tempfile.NamedTemporaryFile('w+', delete=False)
            for line in output:
                infile.write(line)
            infile.seek(0)
        else:
            infile = open(prog.infile)
    else:
        infile = open(prog.infile)

    # make compiler modules directly accessible
    sys.path.insert(0, 'Compiler')
    # create the tapes
    exec(compile(infile.read(), infile.name, 'exec'), VARS)

    if changed and not options.debug:
        os.unlink(infile.name)

    prog.finalize()

    if prog.req_num:
        print('Program requires:')
        for x in prog.req_num.pretty():
            print(x)

    if prog.verbose:
        print('Program requires:', repr(prog.req_num))
        print('Cost:', 0 if prog.req_num is None else prog.req_num.cost())
        print('Memory size:', dict(prog.allocated_mem))

    return prog
