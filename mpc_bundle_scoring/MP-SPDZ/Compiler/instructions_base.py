import itertools
from random import randint
import time
import inspect
import functools
import copy
from Compiler.exceptions import *
from Compiler.config import *
from Compiler import util
from Compiler import tools
from Compiler import program


###
### Opcode constants
###
### Whenever these are changed the corresponding enums in Processor/instruction.h
### MUST also be changed. (+ the documentation)
###
opcodes = dict(
    # Emulation
    CISC = 0,
    # Load/store
    LDI = 0x1,
    LDSI = 0x2,
    LDMC = 0x3,
    LDMS = 0x4,
    STMC = 0x5,
    STMS = 0x6,
    LDMCI = 0x7,
    LDMSI = 0x8,
    STMCI = 0x9,
    STMSI = 0xA,
    MOVC = 0xB,
    MOVS = 0xC,
    PROTECTMEMS = 0xD,
    PROTECTMEMC = 0xE,
    PROTECTMEMINT = 0xF,
    LDMINT = 0xCA,
    STMINT = 0xCB,
    LDMINTI = 0xCC,
    STMINTI = 0xCD,
    PUSHINT = 0xCE,
    POPINT = 0xCF,
    MOVINT = 0xD0,
    # Machine
    LDTN = 0x10,
    LDARG = 0x11,
    REQBL = 0x12,
    STARG = 0x13,
    TIME = 0x14,
    START = 0x15,
    STOP = 0x16,
    USE = 0x17,
    USE_INP = 0x18,
    RUN_TAPE = 0x19,
    JOIN_TAPE = 0x1A,
    CRASH = 0x1B,
    USE_PREP = 0x1C,
    STARTGRIND = 0x1D,
    STOPGRIND = 0x1E,
    NPLAYERS = 0xE2,
    THRESHOLD = 0xE3,
    PLAYERID = 0xE4,
    USE_EDABIT = 0xE5,
    USE_MATMUL = 0x1F,
    # Addition
    ADDC = 0x20,
    ADDS = 0x21,
    ADDM = 0x22,
    ADDCI = 0x23,
    ADDSI = 0x24,
    SUBC = 0x25,
    SUBS = 0x26,
    SUBML = 0x27,
    SUBMR = 0x28,
    SUBCI = 0x29,
    SUBSI = 0x2A,
    SUBCFI = 0x2B,
    SUBSFI = 0x2C,
    # Multiplication/division
    MULC = 0x30,
    MULM = 0x31,
    MULCI = 0x32,
    MULSI = 0x33,
    DIVC = 0x34,
    DIVCI = 0x35,
    MODC = 0x36,
    MODCI = 0x37,
    LEGENDREC = 0x38,
    DIGESTC = 0x39,
    INV2M = 0x3a,
    FLOORDIVC = 0x3b,
    GMULBITC = 0x136,
    GMULBITM = 0x137,
    # Open
    OPEN = 0xA5,
    MULS = 0xA6,
    MULRS = 0xA7,
    DOTPRODS = 0xA8,
    TRUNC_PR = 0xA9,
    MATMULS = 0xAA,
    MATMULSM = 0xAB,
    CONV2DS = 0xAC,
    CHECK = 0xAF,
    # Data access
    TRIPLE = 0x50,
    BIT = 0x51,
    SQUARE = 0x52,
    INV = 0x53,
    GBITTRIPLE = 0x154,
    GBITGF2NTRIPLE = 0x155,
    INPUTMASK = 0x56,
    INPUTMASKREG = 0x5C,
    PREP = 0x57,
    DABIT = 0x58,
    EDABIT = 0x59,
    SEDABIT = 0x5A,
    RANDOMS = 0x5B,
    RANDOMFULLS = 0x5D,
    # Input
    INPUT = 0x60,
    INPUTFIX = 0xF0,
    INPUTFLOAT = 0xF1,
    INPUTMIXED = 0xF2,
    INPUTMIXEDREG = 0xF3,
    RAWINPUT = 0xF4,
    INPUTPERSONAL = 0xF5,
    STARTINPUT = 0x61,
    STOPINPUT = 0x62,  
    READSOCKETC = 0x63,
    READSOCKETS = 0x64,
    WRITESOCKETC = 0x65,
    WRITESOCKETS = 0x66,
    READSOCKETINT = 0x69,
    WRITESOCKETINT = 0x6a,
    WRITESOCKETSHARE = 0x6b,
    LISTEN = 0x6c,
    ACCEPTCLIENTCONNECTION = 0x6d,
    CLOSECLIENTCONNECTION = 0x6e,
    READCLIENTPUBLICKEY = 0x6f,
    # Bitwise logic
    ANDC = 0x70,
    XORC = 0x71,
    ORC = 0x72,
    ANDCI = 0x73,
    XORCI = 0x74,
    ORCI = 0x75,
    NOTC = 0x76,
    # Bitwise shifts
    SHLC = 0x80,
    SHRC = 0x81,
    SHLCI = 0x82,
    SHRCI = 0x83,
    SHRSI = 0x84,
    # Branching and comparison
    JMP = 0x90,
    JMPNZ = 0x91,
    JMPEQZ = 0x92,
    EQZC = 0x93,
    LTZC = 0x94,
    LTC = 0x95,
    GTC = 0x96,
    EQC = 0x97,
    JMPI = 0x98,
    # Integers
    BITDECINT = 0x99,
    LDINT = 0x9A,
    ADDINT = 0x9B,
    SUBINT = 0x9C,
    MULINT = 0x9D,
    DIVINT = 0x9E,
    PRINTINT = 0x9F,
    INCINT = 0xD1,
    SHUFFLE = 0xD2,
    # Conversion
    CONVINT = 0xC0,
    CONVMODP = 0xC1,
    GCONVGF2N = 0x1C1,
    # IO
    PRINTMEM = 0xB0,
    PRINTREG = 0XB1,
    RAND = 0xB2,
    PRINTREGPLAIN = 0xB3,
    PRINTCHR = 0xB4,
    PRINTSTR = 0xB5,
    PUBINPUT = 0xB6,
    RAWOUTPUT = 0xB7,
    STARTPRIVATEOUTPUT = 0xB8,
    STOPPRIVATEOUTPUT = 0xB9,
    PRINTCHRINT = 0xBA,
    PRINTSTRINT = 0xBB,
    PRINTFLOATPLAIN = 0xBC,
    WRITEFILESHARE = 0xBD,     
    READFILESHARE = 0xBE,
    CONDPRINTSTR = 0xBF,
    PRINTFLOATPREC = 0xE0,
    CONDPRINTPLAIN = 0xE1,
    INTOUTPUT = 0xE6,
    FLOATOUTPUT = 0xE7,
    GBITDEC = 0x184,
    GBITCOM = 0x185,
    # Secure socket
    INITSECURESOCKET = 0x1BA,
    RESPSECURESOCKET = 0x1BB
)


def int_to_bytes(x):
    """ 32 bit int to big-endian 4 byte conversion. """
    assert(x < 2**32 and x >= -2**32)
    return [(x >> 8*i) % 256 for i in (3,2,1,0)]


global_vector_size_stack = []
global_instruction_type_stack = ['modp']

def set_global_vector_size(size):
    stack = global_vector_size_stack
    if size == 1 and not stack:
        return
    stack.append(size)

def set_global_instruction_type(t):
    if t == 'modp' or t == 'gf2n':
        global_instruction_type_stack.append(t)
    else:
        raise CompilerError('Invalid type %s for setting global instruction type')

def reset_global_vector_size():
    stack = global_vector_size_stack
    if global_vector_size_stack:
        stack.pop()

def reset_global_instruction_type():
    global_instruction_type_stack.pop()

def get_global_vector_size():
    stack = global_vector_size_stack
    if stack:
        return stack[-1]
    else:
        return 1

def get_global_instruction_type():
    return global_instruction_type_stack[-1]


def vectorize(instruction, global_dict=None):
    """ Decorator to vectorize instructions. """

    if global_dict is None:
        global_dict = inspect.getmodule(instruction).__dict__

    class Vectorized_Instruction(instruction):
        __slots__ = ['size']
        def __init__(self, size, *args, **kwargs):
            self.size = size
            super(Vectorized_Instruction, self).__init__(*args, **kwargs)
            if not kwargs.get('copying', False):
                for arg,f in zip(self.args, self.arg_format):
                    if issubclass(ArgFormats[f], RegisterArgFormat):
                        arg.set_size(size)
        def get_code(self):
            return instruction.get_code(self, self.get_size())
        def get_pre_arg(self):
            try:
                return "%d, " % self.size
            except:
                return "{undef}, "
        def is_vec(self):
            return True
        def get_size(self):
            return self.size
        def expand(self):
            set_global_vector_size(self.size)
            super(Vectorized_Instruction, self).expand()
            reset_global_vector_size()
        def copy(self, size, subs):
            return type(self)(size, *self.get_new_args(size, subs),
                              copying=True)

    @functools.wraps(instruction)
    def maybe_vectorized_instruction(*args, **kwargs):
        size = get_global_vector_size()
        for arg in args:
            try:
                size = arg.size
                break
            except:
                pass
        if size == 1:
            return instruction(*args, **kwargs)
        else:
            return Vectorized_Instruction(size, *args, **kwargs)
    maybe_vectorized_instruction.vec_ins = Vectorized_Instruction
    maybe_vectorized_instruction.std_ins = instruction
    
    vectorized_name = 'v' + instruction.__name__
    Vectorized_Instruction.__name__ = vectorized_name
    global_dict[vectorized_name] = Vectorized_Instruction
    global_dict[instruction.__name__ + '_class'] = instruction
    instruction.__doc__ = ''
    # exclude GF(2^n) instructions from documentation
    if instruction.code and instruction.code >> 8 == 1:
        maybe_vectorized_instruction.__doc__ = ''
    return maybe_vectorized_instruction


def gf2n(instruction):
    """ Decorator to create GF_2^n instruction corresponding to a given
        modp instruction.

        Adds the new GF_2^n instruction to the globals dictionary. Also adds a
        vectorized GF_2^n instruction if a modp version exists. """
    global_dict = inspect.getmodule(instruction).__dict__

    if 'v' + instruction.__name__ in global_dict:
        vectorized = True
    else:
        vectorized = False

    if isinstance(instruction, type) and issubclass(instruction, Instruction):
        instruction_cls = instruction
    else:
        try:
            instruction_cls = global_dict[instruction.__name__ + '_class']
        except KeyError:
            raise CompilerError('Cannot decorate instruction %s' % instruction)

    def reformat(arg_format):
        if isinstance(arg_format, list):
            __format = []
            for __f in arg_format:
                if __f in ('int', 'p', 'ci', 'str'):
                    __format.append(__f)
                else:
                    __format.append(__f[0] + 'g' + __f[1:])
            arg_format[:] = __format
        elif isinstance(arg_format, property):
            pass
        else:
            for __f in arg_format.args:
                reformat(__f)

    class GF2N_Instruction(instruction_cls):
        __doc__ = instruction_cls.__doc__.replace('c_', 'c^g_').replace('s_', 's^g_')
        __slots__ = []
        field_type = 'gf2n'
        if isinstance(instruction_cls.code, int):
            code = (1 << 8) + instruction_cls.code

        # set modp registers in arg_format to GF2N registers
        if 'gf2n_arg_format' in instruction_cls.__dict__:
            arg_format = instruction_cls.gf2n_arg_format
        elif isinstance(instruction_cls.arg_format, itertools.repeat):
            __f = next(instruction_cls.arg_format)
            if __f != 'int' and __f != 'p':
                arg_format = itertools.repeat(__f[0] + 'g' + __f[1:])
        else:
            arg_format = copy.deepcopy(instruction_cls.arg_format)
            reformat(arg_format)

        def is_gf2n(self):
            return True

        def expand(self):
            set_global_instruction_type('gf2n')
            super(GF2N_Instruction, self).expand()
            reset_global_instruction_type()

    GF2N_Instruction.__name__ = 'g' + instruction_cls.__name__
    if vectorized:
        vec_GF2N = vectorize(GF2N_Instruction, global_dict)

    @functools.wraps(instruction)
    def maybe_gf2n_instruction(*args, **kwargs):
        if get_global_instruction_type() == 'gf2n':
            if vectorized:
                return vec_GF2N(*args, **kwargs)
            else:
                return GF2N_Instruction(*args, **kwargs)
        else:
            return instruction(*args, **kwargs)
    
    # If instruction is vectorized, new GF2N instruction must also be
    if vectorized:
        global_dict[GF2N_Instruction.__name__] = vec_GF2N
    else:
        global_dict[GF2N_Instruction.__name__] = GF2N_Instruction

    global_dict[instruction.__name__ + '_class'] = instruction_cls
    instruction_cls.__doc__ = ''
    return maybe_gf2n_instruction
    #return instruction

class Mergeable:
    pass

def cisc(function):
    class MergeCISC(Mergeable):
        instructions = {}

        def __init__(self, *args, **kwargs):
            self.args = args
            self.kwargs = kwargs
            self.calls = [(args, kwargs)]
            self.params = []
            self.used = []
            for arg in self.args[1:]:
                if isinstance(arg, program.curr_tape.Register):
                    self.used.append(arg)
                    self.params.append(type(arg))
                else:
                    self.params.append(arg)
            self.function = function
            program.curr_block.instructions.append(self)

        def get_def(self):
            return [call[0][0] for call in self.calls]

        def get_used(self):
            return self.used

        def is_vec(self):
            return True

        def merge_id(self):
            return self.function, tuple(self.params), \
                tuple(sorted(self.kwargs.items()))

        def merge(self, other):
            self.calls += other.calls
            self.used += other.used

        def get_size(self):
            return self.args[0].size

        def new_instructions(self, size, regs):
            if self.merge_id() not in self.instructions:
                from Compiler.program import Tape
                tape = Tape(self.function.__name__, program)
                old_tape = program.curr_tape
                program.curr_tape = tape
                block = tape.BasicBlock(tape, None, None)
                tape.active_basicblock = block
                set_global_vector_size(None)
                args = []
                for arg in self.args:
                    try:
                        args.append(type(arg)(size=None))
                    except:
                        args.append(arg)
                program.options.cisc = False
                self.function(*args, **self.kwargs)
                program.options.cisc = True
                reset_global_vector_size()
                program.curr_tape = old_tape
                for x, bl in tape.req_bit_length.items():
                    old_tape.require_bit_length(bl - 1, x)
                from Compiler.allocator import Merger
                merger = Merger(block, program.options,
                                tuple(program.to_merge))
                args[0].can_eliminate = False
                merger.eliminate_dead_code()
                assert int(program.options.max_parallel_open) == 0, \
                    'merging restriction not compatible with ' \
                    'mergeable CISC instructions'
                n_rounds = merger.longest_paths_merge()
                filtered = filter(lambda x: x is not None, block.instructions)
                self.instructions[self.merge_id()] = list(filtered), args, \
                                                     n_rounds
            template, args, self.n_rounds = self.instructions[self.merge_id()]
            subs = util.dict_by_id()
            for arg, reg in zip(args, regs):
                subs[arg] = reg
            set_global_vector_size(size)
            for inst in template:
                inst.copy(size, subs)
            reset_global_vector_size()

        def expand_merged(self, skip):
            if function.__name__ in skip:
                good = True
                for call in self.calls:
                    if not good:
                        break
                    for arg in call[0]:
                        if isinstance(arg, program.curr_tape.Register) and \
                           not issubclass(type(self.calls[0][0][0]), type(arg)):
                            good = False
                if good:
                    return [self], 0
            tape = program.curr_tape
            block = tape.BasicBlock(tape, None, None)
            tape.active_basicblock = block
            size = sum(call[0][0].size for call in self.calls)
            new_regs = []
            for arg in self.args:
                try:
                    new_regs.append(type(arg)(size=size))
                except:
                    break
            base = 0
            for call in self.calls:
                for new_reg, reg in zip(new_regs[1:], call[0][1:]):
                    set_global_vector_size(reg.size)
                    reg.mov(new_reg.get_vector(base, reg.size), reg)
                    reset_global_vector_size()
                base += reg.size
            self.new_instructions(size, new_regs)
            base = 0
            for call in self.calls:
                reg = call[0][0]
                set_global_vector_size(reg.size)
                reg.mov(reg, new_regs[0].get_vector(base, reg.size))
                reset_global_vector_size()
                base += reg.size
            return block.instructions, self.n_rounds - 1

        def add_usage(self, *args):
            pass

        def get_bytes(self):
            assert len(self.kwargs) < 2
            res = int_to_bytes(opcodes['CISC'])
            res += int_to_bytes(sum(len(x[0]) + 2 for x in self.calls) + 1)
            name = self.function.__name__
            String.check(name)
            res += String.encode(name)
            for call in self.calls:
                call[1].pop('nearest', None)
                assert not call[1]
                res += int_to_bytes(len(call[0]) + 2)
                res += int_to_bytes(call[0][0].size)
                for arg in call[0]:
                    res += self.arg_to_bytes(arg)
            return bytearray(res)

        @classmethod
        def arg_to_bytes(self, arg):
            if arg is None:
                return int_to_bytes(0)
            try:
                return int_to_bytes(arg.i)
            except:
                return int_to_bytes(arg)

        def __str__(self):
            return self.function.__name__ + ' ' + ', '.join(
                str(x) for x in itertools.chain(call[0] for call in self.calls))

    MergeCISC.__name__ = function.__name__

    def wrapper(*args, **kwargs):
        same_sizes = True
        for arg in args:
            try:
                same_sizes &= arg.size == args[0].size
            except:
                pass
        if program.options.cisc and same_sizes:
            return MergeCISC(*args, **kwargs)
        else:
            return function(*args, **kwargs)
    return wrapper

def ret_cisc(function):
    def instruction(res, *args, **kwargs):
        res.mov(res, function(*args, **kwargs))
    instruction.__name__ = function.__name__
    instruction = cisc(instruction)

    def wrapper(*args, **kwargs):
        if not program.options.cisc:
            return function(*args, **kwargs)
        from Compiler import types
        if isinstance(args[0], types._clear):
            res_type = type(args[1])
        else:
            res_type = type(args[0])
        res = res_type(size=args[0].size)
        instruction(res, *args, **kwargs)
        return res
    return wrapper

def sfix_cisc(function):
    from Compiler.types import sfix, sint, cfix, copy_doc
    def instruction(res, arg, k, f, *args):
        assert k is not None
        assert f is not None
        old = sfix.k, sfix.f, cfix.k, cfix.f
        sfix.k, sfix.f, cfix.k, cfix.f = [None] * 4
        res.mov(res, function(sfix._new(arg, k=k, f=f), *args).v)
        sfix.k, sfix.f, cfix.k, cfix.f = old
    instruction.__name__ = function.__name__
    instruction = cisc(instruction)

    def wrapper(*args, **kwargs):
        if isinstance(args[0], sfix):
            for arg in args[1:]:
                assert util.is_constant(arg)
            assert not kwargs
            assert args[0].size == args[0].v.size
            k = args[0].k
            f = args[0].f
            res = sfix._new(sint(size=args[0].size), k=k, f=f)
            instruction(res.v, args[0].v, k, f, *args[1:])
            return res
        else:
            return function(*args, **kwargs)
    copy_doc(wrapper, function)
    return wrapper

class RegType(object):
    """ enum-like static class for Register types """
    ClearModp = 'c'
    SecretModp = 's'
    ClearGF2N = 'cg'
    SecretGF2N = 'sg'
    ClearInt = 'ci'

    Types = [ClearModp, SecretModp, ClearGF2N, SecretGF2N, ClearInt]

    @staticmethod
    def create_dict(init_value_fn):
        """ Create a dictionary with all the RegTypes as keys """
        res = defaultdict(init_value_fn)
        # initialization for legacy
        for t in RegType.Types:
            res[t]
        return res

class ArgFormat(object):
    is_reg = False

    @classmethod
    def check(cls, arg):
        return NotImplemented

    @classmethod
    def encode(cls, arg):
        return NotImplemented

class RegisterArgFormat(ArgFormat):
    is_reg = True

    @classmethod
    def check(cls, arg):
        if not isinstance(arg, program.curr_tape.Register):
            raise ArgumentError(arg, 'Invalid register argument')
        if arg.program != program.curr_tape:
            raise ArgumentError(arg, 'Register from other tape, trace: %s' % \
                                    util.format_trace(arg.caller))
        if arg.reg_type != cls.reg_type:
            raise ArgumentError(arg, "Wrong register type '%s', expected '%s'" % \
                                    (arg.reg_type, cls.reg_type))

    @classmethod
    def encode(cls, arg):
        assert arg.i >= 0
        return int_to_bytes(arg.i)

class ClearModpAF(RegisterArgFormat):
    reg_type = RegType.ClearModp

class SecretModpAF(RegisterArgFormat):
    reg_type = RegType.SecretModp

class ClearGF2NAF(RegisterArgFormat):
    reg_type = RegType.ClearGF2N

class SecretGF2NAF(RegisterArgFormat):
    reg_type = RegType.SecretGF2N

class ClearIntAF(RegisterArgFormat):
    reg_type = RegType.ClearInt

class IntArgFormat(ArgFormat):
    @classmethod
    def check(cls, arg):
        if not isinstance(arg, int) and not arg is None:
            raise ArgumentError(arg, 'Expected an integer-valued argument')

    @classmethod
    def encode(cls, arg):
        return int_to_bytes(arg)

class ImmediateModpAF(IntArgFormat):
    @classmethod
    def check(cls, arg):
        super(ImmediateModpAF, cls).check(arg)
        if arg >= 2**32 or arg < -2**32:
            raise ArgumentError(arg, 'Immediate value outside of 32-bit range')

class ImmediateGF2NAF(IntArgFormat):
    @classmethod
    def check(cls, arg):
        # bounds checking for GF(2^n)???
        super(ImmediateGF2NAF, cls).check(arg)

class PlayerNoAF(IntArgFormat):
    @classmethod
    def check(cls, arg):
        super(PlayerNoAF, cls).check(arg)
        if arg > 256:
            raise ArgumentError(arg, 'Player number > 256')

class String(ArgFormat):
    length = 16

    @classmethod
    def check(cls, arg):
        if not isinstance(arg, str):
            raise ArgumentError(arg, 'Argument is not string')
        if len(arg) > cls.length:
            raise ArgumentError(arg, 'String longer than ' + cls.length)
        if '\0' in arg:
            raise ArgumentError(arg, 'String contains zero-byte')

    @classmethod
    def encode(cls, arg):
        return bytearray(arg, 'ascii') + b'\0' * (cls.length - len(arg))

ArgFormats = {
    'c': ClearModpAF,
    's': SecretModpAF,
    'cw': ClearModpAF,
    'sw': SecretModpAF,
    'cg': ClearGF2NAF,
    'sg': SecretGF2NAF,
    'cgw': ClearGF2NAF,
    'sgw': SecretGF2NAF,
    'ci': ClearIntAF,
    'ciw': ClearIntAF,
    'i': ImmediateModpAF,
    'ig': ImmediateGF2NAF,
    'int': IntArgFormat,
    'p': PlayerNoAF,
    'str': String,
}

def format_str_is_reg(format_str):
    return ArgFormats[format_str].is_reg

def format_str_is_writeable(format_str):
    return format_str_is_reg(format_str) and format_str[-1] == 'w'


class Instruction(object):
    """
    Base class for a RISC-type instruction. Has methods for checking arguments,
    getting byte encoding, emulating the instruction, etc.
    """
    __slots__ = ['args', 'arg_format', 'code', 'caller']
    count = 0
    code_length = 10

    def __init__(self, *args, **kwargs):
        """ Create an instruction and append it to the program list. """
        self.args = list(args)
        if not kwargs.get('copying', False):
            self.check_args()
        if kwargs.get('add_to_prog', True):
            program.curr_block.instructions.append(self)
        if program.DEBUG:
            self.caller = [frame[1:] for frame in inspect.stack()[1:]]
        else:
            self.caller = None
        
        Instruction.count += 1
        if Instruction.count % 100000 == 0:
            print("Compiled %d lines at" % self.__class__.count, time.asctime())

    def get_code(self, prefix=0):
        return (prefix << self.code_length) + self.code

    def get_encoding(self):
        enc = int_to_bytes(self.get_code())
        # add the number of registers if instruction flagged as has var args
        if self.has_var_args():
            enc += int_to_bytes(len(self.args))
        for arg,format in zip(self.args, self.arg_format):
            enc += ArgFormats[format].encode(arg)
        return enc
    
    def get_bytes(self):
        try:
            return bytearray(self.get_encoding())
        except TypeError:
            raise CompilerError('cannot encode %s/%s' % (self, self.get_encoding()))
    
    def check_args(self):
        """ Check the args match up with that specified in arg_format """
        try:
            if len(self.args) != len(self.arg_format):
                raise CompilerError('Incorrect number of arguments for instruction %s' % (self))
        except TypeError:
            pass
        for n,(arg,f) in enumerate(zip(self.args, self.arg_format)):
            try:
                ArgFormats[f].check(arg)
            except ArgumentError as e:
                raise CompilerError('Invalid argument %d "%s" to instruction: %s'
                    % (n, e.arg, self) + '\n' + e.msg)
            except KeyError as e:
                raise CompilerError('Unknown argument %s for instruction %s' % (f, self))
    
    def get_used(self):
        """ Return the set of registers that are read in this instruction. """
        return (arg for arg,w in zip(self.args, self.arg_format) if \
            format_str_is_reg(w) and not format_str_is_writeable(w))
    
    def get_def(self):
        """ Return the set of registers that are written to in this instruction. """
        return (arg for arg,w in zip(self.args, self.arg_format) if \
            format_str_is_writeable(w))
    
    def get_pre_arg(self):
        return ""

    def has_var_args(self):
        try:
            len(self.arg_format)
            return False
        except:
            return True

    def is_vec(self):
        return False

    def is_gf2n(self):
        return False

    def get_size(self):
        return 1

    def add_usage(self, req_node):
        pass

    def merge_id(self):
        return type(self), self.get_size()

    def merge(self, other):
        if self.get_size() != other.get_size():
            # merge as non-vector instruction
            self.args = self.expand_vector_args() + other.expand_vector_args()
            if self.is_vec():
                self.size = 1
        else:
            self.args += other.args

    def expand_vector_args(self):
        if self.is_vec():
            for arg in self.args:
                arg.create_vector_elements()
                res = sum(list(zip(*self.args)), ())
                return list(res)
        else:
            return self.args

    def expand_merged(self, skip):
        return [self], 0

    def get_new_args(self, size, subs):
        new_args = []
        for arg, f in zip(self.args, self.arg_format):
            if arg in subs:
                new_args.append(subs[arg])
            elif arg is None:
                new_args.append(size)
            else:
                if format_str_is_writeable(f):
                    new_args.append(arg.copy())
                    subs[arg] = new_args[-1]
                else:
                    new_args.append(arg)
        return new_args

    # String version of instruction attempting to replicate encoded version
    def __str__(self):
        
        if self.has_var_args():
            varargCount = str(len(self.args)) + ', '
        else:
            varargCount = ''

        return self.__class__.__name__ + ' ' + self.get_pre_arg() + varargCount + ', '.join(str(a) for a in self.args)

    def __repr__(self):
        return self.__class__.__name__ + '(' + self.get_pre_arg() + ','.join(str(a) for a in self.args) + ')'

class VarArgsInstruction(Instruction):
    def has_var_args(self):
        return True

class VectorInstruction(Instruction):
    __slots__ = []
    is_vec = lambda self: True

    def get_code(self):
        return super(VectorInstruction, self).get_code(len(self.args[0]))

###
### Basic arithmetic
###

class AddBase(Instruction):
    __slots__ = []

class SubBase(Instruction):
    __slots__ = []

class MulBase(Instruction):
    __slots__ = []

###
### Basic arithmetic with immediate values
###

class ImmediateBase(Instruction):
    __slots__ = ['op']

class SharedImmediate(ImmediateBase):
    __slots__ = []
    arg_format = ['sw', 's', 'i']

class ClearImmediate(ImmediateBase):
    __slots__ = []
    arg_format = ['cw', 'c', 'i']


###
### Memory access instructions
###

class DirectMemoryInstruction(Instruction):
    __slots__ = []
    def __init__(self, *args, **kwargs):
        super(DirectMemoryInstruction, self).__init__(*args, **kwargs)

class IndirectMemoryInstruction(Instruction):
    __slots__ = []

    def get_direct(self, address):
        return self.direct(self.args[0], address, add_to_prog=False)

class ReadMemoryInstruction(Instruction):
    __slots__ = []

class WriteMemoryInstruction(Instruction):
    __slots__ = []

class DirectMemoryWriteInstruction(DirectMemoryInstruction, \
                                       WriteMemoryInstruction):
    __slots__ = []
    def __init__(self, *args, **kwargs):
        if not program.curr_tape.singular:
            raise CompilerError('Direct memory writing prevented in threads')
        super(DirectMemoryWriteInstruction, self).__init__(*args, **kwargs)

###
### I/O instructions
###

class DoNotEliminateInstruction(Instruction):
    """ What do you think? """
    __slots__ = []

class IOInstruction(DoNotEliminateInstruction):
    """ Instruction that uses stdin/stdout during runtime. These are linked
    to prevent instruction reordering during optimization. """
    __slots__ = []

    @classmethod
    def str_to_int(cls, s):
        """ Convert a 4 character string to an integer. """
        if len(s) > 4:
            raise CompilerError('String longer than 4 characters')
        n = 0
        for c in reversed(s.ljust(4)):
            n <<= 8
            n += ord(c)
        return n

class AsymmetricCommunicationInstruction(DoNotEliminateInstruction):
    """ Instructions involving sending from or to only one party. """
    __slots__ = []

class RawInputInstruction(AsymmetricCommunicationInstruction):
    """ Raw input instructions. """
    __slots__ = []

class PublicFileIOInstruction(DoNotEliminateInstruction):
    """ Instruction to reads/writes public information from/to files. """
    __slots__ = []

class TextInputInstruction(VarArgsInstruction, DoNotEliminateInstruction):
    """ Input from text file or stdin """
    __slots__ = []

###
### Data access instructions
###

class DataInstruction(Instruction):
    __slots__ = []
    field_type = 'modp'

    def add_usage(self, req_node):
        req_node.increment((self.field_type, self.data_type),
                           self.get_size() * self.get_repeat())

    def get_repeat(self):
        return 1

###
### Integer operations
### 

class IntegerInstruction(Instruction):
    """ Base class for integer operations. """
    __slots__ = []
    arg_format = ['ciw', 'ci', 'ci']

class StackInstruction(DoNotEliminateInstruction):
    """ Base class for thread-local stack instructions. """
    __slots__ = []

###
### Clear comparison instructions
###

class UnaryComparisonInstruction(Instruction):
    """ Base class for unary comparisons. """
    __slots__ = []
    arg_format = ['ciw', 'ci']

### 
### Clear shift instructions
### 

class ClearShiftInstruction(ClearImmediate):
    __slots__ = []

    def check_args(self):
        super(ClearShiftInstruction, self).check_args()
        if self.args[2] < 0:
            raise CompilerError('negative shift')

###
### Jumps etc
###

class JumpInstruction(Instruction):
    __slots__ = ['jump_arg']

    def set_relative_jump(self, value):
        if value == -1:
            raise CompilerError('Jump by -1 would cause infinite loop')
        self.args[self.jump_arg] = value

    def get_relative_jump(self):
        return self.args[self.jump_arg]


class CISC(Instruction):
    """
    Base class for a CISC instruction.
    
    Children must implement expand(self) to process the instruction.
    """
    __slots__ = []
    code = None

    def __init__(self, *args):
        self.args = args
        self.check_args()
        self.expand()
    
    def expand(self):
        """ Expand this into a sequence of RISC instructions. """
        raise NotImplementedError('expand method must be implemented')


class InvertInstruction(Instruction):
    __slots__ = []

    def __init__(self, *args, **kwargs):
        if program.options.ring and not self.is_gf2n():
            raise CompilerError('inverse undefined in rings')
        super(InvertInstruction, self).__init__(*args, **kwargs)
