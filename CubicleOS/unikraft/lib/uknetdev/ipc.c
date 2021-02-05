#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/param.h>
#include <uk/print.h>

#include <uk/essentials.h>

//stack
struct ipc_struct UKNETDEV_ipc_tab_stack[] __attribute__((aligned(4096))) = {
};
int UKNETDEV_ipc_tab_stack_nr = sizeof(UKNETDEV_ipc_tab_stack)/sizeof(struct ipc_struct);

void uknetdev_ipc_add_stack(char *begin, int size, int mask) {
}

//globals
struct ipc_struct UKNETDEV_ipc_tab_globals[] __attribute__((aligned(4096))) = {
	{0, 6, (1 << 17) | ( 1 << 11)},
};
int UKNETDEV_ipc_tab_globals_nr = sizeof(UKNETDEV_ipc_tab_globals)/sizeof(struct ipc_struct);

//heap
#define HEAP_MAX	100
struct ipc_struct UKNETDEV_ipc_tab_heap[HEAP_MAX] __attribute__((aligned(4096))) = {
};
int UKNETDEV_ipc_tab_heap_nr = HEAP_MAX;


void uknetdev_ipc_remove_heap(void *begin) {
	for(int i = 0; i < HEAP_MAX; i++) {
		if(UKNETDEV_ipc_tab_heap[i].begin != begin)
			continue;

		UKNETDEV_ipc_tab_heap[i].begin = 0;
		UKNETDEV_ipc_tab_heap[i].size = 0;
		UKNETDEV_ipc_tab_heap[i].mask = 0;

		return;
	}

	uk_pr_crit("Entry with begin %p doesn exist\n", begin);
	while(1);
}

void uknetdev_ipc_add_heap(void *begin, size_t size, int mask) {
	for(int i = 0; i < HEAP_MAX; i++) {
		if(UKNETDEV_ipc_tab_heap[i].size != 0)
			continue;

		UKNETDEV_ipc_tab_heap[i].begin = begin;
		UKNETDEV_ipc_tab_heap[i].size = size;
		UKNETDEV_ipc_tab_heap[i].mask = mask;

		return;
	}

	uk_pr_crit("add more records to UKNETDEV ipc struct\n");

	for(int i = 0; i < HEAP_MAX; i++) {
		uk_pr_crit("[%d] %p %x %x\n", i, UKNETDEV_ipc_tab_heap[i].begin, UKNETDEV_ipc_tab_heap[i].size, UKNETDEV_ipc_tab_heap[i].mask);
	}
	while(1);
}
