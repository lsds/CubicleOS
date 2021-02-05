#include "dlmemreq.h"

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/param.h>

#include <vfscore/dentry.h>
#include <vfscore/vnode.h>

#include <uk/essentials.h>


//#define DEBUG

void ramfs_abort(void) {
	uk_pr_crit("abort has been called\n");while(1);
}

void *__expand_heap(size_t *pn)
{
    uk_pr_crit("expand_heap is not supported\n");
    while(1);
}

static void *ch_start = NULL;
static long ch_ctr = 0;
static long ch_size = (CONFIG_LINUXU_DEFAULT_HEAPMB*1024*1024LL/2);
//static long ch_size = (524*1024*1024LL);
//static long ch_size = (4096*50);

char* get_memory_ramfs(unsigned num_bytes) {
#ifdef DEBUG
    uk_pr_crit("get_mem RAMFS: %p/%x/%x/%lx\n", ch_start, ch_ctr, ch_size, num_bytes);
#endif
	if(ch_start == NULL) {
//		uk_pr_crit("allocating %x bytes for RaMFS\n", ch_size);
		ch_start = malloc(ch_size);
		if(ch_start == NULL) {
			uk_pr_crit("cannot allocate memory (%ld) for RAMFS \n", ch_size);while(1);
		}
		memset(ch_start, 0, ch_size);
		uk_pr_crit("RaMFS HEAP: %p -- %p (%x)\n", ch_start, ch_start + ch_size, ch_size);
		ch_ctr = 0;
	}

	if(ch_ctr + num_bytes > ch_size) {
		uk_pr_crit("OOM, die\n");
		while(1);
	}

	char *ret = (char *) (ch_start + ch_ctr);
	ch_ctr += num_bytes;

	return ret;
}
