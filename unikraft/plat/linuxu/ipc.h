#ifndef IPC_H
#define IPC_H
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/param.h>

#include <uk/essentials.h>

extern struct ipc_struct LINUXUPLAT_ipc_tab_stack[];
void linuxuplat_ipc_add_heap(void *, size_t, int);
void linuxuplat_ipc_remove_heap(void *);
void linuxuplat_ipc_add_stack(char *begin, int size, int mask);


#endif