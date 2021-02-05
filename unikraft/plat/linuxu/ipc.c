#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/param.h>
#include <uk/assert.h>

#include <uk/essentials.h>

//stack
struct ipc_struct LINUXUPLAT_ipc_tab_stack[] __attribute__((aligned(4096))) = {
//*stack, create_stack
	{0, sizeof(long), (1 << 6) | ( 1 << 19)},
//*tls, create_stacl
	{1, sizeof(long), (1 << 6) | ( 1 << 19)},
};
int LINUXUPLAT_ipc_tab_stack_nr = sizeof(LINUXUPLAT_ipc_tab_stack)/sizeof(struct ipc_struct);

void linuxuplat_ipc_add_stack(char *begin, int size, int mask) {
	LINUXUPLAT_ipc_tab_stack[0].begin = begin;
	LINUXUPLAT_ipc_tab_stack[0].size = size;
	LINUXUPLAT_ipc_tab_stack[0].mask = mask;
}

//globals
struct ipc_struct LINUXUPLAT_ipc_tab_globals[0] __attribute__((aligned(4096))) = {
};
int LINUXUPLAT_ipc_tab_globals_nr = sizeof(LINUXUPLAT_ipc_tab_globals)/sizeof(struct ipc_struct);

//heap
#define HEAP_MAX	100
struct ipc_struct LINUXUPLAT_ipc_tab_heap[HEAP_MAX] __attribute__((aligned(4096))) = {
};
int LINUXUPLAT_ipc_tab_heap_nr = HEAP_MAX;


void linuxuplat_ipc_remove_heap(void *begin) {
	for(int i = 0; i < HEAP_MAX; i++) {
		if(LINUXUPLAT_ipc_tab_heap[i].begin != begin)
			continue;

		LINUXUPLAT_ipc_tab_heap[i].begin = 0;
		LINUXUPLAT_ipc_tab_heap[i].size = 0;
		LINUXUPLAT_ipc_tab_heap[i].mask = 0;

		return;
	}

	uk_pr_crit("Entry with begin %p doesn exist\n", begin);
	while(1);
}

void linuxuplat_ipc_add_heap(void *begin, size_t size, int mask) {
	for(int i = 0; i < HEAP_MAX; i++) {
		if(LINUXUPLAT_ipc_tab_heap[i].size != 0)
			continue;

		LINUXUPLAT_ipc_tab_heap[i].begin = begin;
		LINUXUPLAT_ipc_tab_heap[i].size = size;
		LINUXUPLAT_ipc_tab_heap[i].mask = mask;

		return;
	}

	uk_pr_crit("add more records to LINUXUPLAT ipc struct\n");
	while(1);
}
