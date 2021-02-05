#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/param.h>

#include <vfscore/dentry.h>
#include <vfscore/vnode.h>

#include <uk/essentials.h>

//stack
struct ipc_struct RAMFS_ipc_tab_stack[] __attribute__((aligned(4096))) = {
//ramfs_lookup, vp
	{0, sizeof(long), (1 << 21) | ( 1 << 2)},
};
int RAMFS_ipc_tab_stack_nr = sizeof(RAMFS_ipc_tab_stack)/sizeof(struct ipc_struct);

void ramfs_ipc_add_stack(char *begin, int size, int mask) {
	RAMFS_ipc_tab_stack[0].begin = begin;
	RAMFS_ipc_tab_stack[0].size = size;
	RAMFS_ipc_tab_stack[0].mask = mask;
}

//globals
struct ipc_struct RAMFS_ipc_tab_globals[0] __attribute__((aligned(4096))) = {
};
int RAMFS_ipc_tab_globals_nr = sizeof(RAMFS_ipc_tab_globals)/sizeof(struct ipc_struct);

//heap
#define HEAP_MAX	100
struct ipc_struct RAMFS_ipc_tab_heap[HEAP_MAX] __attribute__((aligned(4096))) = {
};
int RAMFS_ipc_tab_heap_nr = HEAP_MAX;


void ramfs_ipc_remove_heap(void *begin) {
	for(int i = 0; i < HEAP_MAX; i++) {
		if(RAMFS_ipc_tab_heap[i].begin != begin)
			continue;

		RAMFS_ipc_tab_heap[i].begin = 0;
		RAMFS_ipc_tab_heap[i].size = 0;
		RAMFS_ipc_tab_heap[i].mask = 0;

		return;
	}

	uk_pr_crit("Entry with begin %p doesn exist\n", begin);
	while(1);
}

void ramfs_ipc_add_heap(void *begin, size_t size, int mask) {
	for(int i = 0; i < HEAP_MAX; i++) {
		if(RAMFS_ipc_tab_heap[i].size != 0)
			continue;

		RAMFS_ipc_tab_heap[i].begin = begin;
		RAMFS_ipc_tab_heap[i].size = size;
		RAMFS_ipc_tab_heap[i].mask = mask;

		return;
	}

	uk_pr_crit("add more records to RAMFS ipc struct\n");
	while(1);
}
