# SQLite for Unikraft
This is the port of SQLite for Unikraft as external library.

## Build
SQLite depends on the following libraries, that need to be added to
`Makefile` in this order:

* `pthreads`, e.g. `pthread-embedded`
* `libc`, e.g. `newlib`

Before you proceed to writing your own application, you can use the
`main()` function provided in the SQLite glue code by enabling it in
its configuration menu.

## Root filesystem

To import/export databases and/or csv files, it is necessary to have
a filesystem. The steps for creating and using a filesystem are the
same as the ones used for
[nginx](https://github.com/unikraft/lib-nginx/blob/staging/README.md).

## Further information
Please refer to the `README.md` as well as the documentation in the
`doc/` subdirectory of the main unikraft repository.
