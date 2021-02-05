#  CubicleOS: A Library OS with Software Componentisation for Practical Isolation

CubicleOS is a prototype of a LibOS that allows to partition a monolithic system without the use of message-based IPC.
CubicleOS works as a runtime partitioning engine, but requires changes to the software and relies on hardare support, the Intel MPK technology.

CubicleOS offers three core abstractions to component developers: (i) cubicles, which are isolated components; (ii) windows,
which enable dynamic sharing across components; and (iii) cross-cubicle calls, which carry out control flow authorisation.
Together, these abstractions provide spatial memory isolation, temporal memory isolation, and control flow integrity, respectively.

CubicleOS is implemented on top of Unikraft, a featurerich library OS that can execute existing POSIX-compatible applications,
and runs on top of an existing host OS such as Linux.

## Source structure 

* [CubicleOS](CubicleOS): CubicleOS sources
* [check.c](check.c): A simple test to check Intel MPK
* [Dockerfile](Dockerfile): A Dockerfile to build CubicleOS and two applications, NGINX and SQLite
* [run.sh](run.sh): a bash script to build the container and run various apps with CubicleOS
* [build.conf](build.conf): a configuration file used for Genode. Used for evaluation
* [parser](parser): A parser for raw SQLite Speedtest1 results. Used for evaluation.

## How to build

Just build the container as follows:
```
docker build . --tag cubicles
```

## How to run Speedtest1 benchmark

```
docker run --env LD_LIBRARY_PATH=/CubicleOS/kernel/sqlite --privileged --rm -it  cubicles:latest /CubicleOS/kernel/loader /CubicleOS/kernel/sqlite --size 100 -mmap 0 --stats testing
```

## How to run NGINX 

```
docker run --cap-add=NET_ADMIN --device /dev/net/tun --env LD_LIBRARY_PATH=/CubicleOS/kernel/nginx --privileged --rm -it  cubicles:latest /bin/bash /CubicleOS/kernel/net_COS.sh
```

## How to reproduce results from the paper
### SQLite

1. The benchmark on top of Linux inside a container, e.g. *vanilla*
```
docker run --privileged --rm -it  cubicles:latest /CubicleOS/vanilla/vanilla --size 100 -mmap 0 --stats testing
```

To use the parser, please save the output as 01_vanila.txt

2. On top of Unikraft
```
docker run --privileged --rm -it  cubicles:latest /CubicleOS/app-sqlite/build/app-sqlite_linuxu-x86_64 --size 100 -mmap 0 --stats testing
```
To use the parser, please save the output as 02_unikraft.txt

3. Genode with 3 components on top of Linux (no virtualisation)
```
docker run --privileged --rm -it  cubicles:latest make -C /genode/build/x86_64 run/sqlite
```
To use the parser, please save the output as 03_genode_3.txt

4. Genode with 4 components on top of Linux (no virtualisation)
```
docker run --privileged --rm -it  cubicles:latest make -C /genode/build/x86_64 run/sqlite4
```
To use the parser, please save the output as 04_genode_4.txt

5. CubicleOS split into 3 components
```
docker run --env LD_LIBRARY_PATH=/CubicleOS/kernel/sqlite --privileged --rm -it  cubicles:latest /CubicleOS/kernel/loader3 /CubicleOS/kernel/sqlite --size 100 -mmap 0 --stats testing
```
To use the parser, please save the output as 05_cubicle_3.txt

6. CubicleOS split into 4 components
```
docker run --env LD_LIBRARY_PATH=/CubicleOS/kernel/sqlite --privileged --rm -it  cubicles:latest /CubicleOS/kernel/loader4 /CubicleOS/kernel/sqlite --size 100 -mmap 0 --stats testing
```
To use the parser, please save the output as 06_cubicle_4.txt

7. CubicleOS split into 7 components
```
docker run --env LD_LIBRARY_PATH=/CubicleOS/kernel/sqlite --privileged --rm -it  cubicles:latest /CubicleOS/kernel/loader /CubicleOS/kernel/sqlite --size 100 -mmap 0 --stats testing
```
To use the parser, please save the output as 07_cubicle_7.txt

8. Genode with 3 components on top of SeL4 (requires virtualisation)
```
docker run --privileged --rm -it  cubicles:latest make -C /genode/build/x86_64 KERNEL=sel4 BOARD=pc run/sqlite
```
To use the parser, please save the output as 08_sel4_3.txt

9. Genode with 4 components on top of SeL4 (requires virtualisation)
```
docker run --privileged --rm -it  cubicles:latest make -C /genode/build/x86_64 KERNEL=sel4 BOARD=pc run/sqlite4
```
To use the parser, please save the output as 09_sel4_4.txt

10. Genode with 3 components on top of Fiasco.OC (requires virtualisation)
```
docker run --privileged --rm -it  cubicles:latest make -C /genode/build/x86_64 KERNEL=foc BOARD=pc run/sqlite
```
To use the parser, please save the output as 10_foc_3.txt

11. Genode with 4 components on top of Fiasco.OC (requires virtualisation)
```
docker run --privileged --rm -it  cubicles:latest make -C /genode/build/x86_64 KERNEL=foc BOARD=pc run/sqlite4
```
To use the parser, please save the output as 11_foc_4.txt

12. Genode with 3 components on top of NOVA (requires virtualisation)
```
docker run --privileged --rm -it  cubicles:latest make -C /genode/build/x86_64 KERNEL=nova BOARD=pc run/sqlite
```
To use the parser, please save the output as 12_nova_3.txt

13. Genode with 4 components on top of NOVA (requires virtualisation)
```
docker run --privileged --rm -it  cubicles:latest make -C /genode/build/x86_64 KERNEL=nova BOARD=pc run/sqlite4
```
To use the parser, please save the output as 13_nova_4.txt


### NGINX on top of Unikraft

CubicleOS is based on Unikraft, which didn't provide support for networking for the linuxu platform. We sketched the network support, but it also relies on the Monitor, a CubicleOS component.
```
docker run --cap-add=NET_ADMIN --device /dev/net/tun --env LD_LIBRARY_PATH=/CubicleOS/kernel/nginx --privileged --rm -it  cubicles:latest /bin/bash /CubicleOS/kernel/net_UNI.sh
```
This script compiles and runs NGINX. This setup does not use MPK for isolation.

### How to use Parser to compare results

Parser processes raw results of Speedtest1. It receives as input a log of SQL tests for various setups and generates
relative results. It needs a full of set of tests stored in a directory, which is provided as an argument:
```
./parser paper
```
Two data directories, paper and yandex, are provided as examples. The former contains data obtained from a private server.
These results are reported in the paper. The latter contains results obtained from a public cloud Yandex (http://cloud.yandex.ru/). If you want to use this script but do not have all the necessary setups, you can copy the content of the first file into the missing files.

### How to regenerate rootfs for NGINX

The Docker script fetches pre-generated RAMFS image. Technically, it is a CPIO image turn into .h resource file.
At the beginning, CubicleOS creates a ramdrive and fill it by data from this resource file.
If you want to update this file, you should:

* Generate CPIO by running this _inside_ the desired directory: 
```
cd /CubicleOS/app-nginx/fs0
find . -print -depth | sort | cpio -vo -H newc > ../root.cpio
cd ..
xxd -i root.cpio > ../unikraft/lib/libnginx/rootfs.h
```
Then compile app-nginx and kernel

### I want more configurations

There are several options that you can add by changing Makefile or loader.c:
* NOMPK: Loader implements cross-cubicle calls, but cubicles are not protected. Works on any suitable hardware and does not require porting.
* ALLOW_APP: There is no isolation between LibOS and Application, but the LibOS is partitioned. Allows to run non-modified Applications.
* ALLOW_ALL: All cross-cubicle accesses are alowed and don't require Windows. Allows to estimate the overhead of cross-cubicle calls.
* DEBUG, WDEBUG, WDEBUG2: various forms of debug information
* WDOT: used to generate DOT diagrams. generates raw data, requires post-processing
* NWRAP: do not use cross-cubicle calls and MPK. Just run a set of linked modules.


## Some CubicleOS internals

* [getcontext.S](CubicleOS/kernel/getcontext.S) [setcontext.S](CubicleOS/kernel/setcontext.S) [coro.h](CubicleOS/kernel/coro.h) [ucontext_i.h](CubicleOS/kernel/ucontext_i.h): Implements low-level switching mechanic. Each cross-cubicle call -- coroutine-like switch
* [elf_hook.h](CubicleOS/kernel/elf_hook.h) [elf_hook.c](CubicleOS/kernel/elf_hook.c) [plthook_elf.c](CubicleOS/kernel/plthook_elf.c)  [plthook.h](CubicleOS/kernel/): used to patch GPT and properly parse ELF symbols. Only one will be used in the future.
* [mini-printf.c](CubicleOS/kernel/mini-printf.c) [mini-printf.h](CubicleOS/kernel/mini-printf.h): printf inside a trap, needed for debuging at some stage of the development
* [Makefile](CubicleOS/kernel/Makefile): Compiles dependencies, re-link binaries as separate .so libraries.
* [fig.py](CubicleOS/kernel/fig.py): parses [unikraft/unikraft.ll](CubicleOS/unikraft/unikraft.ll) and generates cross-cubicle calls for each public function. Generates gen_hooks.h.
* [loader.c](CubicleOS/kernel/loader.c) [headers.h](CubicleOS/kernel/headers.h): The Monitor. Loads libraries into memory, patches GPT, intercepts cross-cubicle calls, switches pages between cubicles.
  
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