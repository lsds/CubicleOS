gcc for Unikraft
=============================

This is a port of gcc for Unikraft as an external library.

For the time being we only port libbacktrace and libffi libraries as
they are needed for the libgo port. You will need newlib for both
libraries to build. In addition, 

For libffi you will need the pthread\_embedded external library too.

For libbacktrace you will need the following external libraries:
+ compiler-rt 
+ libunwind
+ libcxx
+ libcxxabi

Note that because of a documented bug in libunwind `unw_getcontext`
leads to a page fault, and in turn `backtrace_full`will also lead to
one as it uses `_Unwind_Backtrace` which calls `unw_getcontext`.

Please refer to the `README.md` as well as the documentation in the `doc/`
subdirectory of the main unikraft repository for further information.
