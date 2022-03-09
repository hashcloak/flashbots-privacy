import itertools, time
from collections import defaultdict, deque
from Compiler.exceptions import *
from Compiler.config import *
from Compiler.instructions import *
from Compiler.instructions_base import *
from Compiler.util import *
import Compiler.graph
import Compiler.program
import heapq, itertools
import operator
import sys
from functools import reduce

class BlockAllocator:
    """ Manages freed memory blocks. """
    def __init__(self):
        self.by_logsize = [defaultdict(set) for i in range(32)]
        self.by_address = {}

    def by_size(self, size):
        if size >= 2 ** 32:
            raise CompilerError('size exceeds addressing capability')
        return self.by_logsize[int(math.log(size, 2))][size]

    def push(self, address, size):
        end = address + size
        if end in self.by_address:
            next_size = self.by_address.pop(end)
            self.by_size(next_size).remove(end)
            size += next_size
        self.by_size(size).add(address)
        self.by_address[address] = size

    def pop(self, size):
        if len(self.by_size(size)) > 0:
            block_size = size
        else:
            logsize = int(math.log(size, 2))
            for block_size, addresses in self.by_logsize[logsize].items():
                if block_size >= size and len(addresses) > 0:
                    break
            else:
                done = False
                for x in self.by_logsize[logsize + 1:]:
                    for block_size, addresses in x.items():
                        if len(addresses) > 0:
                            done = True
                            break
                    if done:
                        break
                else:
                    block_size = 0
        if block_size >= size:
            addr = self.by_size(block_size).pop()
            del self.by_address[addr]
            diff = block_size - size
            if diff:
                self.by_size(diff).add(addr + size)
                self.by_address[addr + size] = diff
            return addr

class StraightlineAllocator:
    """Allocate variables in a straightline program using n registers.
    It is based on the precondition that every register is only defined once."""
    def __init__(self, n, program):
        self.alloc = dict_by_id()
        self.usage = Compiler.program.RegType.create_dict(lambda: 0)
        self.defined = dict_by_id()
        self.dealloc = set_by_id()
        self.n = n
        self.program = program

    def alloc_reg(self, reg, free):
        base = reg.vectorbase
        if base in self.alloc:
            # already allocated
            return

        reg_type = reg.reg_type
        size = base.size
        if free[reg_type, size]:
            res = free[reg_type, size].pop()
        else:
            if self.usage[reg_type] < self.n:
                res = self.usage[reg_type]
                self.usage[reg_type] += size
            else:
                raise RegisterOverflowError()
        self.alloc[base] = res

        base.i = self.alloc[base]

        for dup in base.duplicates:
            dup = dup.vectorbase
            self.alloc[dup] = self.alloc[base]
            dup.i = self.alloc[base]

    def dealloc_reg(self, reg, inst, free):
        if reg.vector:
            self.dealloc |= reg.vector
        else:
            self.dealloc.add(reg)
        base = reg.vectorbase

        seen = set_by_id()
        to_check = set_by_id()
        to_check.add(base)
        while to_check:
            dup = to_check.pop()
            if dup not in seen:
                seen.add(dup)
                base = dup.vectorbase
                if base.vector:
                    for i in base.vector:
                        if i not in self.dealloc:
                            # not all vector elements ready for deallocation
                            return
                        if len(i.duplicates) > 1:
                            for x in i.duplicates:
                                to_check.add(x)
                else:
                    if base not in self.dealloc:
                        return
                for x in itertools.chain(dup.duplicates, base.duplicates):
                    to_check.add(x)

        free[reg.reg_type, base.size].append(self.alloc[base])
        if inst.is_vec() and base.vector:
            self.defined[base] = inst
            for i in base.vector:
                self.defined[i] = inst
        else:
            self.defined[reg] = inst

    def process(self, program, alloc_pool):
        for k,i in enumerate(reversed(program)):
            unused_regs = []
            for j in i.get_def():
                if j.vectorbase in self.alloc:
                    if j in self.defined:
                        raise CompilerError("Double write on register %s " \
                                            "assigned by '%s' in %s" % \
                                                (j,i,format_trace(i.caller)))
                else:
                    # unused register
                    self.alloc_reg(j, alloc_pool)
                    unused_regs.append(j)
            if unused_regs and len(unused_regs) == len(list(i.get_def())) and \
               self.program.verbose:
                # only report if all assigned registers are unused
                print("Register(s) %s never used, assigned by '%s' in %s" % \
                    (unused_regs,i,format_trace(i.caller)))

            for j in i.get_used():
                self.alloc_reg(j, alloc_pool)
            for j in i.get_def():
                self.dealloc_reg(j, i, alloc_pool)

            if k % 1000000 == 0 and k > 0:
                print("Allocated registers for %d instructions at" % k, time.asctime())

        # print "Successfully allocated registers"
        # print "modp usage: %d clear, %d secret" % \
        #     (self.usage[Compiler.program.RegType.ClearModp], self.usage[Compiler.program.RegType.SecretModp])
        # print "GF2N usage: %d clear, %d secret" % \
        #     (self.usage[Compiler.program.RegType.ClearGF2N], self.usage[Compiler.program.RegType.SecretGF2N])
        return self.usage

    def finalize(self, options):
        for reg in self.alloc:
            for x in reg.get_all():
                if x not in self.dealloc and reg not in self.dealloc \
                   and len(x.duplicates) == 1:
                    print('Warning: read before write at register', x)
                    print('\tregister trace: %s' % format_trace(x.caller,
                                                                '\t\t'))
                    if options.stop:
                        sys.exit(1)

def determine_scope(block, options):
    last_def = defaultdict_by_id(lambda: -1)
    used_from_scope = set_by_id()

    def read(reg, n):
        for dup in reg.duplicates:
            if last_def[dup] == -1:
                dup.can_eliminate = False
                used_from_scope.add(dup)

    def write(reg, n):
        if last_def[reg] != -1:
            print('Warning: double write at register', reg)
            print('\tline %d: %s' % (n, instr))
            print('\ttrace: %s' % format_trace(instr.caller, '\t\t'))
            if options.stop:
                sys.exit(1)
        last_def[reg] = n

    for n,instr in enumerate(block.instructions):
        outputs,inputs = instr.get_def(), instr.get_used()
        for reg in inputs:
            if reg.vector and instr.is_vec():
                for i in reg.vector:
                    read(i, n)
            else:
                read(reg, n)
        for reg in outputs:
            if reg.vector and instr.is_vec():
                for i in reg.vector:
                    write(i, n)
            else:
                write(reg, n)

    block.used_from_scope = used_from_scope

class Merger:
    def __init__(self, block, options, merge_classes):
        self.block = block
        self.instructions = block.instructions
        self.options = options
        if options.max_parallel_open:
            self.max_parallel_open = int(options.max_parallel_open)
        else:
            self.max_parallel_open = float('inf')
        self.counter = defaultdict(lambda: 0)
        self.rounds = defaultdict(lambda: 0)
        self.dependency_graph(merge_classes)

    def do_merge(self, merges_iter):
        """ Merge an iterable of nodes in G, returning the number of merged
        instructions and the index of the merged instruction. """
        # sort merges, necessary for inputb
        merge = list(merges_iter)
        merge.sort()
        merges_iter = iter(merge)
        instructions = self.instructions
        mergecount = 0
        try:
            n = next(merges_iter)
        except StopIteration:
            return mergecount, None

        for i in merges_iter:
            instructions[n].merge(instructions[i])
            instructions[i] = None
            self.merge_nodes(n, i)
            mergecount += 1

        return mergecount, n

    def longest_paths_merge(self):
        """ Attempt to merge instructions of type instruction_type (which are given in
        merge_nodes) using longest paths algorithm.

        Returns the no. of rounds of communication required after merging (assuming 1 round/instruction).

        Doesn't use networkx.
        """
        G = self.G
        instructions = self.instructions
        merge_nodes = self.open_nodes
        depths = self.depths
        if not merge_nodes:
            return 0

        # merge opens at same depth
        merges = defaultdict(list)
        for node in merge_nodes:
            merges[depths[node]].append(node)

        # after merging, the first element in merges[i] remains for each depth i,
        # all others are removed from instructions and G
        last_nodes = [None, None]
        for i in sorted(merges):
            merge = merges[i]
            t = type(self.instructions[merge[0]])
            self.counter[t] += len(merge)
            self.rounds[t] += 1
            if len(merge) > 10000:
                print('Merging %d %s in round %d/%d' % \
                    (len(merge), t.__name__, i, len(merges)))
            self.do_merge(merge)

        preorder = None

        if len(instructions) > 1000000:
            print("Topological sort ...")
        order = Compiler.graph.topological_sort(G, preorder)
        instructions[:] = [instructions[i] for i in order if instructions[i] is not None]
        if len(instructions) > 1000000:
            print("Done at", time.asctime())

        return len(merges)

    def dependency_graph(self, merge_classes):
        """ Create the program dependency graph. """
        block = self.block
        options = self.options
        open_nodes = set()
        self.open_nodes = open_nodes
        colordict = defaultdict(lambda: 'gray', asm_open='red',\
                                ldi='lightblue', ldm='lightblue', stm='blue',\
                                mov='yellow', mulm='orange', mulc='orange',\
                                triple='green', square='green', bit='green',\
                                asm_input='lightgreen')

        G = Compiler.graph.SparseDiGraph(len(block.instructions))
        self.G = G

        reg_nodes = {}
        last_def = defaultdict_by_id(lambda: -1)
        last_mem_write = []
        last_mem_read = []
        warned_about_mem = []
        last_mem_write_of = defaultdict(list)
        last_mem_read_of = defaultdict(list)
        last_print_str = None
        last = defaultdict(lambda: defaultdict(lambda: None))
        last_open = deque()
        last_input = defaultdict(lambda: [None, None])

        depths = [0] * len(block.instructions)
        self.depths = depths
        parallel_open = defaultdict(lambda: 0)
        next_available_depth = {}
        self.sources = []
        self.real_depths = [0] * len(block.instructions)
        round_type = {}

        def add_edge(i, j):
            G.add_edge(i, j)
            for d in (self.depths, self.real_depths):
                if d[j] < d[i]:
                    d[j] = d[i]

        def read(reg, n):
            for dup in reg.duplicates:
                if last_def[dup] != -1:
                    add_edge(last_def[dup], n)

        def write(reg, n):
            last_def[reg] = n

        def handle_mem_access(addr, reg_type, last_access_this_kind,
                              last_access_other_kind):
            this = last_access_this_kind[str(addr),reg_type]
            other = last_access_other_kind[str(addr),reg_type]
            if this and other:
                if this[-1] < other[0]:
                    del this[:]
            this.append(n)
            for inst in other:
                add_edge(inst, n)

        def mem_access(n, instr, last_access_this_kind, last_access_other_kind):
            addr = instr.args[1]
            reg_type = instr.args[0].reg_type
            if isinstance(addr, int):
                for i in range(min(instr.get_size(), 100)):
                    addr_i = addr + i
                    handle_mem_access(addr_i, reg_type, last_access_this_kind,
                                      last_access_other_kind)
                if block.warn_about_mem and not warned_about_mem and \
                   (instr.get_size() > 100):
                    print('WARNING: Order of memory instructions ' \
                        'not preserved due to long vector, errors possible')
                    warned_about_mem.append(True)
            else:
                handle_mem_access(addr, reg_type, last_access_this_kind,
                                  last_access_other_kind)
            if block.warn_about_mem and not warned_about_mem and \
               not isinstance(instr, DirectMemoryInstruction):
                print('WARNING: Order of memory instructions ' \
                    'not preserved, errors possible')
                # hack
                warned_about_mem.append(True)

        def strict_mem_access(n, last_this_kind, last_other_kind):
            if last_other_kind and last_this_kind and \
               last_other_kind[-1] > last_this_kind[-1]:
                last_this_kind[:] = []
            last_this_kind.append(n)
            for i in last_other_kind:
                add_edge(i, n)

        def keep_order(instr, n, t, arg_index=None):
            if arg_index is None:
                player = None
            else:
                player = instr.args[arg_index]
            if last[t][player] is not None:
                add_edge(last[t][player], n)
            last[t][player] = n

        def keep_merged_order(instr, n, t):
            if last_input[t][0] is not None:
                if instr.merge_id() != \
                   block.instructions[last_input[t][0]].merge_id():
                    add_edge(last_input[t][0], n)
                    last_input[t][1] = last_input[t][0]
                elif last_input[t][1] is not None:
                    add_edge(last_input[t][1], n)
            last_input[t][0] = n

        for n,instr in enumerate(block.instructions):
            outputs,inputs = instr.get_def(), instr.get_used()

            G.add_node(n)

            # if options.debug:
            #     col = colordict[instr.__class__.__name__]
            #     G.add_node(n, color=col, label=str(instr))
            for reg in inputs:
                if reg.vector and instr.is_vec():
                    for i in reg.vector:
                        read(i, n)
                else:
                    read(reg, n)

            for reg in outputs:
                if reg.vector and instr.is_vec():
                    for i in reg.vector:
                        write(i, n)
                else:
                    write(reg, n)

            # will be merged
            if isinstance(instr, TextInputInstruction):
                keep_merged_order(instr, n, TextInputInstruction)
            elif isinstance(instr, RawInputInstruction):
                keep_merged_order(instr, n, RawInputInstruction)

            if isinstance(instr, merge_classes):
                open_nodes.add(n)
                G.add_node(n, merges=[])
                # the following must happen after adding the edge
                self.real_depths[n] += 1
                depth = depths[n] + 1

                # find first depth that has the right type and isn't full
                skipped_depths = set()
                while (depth in round_type and \
                       round_type[depth] != instr.merge_id()) or \
                      (int(options.max_parallel_open) > 0 and \
                      parallel_open[depth] >= int(options.max_parallel_open)):
                    skipped_depths.add(depth)
                    depth = next_available_depth.get((type(instr), depth), \
                                                     depth + 1)
                for d in skipped_depths:
                    next_available_depth[type(instr), d] = depth

                round_type[depth] = instr.merge_id()
                if int(options.max_parallel_open) > 0:
                    parallel_open[depth] += len(instr.args) * instr.get_size()
                depths[n] = depth

            if isinstance(instr, ReadMemoryInstruction):
                if options.preserve_mem_order:
                    strict_mem_access(n, last_mem_read, last_mem_write)
                else:
                    mem_access(n, instr, last_mem_read_of, last_mem_write_of)
            elif isinstance(instr, WriteMemoryInstruction):
                if options.preserve_mem_order:
                    strict_mem_access(n, last_mem_write, last_mem_read)
                else:
                    mem_access(n, instr, last_mem_write_of, last_mem_read_of)
            elif isinstance(instr, matmulsm):
                if options.preserve_mem_order:
                    strict_mem_access(n, last_mem_read, last_mem_write)
                else:
                    for i in last_mem_write_of.values():
                        for j in i:
                            add_edge(j, n)
            # keep I/O instructions in order
            elif isinstance(instr, IOInstruction):
                if last_print_str is not None:
                    add_edge(last_print_str, n)
                last_print_str = n
            elif isinstance(instr, PublicFileIOInstruction):
                keep_order(instr, n, instr.__class__)
            elif isinstance(instr, startprivateoutput_class):
                keep_order(instr, n, startprivateoutput_class, 2)
            elif isinstance(instr, stopprivateoutput_class):
                keep_order(instr, n, stopprivateoutput_class, 2)
            elif isinstance(instr, prep_class):
                keep_order(instr, n, instr.args[0])
            elif isinstance(instr, StackInstruction):
                keep_order(instr, n, StackInstruction)

            if not G.pred[n]:
                self.sources.append(n)

            if n % 1000000 == 0 and n > 0:
                print("Processed dependency of %d/%d instructions at" % \
                    (n, len(block.instructions)), time.asctime())

    def merge_nodes(self, i, j):
        """ Merge node j into i, removing node j """
        G = self.G
        if j in G[i]:
            G.remove_edge(i, j)
        if i in G[j]:
            G.remove_edge(j, i)
        G.add_edges_from(list(zip(itertools.cycle([i]), G[j], [G.weights[(j,k)] for k in G[j]])))
        G.add_edges_from(list(zip(G.pred[j], itertools.cycle([i]), [G.weights[(k,j)] for k in G.pred[j]])))
        G.get_attr(i, 'merges').append(j)
        G.remove_node(j)

    def eliminate_dead_code(self):
        instructions = self.instructions
        G = self.G
        merge_nodes = self.open_nodes
        count = 0
        open_count = 0
        stats = defaultdict(lambda: 0)
        for i,inst in zip(range(len(instructions) - 1, -1, -1), reversed(instructions)):
            if inst is None:
                continue
            can_eliminate_defs = True
            for reg in inst.get_def():
                for dup in reg.duplicates:
                    if not dup.can_eliminate:
                        can_eliminate_defs = False
                        break
            # remove if instruction has result that isn't used
            unused_result = not G.degree(i) and len(list(inst.get_def())) \
                and can_eliminate_defs \
                and not isinstance(inst, (DoNotEliminateInstruction))
            def eliminate(i):
                G.remove_node(i)
                merge_nodes.discard(i)
                stats[type(instructions[i]).__name__] += 1
                instructions[i] = None
            if unused_result:
                eliminate(i)
                count += 1
            # remove unnecessary stack instructions
            # left by optimization with budget
            if isinstance(inst, popint_class) and \
               (not G.degree(i) or (G.degree(i) == 1 and
                isinstance(instructions[list(G[i])[0]], StackInstruction))) \
                and \
               inst.args[0].can_eliminate and \
               len(G.pred[i]) == 1 and \
               isinstance(instructions[list(G.pred[i])[0]], pushint_class):
                eliminate(list(G.pred[i])[0])
                eliminate(i)
                count += 2
        if count > 0 and self.block.parent.program.verbose:
            print('Eliminated %d dead instructions, among which %d opens: %s' \
                % (count, open_count, dict(stats)))

    def print_graph(self, filename):
        f = open(filename, 'w')
        print('digraph G {', file=f)
        for i in range(self.G.n):
            for j in self.G[i]:
                print('"%d: %s" -> "%d: %s";' % \
                    (i, self.instructions[i], j, self.instructions[j]), file=f)
        print('}', file=f)
        f.close()

    def print_depth(self, filename):
        f = open(filename, 'w')
        for i in range(self.G.n):
            print('%d: %s' % (self.depths[i], self.instructions[i]), file=f)
        f.close()

class RegintOptimizer:
    def __init__(self):
        self.cache = util.dict_by_id()

    def run(self, instructions):
        for i, inst in enumerate(instructions):
            if isinstance(inst, ldint_class):
                self.cache[inst.args[0]] = inst.args[1]
            elif isinstance(inst, IntegerInstruction):
                if inst.args[1] in self.cache and inst.args[2] in self.cache:
                    res = inst.op(self.cache[inst.args[1]],
                                  self.cache[inst.args[2]])
                    if abs(res) < 2 ** 31:
                        self.cache[inst.args[0]] = res
                        instructions[i] = ldint(inst.args[0], res,
                                                add_to_prog=False)
                elif isinstance(inst, addint_class):
                    if inst.args[1] in self.cache and \
                       self.cache[inst.args[1]] == 0:
                        instructions[i] = inst.args[0].link(inst.args[2])
                    elif inst.args[2] in self.cache and \
                       self.cache[inst.args[2]] == 0:
                        instructions[i] = inst.args[0].link(inst.args[1])
            elif isinstance(inst, IndirectMemoryInstruction):
                if inst.args[1] in self.cache:
                    instructions[i] = inst.get_direct(self.cache[inst.args[1]])
            elif type(inst) == convint_class:
                if inst.args[1] in self.cache:
                    res = self.cache[inst.args[1]]
                    self.cache[inst.args[0]] = res
                    if abs(res) < 2 ** 31:
                        instructions[i] = ldi(inst.args[0], res,
                                              add_to_prog=False)
            elif isinstance(inst, mulm_class):
                if inst.args[2] in self.cache:
                    op = self.cache[inst.args[2]]
                    if op == 0:
                        instructions[i] = ldsi(inst.args[0], 0,
                                               add_to_prog=False)
                    elif op == 1:
                        instructions[i] = None
                        inst.args[0].link(inst.args[1])
        instructions[:] = list(filter(lambda x: x is not None, instructions))
