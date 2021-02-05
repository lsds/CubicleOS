#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/param.h>
#include <uk/print.h>

#include <vfscore/mount.h>

#include <uk/essentials.h>

//stack
struct ipc_struct LWIP_ipc_tab_stack[] __attribute__((aligned(4096))) = {
//err_t uknetdev_init(struct netif *nf)
	{0, 0, (1 << 17) | ( 1 << 11)},
	{1, 0, (1 << 17) | ( 1 << 11)},
	{2, 0, (1 << 17) | ( 1 << 11)},
	{3, 0, (1 << 17) | ( 1 << 11)},
//socket static int sock_fd_alloc(int sock_fd)
	{4, 0, (1 << 17) | ( 1 << 21)},
//void uknetdev_input
	{5, 0, (1 << 17) | ( 1 << 21)},
};
int LWIP_ipc_tab_stack_nr = sizeof(LWIP_ipc_tab_stack)/sizeof(struct ipc_struct);

//globals
struct ipc_struct LWIP_ipc_tab_globals[] __attribute__((aligned(4096))) = {
	{0, sizeof(struct mount), (1 << 17) | ( 1 << 21)},
	{1, sizeof(long), (1 << 17) | ( 1 << 21)},
	{2, sizeof(long), (1 << 17) | ( 1 << 21)},
	{3, sizeof(long), (1 << 17) | ( 1 << 21)},
};
int LWIP_ipc_tab_globals_nr = sizeof(LWIP_ipc_tab_globals)/sizeof(struct ipc_struct);

//heap
#define HEAP_MAX	100
struct ipc_struct LWIP_ipc_tab_heap[HEAP_MAX] __attribute__((aligned(4096))) = {
};
int LWIP_ipc_tab_heap_nr = HEAP_MAX;


void lwip_ipc_remove_heap(void *begin) {
	for(int i = 0; i < HEAP_MAX; i++) {
		if(LWIP_ipc_tab_heap[i].begin != begin)
			continue;
#if 0
uk_pr_crit("removing %d %p %x %x\n", i, LWIP_ipc_tab_heap[i].begin, LWIP_ipc_tab_heap[i].size, LWIP_ipc_tab_heap[i].mask);

	for(int j = 0; j < HEAP_MAX; j++) {
		uk_pr_crit("[%d] %p %x %x\n", j, LWIP_ipc_tab_heap[j].begin, LWIP_ipc_tab_heap[j].size, LWIP_ipc_tab_heap[j].mask);
	}
#endif

		LWIP_ipc_tab_heap[i].begin = 0;
		LWIP_ipc_tab_heap[i].size = 0;
		LWIP_ipc_tab_heap[i].mask = 0;


		return;
	}

	uk_pr_crit("Entry with begin %p doesn exist\n", begin);
	while(1);
}

void lwip_ipc_add_heap(void *begin, size_t size, int mask) {

	for(int i = 0; i < HEAP_MAX; i++) {

		if((LWIP_ipc_tab_heap[i].size != 0) && 
			(LWIP_ipc_tab_heap[i].begin != begin) //this workaround doesn't exist in other IPCs. see sockets.c:284
		)
			continue;

		LWIP_ipc_tab_heap[i].begin = begin;
		LWIP_ipc_tab_heap[i].size = size;
		LWIP_ipc_tab_heap[i].mask = mask;
//uk_pr_crit("add %d %p %x %x\n", i, begin, size, mask);
		return;
	}

	uk_pr_crit("add more records to LWIP ipc struct\n");

	for(int i = 0; i < HEAP_MAX; i++) {
		uk_pr_crit("[%d] %p %x %x\n", i, LWIP_ipc_tab_heap[i].begin, LWIP_ipc_tab_heap[i].size, LWIP_ipc_tab_heap[i].mask);
	}


	while(1);
}

void lwip_ipc_add_globals(int i, void *begin, size_t size, int mask) {
		LWIP_ipc_tab_globals[i].begin = begin;
		LWIP_ipc_tab_globals[i].size = size;
		LWIP_ipc_tab_globals[i].mask = mask;
}

void lwip_ipc_add_stack(int i, void *begin, int size, int mask) {
		LWIP_ipc_tab_stack[i].begin = begin;
		LWIP_ipc_tab_stack[i].size = size;
		LWIP_ipc_tab_stack[i].mask = mask;
}
