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

void abort(void) {
	uk_pr_crit("abort has been called\n");while(1);
}

void *__expand_heap(size_t *pn)
{
    uk_pr_crit("expand_heap is not supported\n");
    while(1);
}

static void *ch_start = NULL;
static long ch_ctr = 0;
//static long ch_size = (CONFIG_LINUXU_DEFAULT_HEAPMB*1024*1024/100);
static long ch_size = (10LL*1024*1024);

char* get_memory(unsigned num_bytes) {
#ifdef DEBUG
    uk_pr_crit("get_mem: %p/%x/%x/%lx\n", ch_start, ch_ctr, ch_size, num_bytes);
#endif
	if(ch_start == NULL) {
		uk_pr_crit("allocating %x bytes for VFScoRE\n", ch_size);
		ch_start = malloc(ch_size);
		if(ch_start == NULL) {
			uk_pr_crit("cannot allocate memory for VFSCORE \n");while(1);
		}
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