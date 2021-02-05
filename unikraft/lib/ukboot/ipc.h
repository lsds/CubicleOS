#ifndef IPC_H
#define IPC_H
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/param.h>

#include <uk/essentials.h>

extern struct ipc_struct UKBOOT_ipc_tab_stack[];
void ukboot_ipc_add_heap(void *, size_t, int);
void ukboot_ipc_remove_heap(void *);

#endif