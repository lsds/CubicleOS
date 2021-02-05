#ifndef IPC_H
#define IPC_H
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/param.h>

#include <uk/essentials.h>

struct ipc_struct LWIP_ipc_tab_stack[];
struct ipc_struct LWIP_ipc_tab_globals[];
void lwip_ipc_add_heap(void *, size_t, int);
void lwip_ipc_remove_heap(void *);
void lwip_ipc_add_stack(int i, char *begin, int size, int mask);
void lwip_ipc_add_globals(int i, void *begin, size_t size, int mask);

#endif