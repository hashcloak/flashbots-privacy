Virtual Machine
===============

Calling ``compile.py`` outputs the computation in a format specific to
MP-SPDZ. This includes a schedule file and one or several bytecode
files. The schedule file can be found at
``Programs/Schedules/<progname>.sch``. It contains the names of all
bytecode files found in ``Programs/Bytecode`` and the maximum number
of parallel threads. Each bytecode file represents the complete
computation of one thread, also called tape. The computation of the
main thread is always ``Programs/Bytecode/<progname>-0.bc`` when
compiled by the compiler.

The bytecode is made up of 32-bit units in big-endian byte
order. Every unit represents an instruction code (possibly including
vector size), register number, or immediate value.

For example, adding the secret integers in registers 1 and 2 and then
storing the result at register 0 leads to the following bytecode (in
hexadecimal representation):

.. code-block:: none

  00 00 00 21  00 00 00 00  00 00 00 01  00 00 00 02

This is because ``0x021`` is the code of secret integer addition. The
debugging output (``compile.py -a <prefix>``) looks as follows::

  adds s0, s1, s2 # <instruction number>

There is also a vectorized addition. Adding 10 secret integers in
registers 10-19 and 20-29 and then storing the result in registers 0-9
is represented as follows in bytecode:

.. code-block:: none

  00 00 28 21  00 00 00 00  00 00 00 0a  00 00 00 14

This is because the vector size is stored in the upper 22 bits of the
first 32-bit unit (instruction codes are up to 10 bits long), and
``0x28`` equals 40 or 10 shifted by two bits. In the debugging output
the vectorized addition looks as follows::

  vadds 10, s0, s10, s20 # <instruction number>

Finally, some instructions have a variable number of arguments to
accommodate any number of parallel operations. For these, the first
argument usually indicates the number of arguments yet to come. For
example, multiplying the secret integers in registers 2 and 3 as well
as registers 4 and 5 and the storing the two results in registers 0
and 1 results in the following bytecode:

.. code-block:: none

  00 00 00 a6  00 00 00 06  00 00 00 00  00 00 00 02
  00 00 00 03  00 00 00 01  00 00 00 04  00 00 00 05

and the following debugging output::

  muls 6, s0, s2, s3, s1, s4, s5 # <instruction number>

Note that calling instructions in high-level code never is done with
the explicit number of arguments. Instead, this is derived from number
of function arguments. The example above would this simply be called
as follows::

  muls(s0, s2, s3, s1, s4, s5)

Instructions
------------

The following table list all instructions except the ones for
:math:`\mathrm{GF}(2^n)` computation, untested ones, and those considered
obsolete.

.. csv-table::
   :header: Name, Code
   :widths: 50, 15, 100
   :file: instructions.csv


Compiler.instructions module
----------------------------

.. automodule:: Compiler.instructions
   :members:
   :no-undoc-members:
   :exclude-members: asm_input, inputmask, lts, print_char4_regint,
		     print_char_regint, protectmemc, sqrs,
		     start_grind, startprivateoutput, stop_grind,
		     stopprivateoutput, writesocketc, writesocketint,
		     protectmemint, protectmems, print_mem,
		     matmul_base, g2muls, inputmixed_base, raw_output

Compiler.GC.instructions module
-------------------------------

.. automodule:: Compiler.GC.instructions
   :members:
   :no-undoc-members:
   :exclude-members: BinaryVectorInstruction, NonVectorInstruction,
		     NonVectorInstruction1, ldmsd, stmsd, ldmsdi, stmsdi,
		     stmsdci
