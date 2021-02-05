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

extern struct ipc_struct VFSCORE_ipc_tab_stack[];
void vfs_ipc_add_heap(void *, size_t, int);
void vfs_ipc_remove_heap(void *);

void *dlmalloc(size_t size);
void *dlcalloc(size_t nmemb, size_t size);
void dlfree(void *ptr);
#endif