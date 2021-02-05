#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/param.h>

#include <vfscore/dentry.h>
#include <vfscore/vnode.h>

#include <uk/essentials.h>

extern unsigned char *root_cpio;

//stack
struct ipc_struct APP_ipc_tab_stack[] __attribute__((aligned(4096))) = {
//empty
	{0, 0, (1 << 21) | ( 1 << 0)},
	{1, 0, (1 << 21) | ( 1 << 0)},
	{2, 0, (1 << 21) | ( 1 << 0)},
//libc fstat
	{3, 0, (1 << 21) | ( 1 << 0)},
//console
	{4, 0, (1 << 21) | ( 1 << 0)},
//path in cpio
	{5, 4096, (1 << 21) | ( 1 << 0)},
//ngx_time_update(void)
	{6, 0, (1 << 21) | ( 1 << 0)},
//nginx posix_alloc
	{7, 0, (1 << 6) | ( 1 << 0)},
//ngx_os_specific_init(ngx_log_t *log)
	{8, 0, (1 << 3) | ( 1 << 0)},
//ngx_init_cycle(ngx_cycle_t *old_cycle)
	{9, 0, (1 << 3) | ( 1 << 0)},
//ngx_conf_parse(ngx_conf_t *cf, ngx_str_t *filename)
	{10, 0, (1 << 3) | ( 1 << 0)},
//ngx_create_paths(ngx_cycle_t *cycle, ngx_uid_t user)
	{11, 0, (1 << 3) | ( 1 << 0)},
//ngx_open_listening_sockets(ngx_cycle_t *cycle)
	{12, 0, (1 << 3) | ( 1 << 0)},
//ngx_create_pidfile(ngx_str_t *name, ngx_log_t *log)
	{13, 0, (1 << 3) | ( 1 << 0)},
//ngx_http_userid_init_worker(ngx_cycle_t *cycle)
	{14, 0, (1 << 3) | ( 1 << 0)},
//ngx_event_accept(ngx_event_t *ev)
	{15, 0, (1 << 17) | ( 1 << 0)},
//ngx_open_and_stat_file(ngx_str_t *name, ngx_open_file_info_t *of,
	{16, 0, (1 << 17) | ( 1 << 0)},
//
	{17, 0, (1 << 17) | ( 1 << 0)},
//ngx_select_process_events(ngx_cycle_t *cycle, ngx_msec_t timer,
	{18, 0, (1 << 17) | ( 1 << 0)},
//ngx_writev(ngx_connection_t *c, ngx_iovec_t *vec)
	{19, 0, (1 << 0 ) | (1 << 17) | (1 << 21) },
};
int APP_ipc_tab_stack_nr = sizeof(APP_ipc_tab_stack)/sizeof(struct ipc_struct);

//globals
struct ipc_struct APP_ipc_tab_globals[] __attribute__((aligned(4096))) = {
//cpio
	{0, 8192, (1 << 21) | ( 1 << 0)},
//ngx_select_module.c
	{1, 0, (1 << 17) | ( 1 << 0)},
//ngx_select_module.c
	{2, 0, (1 << 17) | ( 1 << 0)},
};
int APP_ipc_tab_globals_nr = sizeof(APP_ipc_tab_globals)/sizeof(struct ipc_struct);

//heap
#define HEAP_MAX	100
struct ipc_struct APP_ipc_tab_heap[HEAP_MAX] __attribute__((aligned(4096))) = {
};
int APP_ipc_tab_heap_nr = HEAP_MAX;


void app_ipc_remove_heap(void *begin) {
//	uk_pr_crit("REMOVE %p  \n", begin);
	for(int i = 0; i < HEAP_MAX; i++) {
		if(APP_ipc_tab_heap[i].begin != begin)
			continue;

		APP_ipc_tab_heap[i].begin = 0;
		APP_ipc_tab_heap[i].size = 0;
		APP_ipc_tab_heap[i].mask = 0;

		return;
	}

//	uk_pr_crit("Entry with begin %p doesn exist\n", begin);
//	while(1);
}

void app_ipc_add_heap(void *begin, size_t size, int mask) {
//	uk_pr_crit("Add %p %x %x \n", begin, size, mask);
	for(int i = 0; i < HEAP_MAX; i++) {
		if(APP_ipc_tab_heap[i].size != 0)
			continue;

		APP_ipc_tab_heap[i].begin = begin;
		APP_ipc_tab_heap[i].size = size;
		APP_ipc_tab_heap[i].mask = mask;

		return;
	}

	uk_pr_crit("add more records to the APP ipc struct\n");

	for(int i = 0; i < HEAP_MAX; i++) {
		printf("[%d] %p %x %x\n", i, APP_ipc_tab_heap[i].begin, APP_ipc_tab_heap[i].size, APP_ipc_tab_heap[i].mask);
	}
	while(1);
}

void app_ipc_add_stack(int i, void *begin, size_t size, int mask) {
//		uk_pr_crit("Stack Add [%d] %p %x %x \n", i, begin, size, mask);
		APP_ipc_tab_stack[i].begin = begin;
		APP_ipc_tab_stack[i].size = size;
		APP_ipc_tab_stack[i].mask = mask;
}

void app_ipc_add_global(int i, void *begin, size_t size, int mask) {
//		uk_pr_crit("Global Add [%d] %p %x %x \n", i, begin, size, mask);
		APP_ipc_tab_globals[i].begin = begin;
		APP_ipc_tab_globals[i].size = size;
		APP_ipc_tab_globals[i].mask = mask;
}
