#  CubicleOS: A Library OS with Software Componentisation for Practical Isolation

CubicleOS is a prototype of a LibOS that allows to partition a monolithic system without the use of message-based IPC.
CubicleOS works as a runtime partitioning engine, but requires changes to the software and relies on hardare support, the Intel MPK technology.

CubicleOS offers three core abstractions to component developers: (i) cubicles, which are isolated components; (ii) windows,
which enable dynamic sharing across components; and (iii) cross-cubicle calls, which carry out control flow authorisation.
Together, these abstractions provide spatial memory isolation, temporal memory isolation, and control flow integrity, respectively.

CubicleOS is implemented on top of Unikraft, a featurerich library OS that can execute existing POSIX-compatible applications,
and runs on top of an existing host OS such as Linux.

## How to build and run
```
git clone https://github.com/lsds/cubicleos
```
### Check MPK
```
cd check_mpk
gcc ./check.c
./a.out
```

Expected output:
```
pkey alloc = 1
pkey: Success
```

### Building and running 
```
cd cubicleos/sqlite
make
cd ../kernel/
LD_LIBRARY_PATH=./sqlite unbuffer ./loader sqlite --size 100 -mmap 0 --stats testing | ts -s '%M:%.S'
```

you can use `sqlite3` and `sqlite4`, they deploy different number of cubicles.

## Some configuration opetions

There are several options that you can add by changing Makefile or loader.c:
* NOMPK: Loader implements cross-cubicle calls, but cubicles are not protected. Works on any suitable hardware and does not require porting.
* ALLOW_APP: There is no isolation between LibOS and Application, but the LibOS is partitioned. Allows to run non-modified Applications.
* ALLOW_ALL: All cross-cubicle accesses are alowed and don't require Windows. Allows to estimate the overhead of cross-cubicle calls.
* DEBUG, WDEBUG, WDEBUG2: various forms of debug information
* WDOT: used to generate DOT diagrams. generates raw data, requires post-processing
* NWRAP: do not use cross-cubicle calls and MPK. Just run a set of linked modules.


## Some CubicleOS internals

* [getcontext.S](kernel/getcontext.S) [setcontext.S](kernel/setcontext.S) [coro.h](kernel/coro.h) [ucontext_i.h](kernel/ucontext_i.h): Implements low-level switching mechanic. Each cross-cubicle call -- coroutine-like switch
* [elf_hook.h](kernel/elf_hook.h) [elf_hook.c](kernel/elf_hook.c) [plthook_elf.c](kernel/plthook_elf.c)  [plthook.h](kernel/): used to patch GPT and properly parse ELF symbols. Only one will be used in the future.
* [mini-printf.c](kernel/mini-printf.c) [mini-printf.h](kernel/mini-printf.h): printf inside a trap, needed for debuging at some stage of the development
* [Makefile](kernel/Makefile): Compiles dependencies, re-link binaries as separate .so libraries.
* [fig.py](kernel/fig.py): parses [unikraft/unikraft.ll](unikraft/unikraft.ll) and generates cross-cubicle calls for each public function. Generates gen_hooks.h.
* [loader.c](kernel/loader.c) [headers.h](kernel/headers.h): The Monitor. Loads libraries into memory, patches GPT, intercepts cross-cubicle calls, switches pages between cubicles.
  
## Disclaimer

The project is a proof-of-concept and at a very early development stage. It does not pretend to be ready-to-use. It likely has various flaws, requires significant improvement, and refactoring.

## What does LLVM macro stand for?

Early prototypes use semi-automatic window management. Now all windows 'manual' and this macro just shows the orgiginal form of variables.

## Known bugs and errors

### RAMFS cannot allocate memory

There is a known bug inside Unikraft or CubicleOS. It appears once per 10 or 20 runs:
```
[ 9971.877293] CRIT: [libramfs] dlmemreq.c @ 41   : cannot allocate memory (262144000) for RAMFS 
```

You need to stop the test, kill all relevant processes, and start again.

### Does not start 

Different OSes and Kernels somethimes have a slightly different structure of /proc/self/mem, which we parse at start.
We tested CubicleOSs with Debian 10 and Linux kernel 4.4.0. 