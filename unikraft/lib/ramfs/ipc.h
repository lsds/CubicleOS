#ifndef IPC_H
#define IPC_H
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/param.h>

#include <vfscore/dentry.h>
#include <vfscore/vnode.h>

#include <uk/essentials.h>

extern struct ipc_struct RAMFS_ipc_tab_stack[];
void ramfs_ipc_add_heap(void *, size_t, int);
void ramfs_ipc_remove_heap(void *);
void ramfs_ipc_add_stack(char *begin, int size, int mask);


void *dl_ramfs_malloc(size_t size);
void *dl_ramfs_calloc(size_t nmemb, size_t size);
void dl_ramfs_free(void *ptr);
int dl_ramfs_posix_memalign(void **memptr, size_t alignment, size_t size);

#endif