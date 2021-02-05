#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/param.h>

#include <vfscore/dentry.h>
#include <vfscore/vnode.h>

#include <uk/essentials.h>



struct ipc_struct VFSCORE_ipc_tab_stack[] __attribute__((aligned(4096))) = {
//	{&filename
	{0, sizeof(long), (1 << 21) | ( 1 << 2)},
//	{&ddp
	{1, sizeof(long), (1 << 21) | ( 1 << 2)},
//	{&dp
	{2, sizeof(long), (1 << 21) | ( 1 << 2)},
//	{&vp
	{3, sizeof(long), (1 << 21) | ( 1 << 2)},
//	{&name
	{4, PATH_MAX, (1 << 21) | ( 1 << 2)},
//	{&path_in_open
	{5, PATH_MAX, (1 << 21) | ( 1 << 2)},
//	{&path_in_unlink
	{6, PATH_MAX, (1 << 21) | ( 1 << 2)},
//	{&path_in_lxstat
	{7, PATH_MAX, (1 << 21) | ( 1 << 2)},
//uio_write +
	{8, sizeof(struct uio), (1 << 21) | ( 1 << 2) | (1 << 17) },
//uio_read +
	{9, sizeof(struct uio), (1 << 21) | ( 1 << 2) | (1 << 17) },
//vattr +
	{10, sizeof(struct vattr), (1 << 21) | ( 1 << 2)},
};
int VFSCORE_ipc_tab_stack_nr = sizeof(VFSCORE_ipc_tab_stack)/sizeof(struct ipc_struct);


struct ipc_struct VFSCORE_ipc_tab_globals[0] __attribute__((aligned(4096))) = {
};
int VFSCORE_ipc_tab_globals_nr = sizeof(VFSCORE_ipc_tab_globals)/sizeof(struct ipc_struct);

#define HEAP_MAX	101

struct ipc_struct VFSCORE_ipc_tab_heap[HEAP_MAX] __attribute__((aligned(4096))) = {
};
int VFSCORE_ipc_tab_heap_nr = sizeof(VFSCORE_ipc_tab_heap)/sizeof(struct ipc_struct);
int VFSCORE_ipc_tab_heap_nr_real = 0;



void vfs_ipc_add_heap(void *begin, size_t size, int mask) {
//	uk_pr_crit("Adding %p %x %x\n", begin, size, mask);

	for(int i = 0; i < HEAP_MAX; i++) {
		if(VFSCORE_ipc_tab_heap[i].size != 0) {
//			uk_pr_crit("HEAP: %d: %p %x %x\n", i, VFSCORE_ipc_tab_heap[i].begin, VFSCORE_ipc_tab_heap[i].size, VFSCORE_ipc_tab_heap[i].mask);
			continue;
		}

		VFSCORE_ipc_tab_heap[i].begin = begin;
		VFSCORE_ipc_tab_heap[i].size = size;
		VFSCORE_ipc_tab_heap[i].mask = mask;

		if(i >= VFSCORE_ipc_tab_heap_nr_real)
			VFSCORE_ipc_tab_heap_nr_real = i;

		return;
	}

	uk_pr_crit("add more records to VFSCORE ipc struct\n");
	while(1);
}

void vfs_ipc_remove_heap(void *begin) {
//	uk_pr_crit("Removing %p\n", begin);
	for(int i = 0; i < VFSCORE_ipc_tab_heap_nr_real; i++) {
		if(VFSCORE_ipc_tab_heap[i].begin != begin)
			continue;

		VFSCORE_ipc_tab_heap[i].begin = 0;
		VFSCORE_ipc_tab_heap[i].size = 0;
		VFSCORE_ipc_tab_heap[i].mask = 0;

		return;
	}

	uk_pr_crit("Entry with begin %p doesn exist\n", begin);
//	while(1);
}

