#define _GNU_SOURCE
#include <linux/limits.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <linux/limits.h>
#include <link.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <sched.h> 

int get_module_id(char *name);
char *mem_to_lib(void *address);

#include "elf_hook.h"
#include "plthook.h"
#include "gen_hooks.h"

#include "mini-printf.h"

//#define DEBUG
//#define DOT
//#define NOMPK
//#define ALLOW_APP
//#define ALLOW_ALL

////////////////////////////////
Lmid_t lmid;

struct ipc_struct {
	void *begin;
	size_t size;
	unsigned long	mask;
};


char gpath[1024];

int pmap_max = E_END;
struct proc_map_s {
	char name[PATH_MAX];
	unsigned long start;
	unsigned long size;
	int pkey;
	char must;
//
	struct ipc_struct *ipc_tab_stack_ptr;
	struct ipc_struct *ipc_tab_globals_ptr;
	struct ipc_struct *ipc_tab_heap_ptr;
	int ipc_stack_nr;
	int ipc_globals_nr;
	int ipc_heap_nr;
} pmap[100];

#define SMAX	500
#define HMAX	2000
int stacks_max = 0;
int heaps_max = 0;
struct stack_map_s {
	char name[PATH_MAX];
	unsigned long start;
	unsigned long size;
	int mid;
} stacks[SMAX];

char *heap_cfg;
long heap_start;
long heap_end;

char *mmap_cfg;
long mmap_start;
long mmap_end;


int total_pkeys = 0;

void prot_init(int mid) {
#ifdef NOMPK
		return;
#endif

#ifdef DEBUG
	printf("PKEY %d: %lx + %lx\n", pmap[mid].pkey, pmap[mid].start, pmap[mid].size);
#endif
	int status = pkey_mprotect((void *)pmap[mid].start, pmap[mid].size , PROT_READ | PROT_WRITE | PROT_EXEC, pmap[mid].pkey);
	if (status == -1) {
		printf("pkey_mprotect failed\n");
		perror("");
		while(1);
	}
}

void cmp_lock(int mid) {
#ifdef NOMPK
		return;
#endif
	char buf[256];
#ifdef DEBUG
	mini_snprintf(buf, 256,"disabling %d: %s, key = %d \n", mid, pmap[mid].name, pmap[mid].pkey);
	write(0, buf,strlen(buf));
#endif
//	if(mid == 0)
//		return;

//	if(mid == E_UKBOOT)
//		return;


	if(!pmap[mid].pkey) {
		printf("somethin is wrong, there is no key\n");while(1);
		return;
	}

	int status = pkey_set(pmap[mid].pkey, PKEY_DISABLE_ACCESS);
	if (status) {
		printf("pkey_set error\n");while(1);
	}
}

void cmp_unlock(int mid) {
#ifdef NOMPK
		return;
#endif
	char buf[256];
#ifdef DEBUG
	mini_snprintf(buf, 256,"enabling %d: %s, key = %d \n", mid, pmap[mid].name, pmap[mid].pkey);
	write(0, buf,strlen(buf));
#endif

	int status = pkey_set(pmap[mid].pkey, 0);
	if (status) {
		printf("pkey_set error\n");while(1);
	}
}

void cmp_attach(int mid, void *start, size_t size) {
#ifdef NOMPK
		return;
#endif

	if(size == 0)
		return;

#ifdef DEBUG
	printf("CMP_ATTACH %d: %s, key = %d: %p, +%lx\n", mid, pmap[mid].name, pmap[mid].pkey, start, size);
#endif
	int status = pkey_mprotect(start, size , PROT_READ | PROT_WRITE | PROT_EXEC, pmap[mid].pkey);
	if (status == -1) {
		printf("pkey_mprotect failed\n");
		perror("CMP_ATTACH: ");
		while(1);
	}
}

void add_ipc_tab_stack(int mid, void *ipc_tab, void *nr) {
	int nrecords;
	memcpy(&nrecords, nr, sizeof(int));
	printf("add STACK ipc tab %d %p %d\n", mid, ipc_tab, nrecords);
	pmap[mid].ipc_tab_stack_ptr = ipc_tab;
	pmap[mid].ipc_stack_nr = nrecords;
}

void add_ipc_tab_globals(int mid, void *ipc_tab, void *nr) {
	int nrecords;
	memcpy(&nrecords, nr, sizeof(int));
	printf("add GLOBALS ipc tab %d %p %d\n", mid, ipc_tab, nrecords);
	pmap[mid].ipc_tab_globals_ptr = ipc_tab;
	pmap[mid].ipc_globals_nr = nrecords;
}

void add_ipc_tab_heap(int mid, void *ipc_tab, void *nr) {
	int nrecords;
	memcpy(&nrecords, nr, sizeof(int));
	printf("add HEAP ipc tab %d %p %d\n", mid, ipc_tab, nrecords);
	pmap[mid].ipc_tab_heap_ptr = ipc_tab;
	pmap[mid].ipc_heap_nr = nrecords;
}


void stack_attach(int mid, void *start, size_t size) {
	if(size == 0)
		return;
#ifdef DEBUG
	printf("STACK_ATTACH %d: %s, key = %d: %p, +%lx\n", mid, pmap[mid].name, pmap[mid].pkey, start, size);
#endif

	if(stacks_max == SMAX) {
		printf("not enough stacks\n");while(1);
	}

	stacks[stacks_max].start = start;
	stacks[stacks_max].size = size;
	stacks[stacks_max].mid = mid;
	strcpy(stacks[stacks_max].name, pmap[mid].name);

	cmp_attach(mid, start, size);
	stacks_max ++;
}

char *heap_cfg;
long heap_start;
long heap_end;


void heap_attach(int mid, long start, size_t size) {
	if(size == 0)
		return;
#ifdef DEBUG
	printf("HEAP_ATTACH %d: %s, key = %d: %p, +%lx\n", mid, pmap[mid].name, pmap[mid].pkey, start, size);
#endif

	if( !((heap_start <= start)  && (start < heap_end) ) ) {
		printf("it is not heap\n");while(1);
	}

	int idx = (start-heap_start) >> 12;
	for(int i = 0; i < size/4096; i++) {
		heap_cfg[idx+i] = mid;
	}

	cmp_attach(mid, start, size);

	return;
}

void gen_memory_map() {
	FILE *fp;
	char buf[PATH_MAX], lpath[PATH_MAX];
	char perms[5], dev[6];
	int bol = 1;

	char lines[1024][PATH_MAX];

	char wd[PATH_MAX];
	getcwd(wd, PATH_MAX);

	fp = fopen("/proc/self/maps", "r");
	if (fp == NULL) {
		printf("failed to open /proc/self/maps\n");
		return;
	    }

	int max_lines = 0;

	unsigned long start, end, flags, na;

	while (fgets(buf, PATH_MAX, fp) != NULL) {

		int eol = (strchr(buf, '\n') != NULL);
		if (bol) {
			if (!eol) {
				bol = 0;
			}
		} else {
			if (eol) {
				bol = 1;
			}
			continue;
		}

		if (sscanf(buf, "%lx-%lx %4s %lx %5s %ld %s", &start, &end, perms, &flags, dev, &na, lpath) != 7) {
			if (sscanf(buf, "%lx-%lx %4s %lx %5s %ld %s", &start, &end, perms, &flags, dev, &na) != 6)
				continue;
		}

		memcpy(lines[max_lines], buf, PATH_MAX);

		max_lines++;
	}

	fclose(fp);

	for(int i = 0; i < max_lines; i++)
		printf("%s",lines[i]);

//find beginning
	int i;
	for(i = 0; i < max_lines; i++) {
		if(!strstr(lines[i], "[heap]"))
			continue;

		break;
	}
	if(i == max_lines) {
		printf("cannot find heap\n");while(1);
	}

	int begin_l = i + 1;

//find end
	for(i = max_lines-1; i >= 0; i--) {
		if(!strstr(lines[i], wd))
			continue;

		break;
	}
	if(i == 0) {
		printf("cannot find end\n");while(1);
	}

	int end_l = i;
//printf("found: begin %s",lines[begin_l]);
//printf("found: end %s",lines[end_l]);
	sscanf(lines[begin_l], "%lx-%lx %4s %lx %5s %ld %s", &start, &end, perms, &flags, dev, &na);
	printf("begin = %lx\n", start);
	mmap_start = start;

	sscanf(lines[end_l], "%lx-%lx %4s %lx %5s %ld %s", &start, &end, perms, &flags, dev, &na, lpath);
	printf("end = %lx\n", end);
	mmap_end = end;

	mmap_cfg = malloc((mmap_end-mmap_start) >> 12);
	if(mmap_cfg == NULL) {
		printf("cannot allocate memory for mmap\n");while(1);
	}

	memset(mmap_cfg, -1, (mmap_end-mmap_start)>>12);
	for(long i = mmap_start; i < mmap_end; i+=4096) {
		int id = get_module_id(mem_to_lib((void *)i));
		if(id == -1)
			id = stack_to_lib_id_old(i) | (1 << 7);

		mmap_cfg[(i - mmap_start) >> 12] = id;
	}
}

void parse_proc(char *name, unsigned long *mstart, unsigned long *msize) {
	FILE *fp;
	char buf[PATH_MAX], lpath[PATH_MAX];
	char perms[5], dev[6];
	int bol = 1;

	char lines[1024][PATH_MAX];

	char wd[PATH_MAX];
	getcwd(wd, PATH_MAX);
	if(gpath[0]!='/')
		sprintf(&wd[strlen(wd)], "/%s/%s", gpath, name);
	else
		sprintf(wd, "%s/%s", gpath, name);

	fp = fopen("/proc/self/maps", "r");
	if (fp == NULL) {
		printf("failed to open /proc/self/maps\n");
		return;
	    }

	int max_lines = 0;

	unsigned long start, end, flags, na;

	while (fgets(buf, PATH_MAX, fp) != NULL) {

		int eol = (strchr(buf, '\n') != NULL);
		if (bol) {
			if (!eol) {
				bol = 0;
			}
		} else {
			if (eol) {
				bol = 1;
			}
			continue;
		}

		if (sscanf(buf, "%lx-%lx %4s %lx %5s %ld %s", &start, &end, perms, &flags, dev, &na, lpath) != 7) {
			if (sscanf(buf, "%lx-%lx %4s %lx %5s %ld %s", &start, &end, perms, &flags, dev, &na) != 6)
				continue;
		}

		memcpy(lines[max_lines], buf, PATH_MAX);

		max_lines++;
	}

	fclose(fp);

	int i;
	for(i = 0; i < max_lines; i++) {
		sscanf(lines[i], "%lx-%lx %4s %lx %5s %ld %s", &start, &end, perms, &flags, dev, &na, lpath);

		if(strcmp(lpath, wd))
			continue;

		break;
	}

	if(i == max_lines) {
		printf("%s -- not loaded\n", name);
		return;
	}
//	printf("i = %d/%d\n", i, max_lines);

	*mstart = 0;
	*msize = 0;

	do {
		lpath[0] = '\0';
		sscanf(lines[i++], "%lx-%lx %4s %lx %5s %ld %s", &start, &end, perms, &flags, dev, &na, lpath);

		if(lpath[0] == '[')
			break;

		if(strcmp(lpath, wd) && (na != 0)) {
//		if(strcmp(lpath, wd)) {
			break;
		}

		if(na == 0) {
//okay, here there is a problem: there are memory regions between modules and nobody knows which modules own them.
//usually, the following memory belongs to the previous module (it is .data section), but there is one exception.
//it seems that LIBC reserves 3 pages BEFORE its lowest address. 
//So, a module that comes before LIBC, may also have .data section, and we need to substrate 3K from its memory

			if(strstr(lines[i], "libc-")) {
				if(end < 0x3000) {
					printf("we are in a trouble\n");while(1);
				}
				end -= 0x3000;
			}
		}

		printf("\t%lx -- %lx --> %s\n", start, end, lpath);

		*msize += end - start;
		if(*mstart == 0)
			*mstart = start;
	} while(1);

}

char *mem_to_lib(void *address) {
	unsigned long addr = (unsigned long)address;

	for(int i = 0; i < sizeof(pmap)/sizeof(struct proc_map_s); i++) {
		if (pmap[i].start <= addr && addr < pmap[i].start+pmap[i].size)
			return pmap[i].name;
	}

	return NULL;
}

int mem_to_lib_id(long address) {
	if( !((mmap_start <= address)  && (address < mmap_end) ) )
		return -1;

	int mid = mmap_cfg[(address-mmap_start)>>12];
	if(mid & (1 << 7))
		return -1;
	else
		return mid;
}


char *stack_to_lib(long address) {
	if( !((mmap_start <= address)  && (address < mmap_end) ) )
		return NULL;

	int mid = mmap_cfg[(address-mmap_start)>>12];
	if(mid == -1)
		return NULL;
	if(mid & (1 << 7))
		return pmap[mid&0x7f].name;
	else
		return NULL;
}



int stack_to_lib_id(long address) {
	if( !((mmap_start <= address)  && (address < mmap_end) ) )
		return -1;

	int mid = mmap_cfg[(address-mmap_start)>>12];
	if(mid & (1 << 7))
		return mid&0x7f;
	else
		return -1;
}

int stack_to_lib_id_old(long address) {
	unsigned long addr = (unsigned long)address;
//	for(int i = stacks_max-1; i >= 0; i--) {
	for(int i = 0; i < stacks_max; i++) {
#ifdef DEBUG
//		printf("S2L [%d]: %lx <= %p < %lx\n", i, stacks[i].start, addr, stacks[i].start+stacks[i].size);
#endif
		if (stacks[i].start <= addr && addr < stacks[i].start+stacks[i].size) {
			return stacks[i].mid;
		}
	}

	return -1;
}


char *heap_to_lib(long address) {
	if( !((heap_start <= address)  && (address < heap_end) ) )
		return NULL;

	return pmap[heap_cfg[(address-heap_start)>>12]].name;
}


int heap_to_lib_id(long address) {
	if( !((heap_start <= address)  && (address < heap_end) ) )
		return -1;

	return heap_cfg[(address-heap_start)>>12];
}

int replacechar(char *str, char orig, char rep) {
	char *ix = str;
	int n = 0;
	while((ix = strchr(ix, orig)) != NULL) {
		*ix++ = rep;
		n++;
	}

	return n;
}

void patch_plt(int id) {
	char *filename=pmap[id].name;
#ifdef DEBUG
	printf("--------- [%d] %s --------- \n", id, filename);
#endif
#ifdef DOT
	char tmp[1024];
	sscanf(filename, "%s.so", tmp);
	tmp[strlen(tmp)-3]='\0';
	replacechar(tmp, '-','_');
	printf("%s [shape=record,label=\"{%s}\"];\n", tmp, tmp);
#endif
	plthook_t *plthook;
	unsigned int pos = 0; /* This must be initialized with zero. */
	char *name;
	void **addr;

	if (plthook_open(&plthook, filename) != 0) {
		printf("plthook_open error: %s\n", plthook_error());
		return;
	}

	while (plthook_enum(plthook, &pos, &name, &addr) == 0) {
#ifdef DEBUG
		printf("%p --> %p --> %s@", addr, *addr, name);
#endif
		char *lib  = mem_to_lib(*addr);
		if(lib == NULL) {

			Dl_info  DlInfo;
			int  nret;
			nret = dladdr(*addr, &DlInfo);
			if(DlInfo.dli_fbase != pmap[id].start) {
				printf("\n'%s' linked with loader, die\n", name);
				printf("nret = %d, %s, %p\n", nret, DlInfo.dli_fname, DlInfo.dli_fbase);
				printf("need something bettwe\n");
				while(1);
			}
			printf("%s, extending size\n", DlInfo.dli_fname);
			pmap[id].size = ((((unsigned long) (*addr) >> 12) + 1 ) << 12) - (unsigned long) pmap[id].start;
//			printf("this part is disabled to be sure that everything works\n");while(1);
			continue;
		}
#ifdef DEBUG
		printf("@%s -->", lib);
#endif
		if(!strcmp(filename, lib)) {
#ifdef DEBUG
			printf("skip, internal\n");
#endif
#ifdef DOT_SELF
			printf("%s -> %s [ label=\" %s \"];\n",tmp, tmp, name);
#endif
			continue;
		}

#ifdef DOT
		char tmp2[1024];
		sscanf(lib, "%s.so", tmp2);
		tmp2[strlen(tmp2)-3]='\0';
		replacechar(tmp2, '-','_');
#endif

		int ret = install_hook(addr, lib, name);
		switch(ret) {
			case 0:
#ifdef DEBUG
				printf("%p \n", *addr);
#endif
#ifdef DOT
				printf("%s -> %s [ color=green, label=\" %s \"];\n",tmp, tmp2, name);
#endif
				break;
			case 1:
#ifdef DEBUG
				printf("skip, shared\n");
#endif
#ifdef DOT
				printf("%s -> %s [ color=blue, label=\" %s \"];\n",tmp, tmp2, name);
#endif
				break;
			case 2:
#ifdef DEBUG
				printf("skip, but \x1B[31mMUST BE WRAPPED\x1B[37m \n");
#endif
#ifdef DOT
				printf("%s -> %s [ color=red, label=\" %s \"];\n",tmp, tmp2, name);
#endif
				break;
			default:
#ifdef DEBUG
				printf("unsupported");
#endif
				while(1);
		}
	}

	plthook_close(plthook);
}

void scan_modules() {
	DIR *fd;
	struct dirent *in_file;
	if ((fd = opendir (gpath)) == NULL)  {
		fprintf(stderr, "Error : Failed to open input directory - %s\n", strerror(errno)); while(1);
	}

	while ((in_file = readdir(fd)))  {
		if (!strcmp (in_file->d_name, "."))
			continue;
		if (!strcmp (in_file->d_name, ".."))
			continue;

		int mid = get_id(in_file->d_name);
		if(mid == -1)
			mid = pmap_max;

		strcpy(pmap[mid].name, in_file->d_name);
		pmap[mid].start = 0;
		pmap[mid].size = 0;
		pmap[mid].pkey = 0;
		pmap[mid].ipc_stack_nr = 0;
		pmap[mid].ipc_globals_nr = 0;
		pmap[mid].ipc_heap_nr = 0;
		pmap[mid].must = is_must(pmap[mid].name);
		if(pmap[mid].must) {
#ifndef NOMPK
			int pkey = pkey_alloc(0,0);
			if(pkey == -1) {
				printf("no more pkeys, die\n");while(1);
			}
			pmap[mid].pkey = pkey;
#endif
		}

		if(get_id(in_file->d_name) == -1)
			pmap_max++;
	}
}

void dump_modules() {
	printf("------- DUMP MODULES -----------\n");
	printf("id\t\t\tname\t[       start--         end]\tpkey\tipc_nr\tmust\n");
	for(int i = 0; i < pmap_max; i++) {
		if(pmap[i].name[0])
			printf("[%d]\t%20s\t[%lx--%lx]\t%d\t%d/%d/%d\t%d\n",i, pmap[i].name, pmap[i].start, pmap[i].start+pmap[i].size,pmap[i].pkey, pmap[i].ipc_stack_nr, pmap[i].ipc_globals_nr,pmap[i].ipc_heap_nr,pmap[i].must);
		else
			printf("[%d]\n", i);
	}
}

void dump_memory_map() {
	printf("------- DUMP MEMORY -----------\n");
	printf("id\t\t\tname\t[       start--         end]\tpkey\tipc_nr\tmust\n");
	for(int i = 0; i < (mmap_end-mmap_start) >> 12; i++) {
		if((mmap_cfg[i] != -1) && pmap[mmap_cfg[i]].name[0])
			printf("%lx\t%s\n", mmap_start + i*4096, pmap[mmap_cfg[i]]);
		else
			printf("%lx\n", mmap_start + i*4096);
	}
}


int get_module_id(char *name) {
	int i;
	if(name == NULL)
		return -1;

	for(i = 0; i < pmap_max; i++) {
		if(strcmp(pmap[i].name, name))
			continue;

		return i;
	}

	return -1;
}

char should_map(int caller, int callee, void *addr) {
#ifdef ALLOW_APP
	if( (caller == E_APP)  || (callee == E_APP))
		return 1;
#endif

#ifdef ALLOW_ALL
	return 1;
#endif

//workaround: callback tables in lwip/sockets
	if( (caller == E_VFSCORE)  || (callee == E_LWIP))
		return 1;


#ifdef DEBUG
	printf("%s(%d), %s(%d), %p\n", pmap[caller].name, caller, pmap[callee].name, callee, addr);
#endif

	if(!pmap[caller].must) {
		printf("impossible, the caller is not must\n");while(1);
	}

	if(pmap[callee].ipc_tab_globals_ptr == NULL) {
		printf("add GLOBALS (ipc.c) to %s\n", pmap[callee].name);while(1);
	}


//if it is not E_APP, then we tolerate access only to globals
//	cmp_attach(26, pmap[callee].ipc_tab_globals_ptr, (((pmap[callee].ipc_globals_nr * sizeof(struct ipc_struct)) >> 12) + 1 ) << 12 );
	cmp_unlock(callee);
	for(int i = 0; i < pmap[callee].ipc_globals_nr; i++) {
		struct ipc_struct *ipc = (struct ipc_struct *) ((unsigned long) pmap[callee].ipc_tab_globals_ptr + (unsigned long)i*sizeof(struct ipc_struct));
		if(!(ipc->mask & (1 << caller)))
			continue;

#ifdef DEBUG
		printf("Try GLOBALS [%d]: %lx <= %p < %lx\n", i, ipc->begin, addr, (ipc->begin+ipc->size));
#endif

		if( (ipc->begin <= addr) && (addr < (ipc->begin+ipc->size)) ) {
//			cmp_attach(callee, pmap[callee].ipc_tab_globals_ptr, (((pmap[callee].ipc_globals_nr * sizeof(struct ipc_struct)) >> 12) + 1 ) << 12 );
			cmp_lock(callee);
			return 1;
		}
	}

	return 0;
}

char should_map_stack(int caller, int callee, void *addr) {
#ifdef ALLOW_ALL
	return 1;
#endif

#ifdef ALLOW_APP
	if( (caller == E_APP)  || (callee == E_APP))
		return 1;
#endif

//get my stack back
	if( caller == callee)
		return 1;

#ifdef DEBUG
	printf("%s(%d), %s(%d), %p\n", pmap[caller].name, caller, pmap[callee].name, callee, addr);
#endif

	if(!pmap[caller].must) {
		printf("impossible, the caller is not must\n");while(1);
	}

	if(pmap[callee].ipc_tab_stack_ptr == NULL) {
		printf("add ipc.c to %s\n", pmap[callee].name);while(1);
	}

//todo: I use 26, which is debug, but in fact I need key=0
//	cmp_attach(26, pmap[callee].ipc_tab_stack_ptr, (((pmap[callee].ipc_stack_nr * sizeof(struct ipc_struct)) >> 12) + 1 ) << 12 );
	cmp_unlock(callee);
	for(int i = 0; i < pmap[callee].ipc_stack_nr; i++) {
		struct ipc_struct *ipc = (struct ipc_struct *) ((unsigned long) pmap[callee].ipc_tab_stack_ptr + (unsigned long)i*sizeof(struct ipc_struct));
//		printf("[%d] ipc = %p, mask = %x\n", i, ipc, ipc->mask);
		if(!(ipc->mask & (1 << caller)))
			continue;

#ifdef DEBUG
		printf("TRY STACK [%d] %lx <= %p < %lx\n", i, ipc->begin, addr, (ipc->begin+ipc->size));
#endif

		if( (ipc->begin <= addr) && (addr < (ipc->begin+ipc->size)) ) {
//			cmp_attach(callee, pmap[callee].ipc_tab_stack_ptr, (((pmap[callee].ipc_stack_nr * sizeof(struct ipc_struct)) >> 12) + 1 ) << 12 );
			cmp_lock(callee);
			return 1;
		}

		if( (ipc->begin <= addr) && (addr < (ipc->begin+4096)) ) {
//looks like a stack page can be derived, but later, it can be reused by another function, but since the page 
// belongs to another module, and I cannot find the pointer inside the epc_table, I receive segfault.
//			printf("WARNING, probably bug: the pointer is outside of the object but within its page.\n");
//			cmp_attach(callee, pmap[callee].ipc_tab_stack_ptr, (((pmap[callee].ipc_stack_nr * sizeof(struct ipc_struct)) >> 12) + 1 ) << 12 );
			cmp_lock(callee);
			return 1;
		}

	}

	return 0;
}

char should_map_heap(int caller, int callee, void *addr) {
#ifdef ALLOW_APP
	if( (caller == E_APP)  || (callee == E_APP))
		return 1;
#endif

#ifdef ALLOW_ALL
	return 1;
#endif


//always get my memory back
	if(callee == caller)
		return 1;

//remap friends 
	if(pmap[callee].pkey == pmap[caller].pkey)
		return 1;

/***** workarounds ***/
	if(callee==E_APP && caller==E_LINUXUPLAT)
		return 1;

//workaround
//	if(caller==E_VFSCORE && callee == E_UKNETDEV)
//		return 1;
//workaround
//this is a pbuff created by LWIP on behalf of VFSCORE 
//	if(callee==E_VFSCORE && caller == E_UKNETDEV)
//		return 1;



#ifdef DEBUG
	printf("%s(%d), %s(%d), %p\n", pmap[caller].name, caller, pmap[callee].name, callee, addr);
#endif

	if(!pmap[caller].must) {
		printf("impossible, the caller is not must\n");while(1);
	}

	if(pmap[callee].ipc_tab_heap_ptr == NULL) {
		printf("add ipc.c to %s\n", pmap[callee].name);while(1);
	}

//	cmp_attach(26, pmap[callee].ipc_tab_heap_ptr, (((pmap[callee].ipc_heap_nr * sizeof(struct ipc_struct)) >> 12) + 1 ) << 12 );
	cmp_unlock(callee);
	for(int i = 0; i < pmap[callee].ipc_heap_nr; i++) {
		struct ipc_struct *ipc = (struct ipc_struct *) ((unsigned long) pmap[callee].ipc_tab_heap_ptr + (unsigned long)i*sizeof(struct ipc_struct));
//		printf("[%d] ipc = %p\n", i, ipc);
		if(!(ipc->mask & (1 << caller)))
			continue;

#ifdef DEBUG
		printf("TRY HEAP [%d] %lx <= %p < %lx\n", i, ipc->begin, addr, (ipc->begin+ipc->size));
#endif

		if( (ipc->begin <= addr) && (addr < (ipc->begin+ipc->size)) ) {
//			cmp_attach(callee, pmap[callee].ipc_tab_heap_ptr, (((pmap[callee].ipc_heap_nr * sizeof(struct ipc_struct)) >> 12) + 1 ) << 12 );
			cmp_lock(callee);
			return 1;
		}

		if( (ipc->begin <= addr) && (addr < (ipc->begin+4096)) ) {
//looks like a stack page can be derived, but later, it can be reused by another function, but since the page 
// belongs to another module, and I cannot find the pointer inside the epc_table, I receive segfault.
			printf("WARNING, probably bug: the pointer %p is outside of the object (%x) but within its page,\n", ipc->begin, ipc->size);
//			cmp_attach(callee, pmap[callee].ipc_tab_heap_ptr, (((pmap[callee].ipc_heap_nr * sizeof(struct ipc_struct)) >> 12) + 1 ) << 12 );
			cmp_lock(callee);
			return 1;
		}

	}

	return 0;
}

enum state {
	RET_STACK_TO_WRAP,
	ACCESS_TO_GLOBAL,
	ACCESS_TO_STACK,
	ACCESS_TO_HEAP,
};

enum state get_state(long rip, long rsp, long addr, int *caller, int *callee) {
	int rip_lib = mem_to_lib_id(rip);

	if(rip_lib == -1) {
		*caller = stack_to_lib_id(rsp);
		*callee = stack_to_lib_id(addr);
		return RET_STACK_TO_WRAP;
	}

	int si_lib = mem_to_lib_id(addr);

	if(si_lib != -1) {
		*caller = rip_lib;
//the caller is not a 'must' module, we need to get from the stack
		if(pmap[*caller].must==0)
			*caller = stack_to_lib_id(rsp);
		*callee = si_lib;

		return ACCESS_TO_GLOBAL;
	}

	int si_stack = stack_to_lib_id(addr);

	if(si_stack != -1) {
		*caller = rip_lib;
		if(pmap[*caller].must==0)
			*caller = stack_to_lib_id(rsp);
		*callee = si_stack;

		return ACCESS_TO_STACK;
	}

	int si_heap = heap_to_lib_id(addr);

	if(si_heap != -1 ) {
		*caller = rip_lib;
		if(pmap[*caller].must==0)
			*caller = stack_to_lib_id(rsp);
		*callee = si_heap;

		return ACCESS_TO_HEAP;
	}

	printf("wrong state\n");while(1);
}

//void sig_handler(int signo) {
void sig_handler(int j, siginfo_t *si, void *uap) {
	mcontext_t *mctx = &((ucontext_t *)uap)->uc_mcontext;
	greg_t *rsp = &mctx->gregs[REG_RSP];
	greg_t *rip = &mctx->gregs[REG_RIP];
	char buf[256];
//#if 1
#ifdef DEBUG
	mini_snprintf(buf, 256,"SIGSegv PKEY = %d, CODE: %d\n", si->si_pkey, si->si_code);
	write(0, buf,strlen(buf));
	mini_snprintf(buf, 256,"SIGSEGV:\t\trip\t\trsp\t\tsi_addr\n");
	write(0, buf,strlen(buf));
	mini_snprintf(buf, 256,"Value:\t\t%x%x\t%x%x\t%x%x\n", (*rip)>>32,*rip, (*rsp)>>32,*rsp, ((unsigned long)si->si_addr)>>32,si->si_addr);
	write(0, buf,strlen(buf));
	mini_snprintf(buf, 256,"lib:\t\t%s\t%s\t%s\n", mem_to_lib(*rip)?mem_to_lib(*rip):"    (null)    ", mem_to_lib(*rsp)?mem_to_lib(*rsp):"    (null)    ", mem_to_lib(si->si_addr)?mem_to_lib(si->si_addr):"    (null)    ");
	write(0, buf,strlen(buf));
	mini_snprintf(buf, 256,"stack:\t\t%s\t%s\t%s\n", stack_to_lib(*rip)?stack_to_lib(*rip):"    (null)    ", stack_to_lib(*rsp)?stack_to_lib(*rsp):"    (null)    ", stack_to_lib(si->si_addr)?stack_to_lib(si->si_addr):"    (null)    ");
	write(0, buf,strlen(buf));
	if(heap_cfg) {
		mini_snprintf(buf, 256,"heap:\t\t%s\t%s\t%s\n", heap_to_lib(*rip)?heap_to_lib(*rip):"    (null)    ", heap_to_lib(*rsp)?heap_to_lib(*rsp):"    (null)    ", heap_to_lib(si->si_addr)?heap_to_lib(si->si_addr):"    (null)    ");
		write(0, buf,strlen(buf));
	}
#endif


	int caller;
	int callee;

	switch(get_state(*rip, *rsp, (long) si->si_addr, &caller, &callee)) {
		case RET_STACK_TO_WRAP:
//			if(caller != E_APP) {
//				printf("[%d] we tolerate only accesses to APP's stack, caller = %d\n", __LINE__, caller);
//				while(1);
//			}

			if( caller == callee ) {
				cmp_attach(caller, (void *) ( (( (unsigned long) si->si_addr) >> 12 ) << 12), 4096);
				break;
			}

			printf("[%d] cannot remap LOADER's stack\n", __LINE__);
			while(1);
		case ACCESS_TO_GLOBAL:
			if(should_map(caller, callee, si->si_addr)) {
				cmp_attach(caller, (void *) ( (( (unsigned long) si->si_addr) >> 12 ) << 12), 4096);
				break;
			}

			printf("[%d] cannot map global\n", __LINE__);
			while(1);
		case ACCESS_TO_STACK:
			if(should_map_stack(caller, callee, si->si_addr)) {
				cmp_attach(caller, (void *) ( (( (unsigned long) si->si_addr) >> 12 ) << 12), 4096);
				break;
			}

			printf("[%d] cannot map stack\n", __LINE__);

	mini_snprintf(buf, 256,"SIGSegv PKEY = %d, CODE: %d\n", si->si_pkey, si->si_code);
	write(0, buf,strlen(buf));
	mini_snprintf(buf, 256,"SIGSEGV:\t\trip\t\trsp\t\tsi_addr\n");
	write(0, buf,strlen(buf));
	mini_snprintf(buf, 256,"Value:\t\t%x%x\t%x%x\t%x%x\n", (*rip)>>32,*rip, (*rsp)>>32,*rsp, ((unsigned long)si->si_addr)>>32,si->si_addr);
	write(0, buf,strlen(buf));
	mini_snprintf(buf, 256,"lib:\t\t%s\t%s\t%s\n", mem_to_lib(*rip)?mem_to_lib(*rip):"    (null)    ", mem_to_lib(*rsp)?mem_to_lib(*rsp):"    (null)    ", mem_to_lib(si->si_addr)?mem_to_lib(si->si_addr):"    (null)    ");
	write(0, buf,strlen(buf));
	mini_snprintf(buf, 256,"stack:\t\t%s\t%s\t%s\n", stack_to_lib(*rip)?stack_to_lib(*rip):"    (null)    ", stack_to_lib(*rsp)?stack_to_lib(*rsp):"    (null)    ", stack_to_lib(si->si_addr)?stack_to_lib(si->si_addr):"    (null)    ");
	write(0, buf,strlen(buf));
	if(heap_cfg) {
		mini_snprintf(buf, 256,"heap:\t\t%s\t%s\t%s\n", heap_to_lib(*rip)?heap_to_lib(*rip):"    (null)    ", heap_to_lib(*rsp)?heap_to_lib(*rsp):"    (null)    ", heap_to_lib(si->si_addr)?heap_to_lib(si->si_addr):"    (null)    ");
		write(0, buf,strlen(buf));
	}



			while(1);

		case ACCESS_TO_HEAP:
			if(should_map_heap(caller, callee, si->si_addr)) {
				cmp_attach(caller, (void *) ( (( (unsigned long) si->si_addr) >> 12 ) << 12), 4096);
				break;
			}
			printf("[%d] cannot map heap\n", __LINE__);


	mini_snprintf(buf, 256,"SIGSegv PKEY = %d, CODE: %d\n", si->si_pkey, si->si_code);
	write(0, buf,strlen(buf));
	mini_snprintf(buf, 256,"SIGSEGV:\t\trip\t\trsp\t\tsi_addr\n");
	write(0, buf,strlen(buf));
	mini_snprintf(buf, 256,"Value:\t\t%x%x\t%x%x\t%x%x\n", (*rip)>>32,*rip, (*rsp)>>32,*rsp, ((unsigned long)si->si_addr)>>32,si->si_addr);
	write(0, buf,strlen(buf));
	mini_snprintf(buf, 256,"lib:\t\t%s\t%s\t%s\n", mem_to_lib(*rip)?mem_to_lib(*rip):"    (null)    ", mem_to_lib(*rsp)?mem_to_lib(*rsp):"    (null)    ", mem_to_lib(si->si_addr)?mem_to_lib(si->si_addr):"    (null)    ");
	write(0, buf,strlen(buf));
	mini_snprintf(buf, 256,"stack:\t\t%s\t%s\t%s\n", stack_to_lib(*rip)?stack_to_lib(*rip):"    (null)    ", stack_to_lib(*rsp)?stack_to_lib(*rsp):"    (null)    ", stack_to_lib(si->si_addr)?stack_to_lib(si->si_addr):"    (null)    ");
	write(0, buf,strlen(buf));
	if(heap_cfg) {
		mini_snprintf(buf, 256,"heap:\t\t%s\t%s\t%s\n", heap_to_lib(*rip)?heap_to_lib(*rip):"    (null)    ", heap_to_lib(*rsp)?heap_to_lib(*rsp):"    (null)    ", heap_to_lib(si->si_addr)?heap_to_lib(si->si_addr):"    (null)    ");
		write(0, buf,strlen(buf));
	}


			while(1);
	}
}

void setup_sig() {
	stack_t sigstack;
	struct sigaction sa;

	int stsize = SIGSTKSZ*100;

	sigstack.ss_sp = malloc(stsize);
	if(sigstack.ss_sp == NULL) {
		perror("malloc");
	}

	sigstack.ss_size = stsize;
	sigstack.ss_flags = 0;
	if(sigaltstack(&sigstack, NULL) == -1) {
		perror("sigstack");
		exit(1);
	}

#ifdef DEBUG
	printf("%d Alternate stack is at %10p-%p\n", stsize, sigstack.ss_sp,sigstack.ss_sp+stsize);
#endif

//	sa.sa_handler = sig_handler;
	sa.sa_sigaction = sig_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_ONSTACK | SA_SIGINFO;

	if(sigaction(SIGSEGV, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}
}

void setup_heap(int a, struct ukplat_memregion_desc *desc) {
	printf("heap region %d: %p -- %p, flags %x\n",a, desc->base, desc->base + desc->len, desc->flags);
	heap_cfg = malloc(desc->len);
	memset(heap_cfg, E_UKALLOC, desc->len);
	heap_start = (long) desc->base;
	heap_end = (long) desc->base + (long) desc->len;
}

static inline unsigned int
pkey_read (void) {
unsigned int result;
__asm__ volatile (".byte 0x0f, 0x01, 0xee"
	: "=a" (result) : "c" (0) : "rdx");
	return result;
}

void *t1_f() {
	for(int i = 0; i < 16; i++)
		pkey_set(i, 0);

	printf("pkey = %x\n", pkey_read());

	char app[PATH_MAX];

	memset(app, 0, PATH_MAX);
	snprintf(app, PATH_MAX, "%s/libuknetdev.so","nginx");
	void *handle2 = dlopen(app, RTLD_NOLOAD | RTLD_NOW);
	if (!handle2) {
		fprintf(stderr, "DLOPEN %s\n", dlerror());
		while(1);
	}

	int (*time_init_ptr)(void);
	time_init_ptr = (int (*)(void)) dlsym(handle2, "pull_rx");
	if(time_init_ptr == NULL) {
		printf("cannot open func\n");while(1);
	}
	while(1) {
//		int ret = (time_init_ptr)();
	}
}


int main(int argc, char *argv[]) {
	if(argc < 2) {
		fprintf(stderr, "Usage: %s mods_dir \n", argv[0]);
		exit(1);
	}


#if 0
	void *dl_hd = dlopen("./mylibdl.so", RTLD_NOW | RTLD_DEEPBIND | RTLD_LOCAL);
	if(!dl_hd) {
		fprintf(stderr, "dl_hd: %s\n", dlerror());	while(1);
	};
	long (*dlopen)(char *, int);
#if 0
	long dlopen_ptr = (long (*)(char *,int)) dlsym(dl_hd, "dlopen");
	if(dlopen_ptr == NULL) {
		fprintf(stderr, "%s\n", dlerror());	while(1);
	}
#endif
#endif



	strcpy(gpath, argv[1]);
	scan_modules();

	char app[PATH_MAX];
	snprintf(app, PATH_MAX, "%s/app.so",argv[1]);

	void *handle = dlopen(app, RTLD_NOW | RTLD_DEEPBIND | RTLD_LOCAL);
//	void *handle = dlmopen(LM_ID_NEWLM, app, RTLD_NOW | RTLD_LOCAL|RTLD_DEEPBIND);
	if (!handle) {
		fprintf(stderr, "%s: %s\n", app, dlerror());
		exit(1);
	}

	dlinfo(handle, RTLD_DI_LMID, &lmid);
	create_cubicles(argv[1]);
	for(int i =0; i < pmap_max; i++) {
		parse_proc(pmap[i].name, &pmap[i].start, &pmap[i].size);
		printf("'%s': %lx, +%lx\n", pmap[i].name, pmap[i].start, pmap[i].size);
	}

#ifdef DOT
	printf("digraph \"Call graph\" {\n");
#endif

	setup_sig();

#if SQLITE_FINAL
//SQLite
//C3
#if SQLITE_3
	pmap[E_RAMFS].pkey = pmap[E_VFSCORE].pkey;
#endif
	pmap[E_UKALLOC].pkey = pmap[E_VFSCORE].pkey;
	pmap[E_UKBOOT].pkey = pmap[E_VFSCORE].pkey;
#endif

#ifdef NGINX_FINAL
	pmap[E_UKALLOC].pkey = pmap[E_APP].pkey;
#endif

	for(int i =0; i < pmap_max; i++) {
		if(pmap[i].size) {
			patch_plt(i);

			if(pmap[i].must) {
				prot_init(i);
			}
		}
	}

	for(int i =0; i < pmap_max; i++) {
		if(pmap[i].must) {
			cmp_lock(i);
		}
	}

#ifdef DOT
	printf("}\n");
	while(1);
#endif

	gen_memory_map();

//buggy with -O2
//	dump_memory_map();

	dump_modules();
	printf("------------ loading done -----------\n");

	cmp_unlock(get_module_id("liblinuxuplat.so"));

printf("------------------------- test\n");

#if 0
	pthread_t t1;
	if(pthread_create(&t1, NULL, t1_f, NULL)) {
		printf("perror()\n");while(1);
	}
#endif



	memset(app, 0, PATH_MAX);
	snprintf(app, PATH_MAX, "%s/liblinuxuplat.so",argv[1]);
	void *handle2 = dlopen(app, RTLD_NOLOAD | RTLD_NOW);
	if (!handle2) {
		fprintf(stderr, "%s\n", dlerror());
		exit(1);
	}

	int (*main_ptr)(int, char **);
	main_ptr = (int (*)(int, char **)) dlsym(handle2, "_liblinuxuplat_entry");
	(*main_ptr)(argc-1,&argv[1]);

	return 0;
}
