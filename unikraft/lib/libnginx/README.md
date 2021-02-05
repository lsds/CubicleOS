# Nginx for Unikraft
This is the port of Nginx for Unikraft as external library.

## Build
Nginx depends on the following libraries, that need to
be added to `Makefile` in this order:

* `pthreads`, e.g. `pthread-embedded`
* `libc`, e.g. `newlib`
* network stack, e.g. `lwip`

Before you proceed to writing your own application, you can use the `main()`
function provided in the Nginx glue code by enabling it in its configuration
menu.

## Root filesystem
### Creating the filesystem
Nginx needs a filesystem which should contain its configuration files, HTML
files and log files. Therefore, the filesystem needs to be created before
running the VM. You may find such an example in `nginx-rootfs-example/`
subdirectory.

### Using the filesystem
Mounting the filesystem is a transparent operation. All you have to do
is to provide the right Qemu parameters in order for Unikraft to mount
the filesystem.  We will use the 9pfs support for filesystems and for
this you will need to use the following parameters:

```bash
-fsdev local,id=myid,path=<some directory>,security_model=none \
-device virtio-9p-pci,fsdev=myid,mount_tag=test,disable-modern=on,disable-legacy=off
```

You should also use `vfs.rootdev=test` to specify the 9pfs mounting
tag to Unikraft. To enable 9pfs, you'll need to select the following
menu options, all under `Library Configuration`:

* `uk9p: 9p client`
* `vfscore: VFS Core Interface`
	  &rarr; `vfscore: Configuration`
	  &rarr; `Automatically mount a root filesysytem`
	  &rarr; `Default root filesystem`
	  &rarr; `9PFS`
* `uk library parameter`

## Further information
Please refer to the `README.md` as well as the documentation in the `doc/`
subdirectory of the main unikraft repository.
