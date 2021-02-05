#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/param.h>

#include <uk/plat/memory.h>
#include <uk/essentials.h>

struct ipc_struct UKBOOT_ipc_tab_stack[] __attribute__((aligned(4096))) = {
	{0, sizeof(struct ukplat_memregion_desc), (1 << 19) | ( 1 << 25)}
};

int UKBOOT_ipc_tab_stack_nr = sizeof(UKBOOT_ipc_tab_stack)/sizeof(struct ipc_struct);

struct ipc_struct UKBOOT_ipc_tab_globals[0] __attribute__((aligned(4096))) = {
};

int UKBOOT_ipc_tab_globals_nr = sizeof(UKBOOT_ipc_tab_globals)/sizeof(struct ipc_struct);

#define HEAP_MAX 10

struct ipc_struct UKBOOT_ipc_tab_heap[HEAP_MAX] __attribute__((aligned(4096))) = {
};

int UKBOOT_ipc_tab_heap_nr = HEAP_MAX;
int UKBOOT_ipc_tab_heap_nr_real = 0;

void ukboot_ipc_add_heap(void *begin, size_t size, int mask) {
//	uk_pr_crit("Adding %p %x %x\n", begin, size, mask);

	for(int i = 0; i < HEAP_MAX; i++) {
		if(UKBOOT_ipc_tab_heap[i].size != 0) {
//			uk_pr_crit("HEAP: %d: %p %x %x\n", i, UKBOOT_ipc_tab_heap[i].begin, UKBOOT_ipc_tab_heap[i].size, UKBOOT_ipc_tab_heap[i].mask);
			continue;
		}

		UKBOOT_ipc_tab_heap[i].begin = begin;
		UKBOOT_ipc_tab_heap[i].size = size;
		UKBOOT_ipc_tab_heap[i].mask = mask;

		if(i >= UKBOOT_ipc_tab_heap_nr_real)
			UKBOOT_ipc_tab_heap_nr_real = i;

		return;
	}

	uk_pr_crit("add more records to UKBOOT HEAP ipc struct\n");
	while(1);
}

void ukboot_ipc_remove_heap(void *begin) {
//	uk_pr_crit("Removing %p\n", begin);
	for(int i = 0; i < UKBOOT_ipc_tab_heap_nr_real; i++) {
		if(UKBOOT_ipc_tab_heap[i].begin != begin)
			continue;

		UKBOOT_ipc_tab_heap[i].begin = 0;
		UKBOOT_ipc_tab_heap[i].size = 0;
		UKBOOT_ipc_tab_heap[i].mask = 0;

		return;
	}

	uk_pr_crit("Entry with begin %p doesn exist\n", begin);
//	while(1);
}
