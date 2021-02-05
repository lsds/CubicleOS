#ifndef IPC_H
#define IPC_H

void app_ipc_add_heap(void *, int, int);
void app_ipc_remove_heap(void *);
void app_ipc_add_stack(int i, char *begin, int size, int mask);
void app_ipc_add_global(int i, char *begin, int size, int mask);

#endif