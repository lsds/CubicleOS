#!/usr/bin/env python3
import argparse
import os
import re
import ntpath
import glob

def gen_pre_wrap(module, name):
  if 'ukalloc'==module and name=='uk_free_ifpages':
    ret="""\
		if(arg1 != 0) {
		struct metadata_ifpages *tmp = NULL;
		if(arg1 & 0xfff)
			tmp = (struct metadata_ifpages *) ( (unsigned long) arg1 - sizeof(struct metadata_ifpages));
		else
			tmp = (struct metadata_ifpages *) ( (unsigned long) arg1 - 4096 );

		cmp_attach(callee, tmp, 4096);
		heap_attach(callee, tmp->base, tmp->num_pages*4096);
#if WDEBUG
		printf(\"PRE-"""+name.upper()+""": %p %x\\n\", tmp->base, tmp->num_pages);
#endif
	}
"""
    return ret

  if 'ukalloc'==module and name=='uk_realloc_ifpages':
    ret="""\
#if 1
	if(arg1 != 0) {
		struct metadata_ifpages *tmp = (struct metadata_ifpages *) ( arg1 - sizeof(struct metadata_ifpages));
		cmp_attach(callee, tmp, 4096);
		heap_attach(callee, tmp->base, tmp->num_pages*4096);
#if WDEBUG
		printf(\"PRE-"""+name.upper()+""": %p %x\\n", tmp->base, tmp->num_pages);
#endif
	}
#endif
"""
    return ret

  return ""


def gen_post_wrap(module, name):
  if 'linuxuplat'==module and name=='uk_sched_thread_create':
    ret="""\
	if(ret != 0) {
		struct uk_thread *tmp = (struct uk_thread *) ret;
//		printf("create Thread %s with stack %x\\n", tmp->name, tmp->stack);
	}
"""
    return ret
  if 'ukalloc'==module and name=='uk_posix_memalign_ifpages':
    ret="""\
	if(ret == 0) {
		unsigned long **tt = (long **)(arg1);
//		cmp_attach(remap[prev_module[layer-1]], tt, 4096);
		unsigned long page_begin = ((unsigned long) (*tt) >> 12 ) << 12;
		struct metadata_ifpages *tmp;
		char fix = 0;
		if(page_begin == (long) (*tt)) {
		    tmp = (struct metadata_ifpages *) ( page_begin - 4096 );
		    fix = 1;
		} else
		    tmp = (struct metadata_ifpages *) ( page_begin );
		cmp_attach(remap[prev_module[layer-1]], tmp, 4096);
#if WDEBUG
		printf(\""""+name.upper()+""": %p %p, %p %x, %x %x \\n", *tt, tmp, tmp->base, tmp->num_pages, arg2, arg3);
#endif
		heap_attach(remap[prev_module[layer-1]], tmp->base, (tmp->num_pages - fix) * 4096);
	}

"""
    return ret

  if 'ukalloc'==module and (name=='uk_malloc_ifpages' or name=='uk_calloc_compat'):
    ret="""\
	if(ret != 0) {
		struct metadata_ifpages *tmp = (struct metadata_ifpages *) ( ret - sizeof(struct metadata_ifpages));
		cmp_attach(remap[prev_module[layer-1]], tmp, 4096);
#if WDEBUG
		printf(\""""+name.upper()+""": %p %x\\n\", tmp->base, tmp->num_pages);
#endif
		heap_attach(remap[prev_module[layer-1]], tmp->base, tmp->num_pages * 4096);
	}
"""
    return ret

  if 'ukalloc'==module and name=='uk_realloc_ifpages':
    ret="""\
#if 1
	if(ret != 0) {
		struct metadata_ifpages *tmp = (struct metadata_ifpages *) ( ret - sizeof(struct metadata_ifpages));
		heap_attach(remap[prev_module[layer-1]], tmp, 4096);
#if WDEBUG
		printf("REALLOCATED: %p %x\\n", tmp->base, tmp->num_pages);
#endif
		heap_attach(remap[prev_module[layer-1]], tmp->base, tmp->num_pages * 4096);
	}
#endif
"""
    return ret

  if 'linuxuplat'==module and name=='ukplat_memregion_get':
    ret="""\
	if(ret != -1) {
		cmp_attach(remap[prev_module[layer-1]], arg1, 4096);
		setup_heap(arg0, arg1);
	}
"""
    return ret

  return ""

def change_enum(module, name):
  if 'main'==name and module=='ukboot':
    return "app"

  return module


def change_module(module, name):
  if 'main'==name and module=='ukboot':
    return "app.so"

  return 'lib'+module+'.so'


def get_hook(module, name):
  if 'ukplat_lcpu_restore_irqf'==name and module=='linuxuplat':
    ret="""\
		return 0;
"""
    return ret

  if 'ukplat_lcpu_save_irqf'==name and module=='linuxuplat':
    ret="""\
		return 0;
"""
    return ret

  if 'dlsym2'==name and module=='posix_libdl':
    ret="""\
		return dlsym(arg0, arg1);
"""
    return ret

  if 'dlopen2'==name and module=='posix_libdl':
    ret="""\
	if(arg0 == NULL) {
		return dlopen("python3/app.so", RTLD_NOLOAD | RTLD_NOW);
	}

	char path[4096];

	sprintf(path,"../app-python3/rootfs%s",arg0);
	return dlopen(path, RTLD_NOW | RTLD_DEEPBIND | RTLD_LOCAL);
"""
    return ret


  return ''

def get_type(arg):
  if '*' in arg:
    return 'uint64_t'
  if 'i8' in arg:
    return 'uint8_t'
  if 'i16' in arg:
    return 'uint16_t'
  if 'i32' in arg:
    return 'uint32_t'
  if 'i64' in arg:
    return 'uint64_t'
  if 'void' in arg:
    return 'uint64_t'

  print("unsupported arg '"+arg+"', die")
  exit(0)

def conv_args(arg):
  ret = ""
  n = 0 if ''==arg else arg.count(',') + 1
  for i in range(0, n):
    ret+=get_type(arg.split(',')[i])+" arg"+str(i)
    if i < n-1:
      ret+=", "

  return ret

def conv_args2(n):
  ret = ""

  for i in range(0, n):
    ret+="arg"+str(i)
    if i < n-1:
      ret+=", "
  return ret

def conv_args1(arg):
  ret = ""

  n = 0 if ''==arg else arg.count(',') + 1
  for i in range(0, n):
    ret+=get_type(arg.split(',')[i])
    if i < n-1:
      ret+=", "

  return ret

msg0 = """\
#include "coro.h"
#include "headers.h"

#define WDEBUG 0
#define WDEBUG2 0
#define WDOT 0
#define NWRAP 0

//#define SQLITE_FINAL 0
//#define SQLITE_3 0

int layer = 0;

#define __STACK_SIZE		(4096 * 1024)
#define STACK_MASK_TOP           (~(__STACK_SIZE - 1))


static inline long read_sp(void)
{{
	unsigned long sp;

	__asm__ __volatile__("mov %%rsp, %0" : "=r"(sp));
	return sp;
}}



"""

msg1 = """\
void *{name}_call = NULL;
coro_t *{name}_coro = NULL;
char *{name}_stack = NULL;

static void _{name}_entry_point(uint32_t part0, uint32_t part1{comma} {args}) {{
#if WDEBUG2
	printf("==> CROUTINE STARTS\\n");
#endif
	union ptr_splitter p;
	p.part[0] = part0;
	p.part[1] = part1;
	coro_t *coro = p.ptr;

	{ftype} (*fptr)({args1}) = ({ftype} (*)({args1})) (coro->function);
	int prev = remap[prev_module[layer-1]];
	cmp_lock(prev);
	uint64_t return_value = fptr({args2});
	cmp_unlock(prev);
	coro->state = CORO_FINISHED;
#if WDEBUG2
	printf("==> CROUTINE END\\n");
#endif
	coro_yield(coro, return_value);
}}

{ftype} hooked_{name}({args}) {{
	{pre_hook}

//we called a shared library, and the shared library called us again.
	if(friends(prev_module[layer], E_{module})) {{
		{ftype} (*fptr)({args1}) = ({ftype} (*)({args1})) ({name}_call);
		return ({ftype}) fptr({args2});
	}}

	layer++;

	char {lib}_flag = layer;

#if WDOT
	printf("%d Node%d -> Node%d;\\n", layer, prev_module[layer-1], E_{module});
#endif
	prev_module[layer] = E_{module};

#if WDEBUG
	printf("%*s", {lib}_flag*5, "");
	printf("--> %s@{module}\\n", __func__);
#endif

	thread_t t;
	{name}_coro->thread = &t;
	{name}_coro->state = CORO_NEW;
	_Ux86_64_getcontext(&{name}_coro->context);
	{name}_coro->context.uc_link = 0;
#if 0
	{name}_coro->context.uc_stack.ss_sp = {name}_coro->stack;
	{name}_coro->context.uc_stack.ss_size = default_stack_size;
#else
	{name}_coro->context.uc_stack.ss_sp = (void *) ({name}_coro->stack);
	{name}_coro->context.uc_stack.ss_size = default_stack_size;
#endif

	union ptr_splitter p;
	p.ptr = {name}_coro;
	int callee = remap[prev_module[layer]];
	cmp_unlock(callee);

	long *stack = read_sp() & STACK_MASK_TOP;
	memcpy({name}_coro->stack, stack,  sizeof(long));

{pre_wrap}
	makecontext(&{name}_coro->context, (void (*)())_{name}_entry_point, {narg}, p.part[0], p.part[1]{comma} {args2});
	{ftype} ret = ({ftype}) coro_resume({name}_coro);
	cmp_lock(callee);
{post_wrap}
#if WDEBUG
	printf("%*s", {lib}_flag*5, "");
	printf("<-- %s@{module}\\n", __func__);
#endif

	layer--;

	return ret;
}}


"""

msg2 = """
	char {name}_path[1024];

	snprintf({name}_path, 1024, "%s/{module_low}", path);

	do {{
		void *{name}_handle = dlopen({name}_path, RTLD_NOLOAD | RTLD_NOW);
		if (!{name}_handle) {{
			fprintf(stderr, "%s\\n", dlerror());
			break;
		}}

		void *ipc_tab_stack_nr = dlsym({name}_handle, "{m_enum}_ipc_tab_stack_nr");
		void *ipc_tab_stack = dlsym({name}_handle, "{m_enum}_ipc_tab_stack");
		if(ipc_tab_stack)
			add_ipc_tab_stack(remap[E_{m_enum}], ipc_tab_stack, ipc_tab_stack_nr);

		void *ipc_tab_globals_nr = dlsym({name}_handle, "{m_enum}_ipc_tab_globals_nr");
		void *ipc_tab_globals = dlsym({name}_handle, "{m_enum}_ipc_tab_globals");
		if(ipc_tab_globals)
			add_ipc_tab_globals(remap[E_{m_enum}], ipc_tab_globals, ipc_tab_globals_nr);

		void *ipc_tab_heap_nr = dlsym({name}_handle, "{m_enum}_ipc_tab_heap_nr");
		void *ipc_tab_heap = dlsym({name}_handle, "{m_enum}_ipc_tab_heap");
		if(ipc_tab_heap)
			add_ipc_tab_heap(remap[E_{m_enum}], ipc_tab_heap, ipc_tab_heap_nr);


		{name}_call = dlsym({name}_handle, "{name}");

		if (NULL == {name}_call ) {{
			fprintf(stderr, "Registration %-40s \\x1B[31mfailed!\\x1B[37m     %p \\n", "{module}::{name}", {name}_call);
			break;
		}}

		{name}_stack = mmap(NULL, STACK_SIZE*2, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
		if ({name}_stack == MAP_FAILED) {{
			perror("mmap:");
			break;
		}}

		{name}_coro = calloc(1, sizeof(coro_t));
		{name}_coro->stack = (void *) (( ((unsigned long) ({name}_stack) >> 22) + 1) << 22);
		{name}_coro->function = {name}_call;

		stack_attach(remap[E_{m_enum}], {name}_stack, STACK_SIZE*2);

		fprintf(stderr, "Registration %-40s \\x1B[32msucceed!\\x1B[37m    %p \\n", "{module}::{name}", {name}_call);
	}} while(0);
"""


msg3 = """
void create_cubicles(const char *path) {{

	{hooks}

#if WDOT
	{dots}
#endif

	remap[E_APP] = get_module_id("app.so");
	prev_module[layer] = E_LINUXUPLAT;
}}

struct hooks_s {{
	char func[PATH_MAX];
	char module[PATH_MAX];
	unsigned long hook;
}} hooks[] = {{
{hooks_table}
}};


char *must[] = {{
	"app.so",
	"libramfs.so",
	"libposix_sysinfo.so",
	"libdevfs.so",
	"libukbus.so",

	"liblinuxuplat.so",
	"libuktime.so",

	"libukswrand.so",
	"libuklibparam.so",
	"libukblkdev.so",
	"libuknetdev.so",
	"libfdt.so",
	"libvfscore.so",
	"libukboot.so",
	"libukalloc.so",
	"libposix_libdl.so", //now I need hook these methods
	"liblwip.so",
}};

char *shared[] = {{
	"libnewlibm.so",
	"libnewlibglue.so",
	"libnewlibc.so",
	"libnolibc.so",
	"libposix_process.so",
	"libposix_user.so",
	"libsyscall_shim.so",
	"libukdebug.so",
	"libukboot_main.so", //never called
	"libpthread-embedded.so",

//no keys for this
	"libukargparse.so",
	"libukmpi.so",
	"libuksglist.so",
	"libuktimeconv.so",
	"libuklock.so",
	"libukallocbbuddy.so",
	"lib9pfs.so",
	"libuk9p.so",
	"libuksched.so",
	"libukschedcoop.so",
//
	"libukmmap.so",
}};


int friends(enum modules a, enum modules b) {{
	if( a == b) 
		return 1;
#if !(SQLITE_FINAL || NGINX_FINAL)
	return 0;
#else

#if SQLITE_FINAL
	enum modules flist[] = {{
		E_UKALLOC,
		E_UKBOOT,
		E_VFSCORE,
#if SQLITE_3
		E_RAMFS,
#endif
	}};
#endif

#if NGINX_FINAL
	enum modules flist[] = {{
		E_UKALLOC,
		E_APP,
	}};
#endif

	char a_ok = 0;
	char b_ok = 0;
	for(int i =0; i<sizeof(flist)/sizeof(enum modules); i++) {{
		if(a_ok == 0 && flist[i] == a)
			a_ok = 1;

		if(b_ok == 0 && flist[i] == b)
			b_ok = 1;

		if( (a_ok + b_ok) == 2)
			return 1;
	}}
	return 0;
#endif
}}


static int page_size;
#define ALIGN_ADDR(addr) ((void*)((size_t)(addr) & ~(page_size - 1)))

char is_must(char *module) {{
	for(int i = 0; i < sizeof(must)/sizeof(long); i++) {{
		if( strcmp(module, must[i])  )
			continue;

		return 1;//yes
	}}

	for(int i = 0; i < sizeof(shared)/sizeof(long); i++) {{
		if( strcmp(module, shared[i])  )
			continue;

		return 0;//no
	}}

printf("please add %s to shared or to must\\n", module);while(1);
}}

int install_hook(unsigned long *addr, char *module, char *func) {{
//	printf("requested hook for %p %s %s\\n", addr, module, func);

#if NWRAP
	return 1; //all shared
#endif

	for(int i = 0; i < sizeof(shared)/sizeof(long); i++) {{
		if( strcmp(module, shared[i])  )
			continue;

		return 1;//blue
	}}

	for(int i = 0; i < sizeof(hooks)/sizeof(struct hooks_s); i++) {{
		if( strcmp(module, hooks[i].module) || strcmp(func, hooks[i].func) )
			continue;

		if(hooks[i].hook == 0)
			return 2; //red

                int prot = get_memory_permission(addr);
                if (prot == 0) {{
			printf("memory protection fail\\n");while(1);
                }}

		if (page_size == 0) {{
			page_size = sysconf(_SC_PAGESIZE);
		}}

                if (!(prot & PROT_WRITE)) {{
                    if (mprotect(ALIGN_ADDR(addr), page_size, PROT_READ | PROT_WRITE) != 0) {{
                        printf("Could not change the process memory permission at %p:",
                                   ALIGN_ADDR(addr));while(1);
                    }}
                }}
                *addr = hooks[i].hook;
                if (!(prot & PROT_WRITE)) {{
                    mprotect(ALIGN_ADDR(addr), page_size, prot);
                }}

		return 0; //green

	}}

	if(is_must(module))
		return 2; //red

// what is it? 

	printf("Check %s@%s\\n", func, module); while(1);
	return 1;
}}

int get_id(char *name) {{
	printf("get id %s\\n", name);
	for(int i = 0; i < E_END; i++) {{
		if( strcmp(modnames[i], name)  )
			continue;

		return i;
	}}

	return -1;
}}

"""

parser = argparse.ArgumentParser(description='UK interface generator')
parser.add_argument("path", help="path to the Unikraft source directory")

args = parser.parse_args()

if not os.path.exists(args.path):
    print("Path '"+args.path +"' does not exist")
    exit(1)

lib_path = args.path+'unikraft.ll'

if not os.path.exists(lib_path):
    print("Unikraft IR file does not exist at '"+lib_path)
    exit(1)


#f_ll = open(args.lib, "r")

mods	= ""
enums	= ""
transs	= ""
wraps	= ""
inits	= ""
dots	= ""
hooks_table = ""

skip	= [
	    "nolibc", "ukdebug", "ukmmap", "uk9p",		# use ... inside
	    ]
#	    "ukalloc", "ukallocbuddy"]				# doesn't work

liblist = [
	"../unikraft/lib/uksched/exportsyms.uk",
       "../unikraft/lib/ramfs/exportsyms.uk",
       "../unikraft/lib/posix-sysinfo/exportsyms.uk",
       "../unikraft/lib/ukswrand/exportsyms.uk",
       "../unikraft/lib/uktimeconv/exportsyms.uk",
       "../unikraft/lib/ukalloc/exportsyms.uk",
       "../unikraft/lib/uklibparam/exportsyms.uk",
       "../unikraft/lib/posix-libdl/exportsyms.uk",
       "../unikraft/lib/ukbus/exportsyms.uk",
       "../unikraft/lib/ukschedcoop/exportsyms.uk",
       "../unikraft/lib/uknetdev/exportsyms.uk",
       "../unikraft/lib/9pfs/exportsyms.uk",
       "../unikraft/lib/fdt/exportsyms.uk",
       "../unikraft/lib/ukmpi/exportsyms.uk",
       "../unikraft/lib/ukargparse/exportsyms.uk",
       "../unikraft/lib/uktime/exportsyms.uk",
       "../unikraft/lib/lwip/exportsyms.uk",
       "../unikraft/lib/uksglist/exportsyms.uk",
       "../unikraft/lib/linuxuplat/exportsyms.uk",
       "../unikraft/lib/uklock/exportsyms.uk",
       "../unikraft/lib/vfscore/exportsyms.uk",
       "../unikraft/lib/ukallocbbuddy/exportsyms.uk",
       "../unikraft/lib/ukblkdev/exportsyms.uk",
       "../unikraft/lib/devfs/exportsyms.uk",
       "../unikraft/lib/ukboot/exportsyms.uk",
           ]

#for file in sorted(glob.glob(args.path+'lib/*/exportsyms.uk')):
for file in liblist:
    if any(x in file for x in skip):
     continue

    lib = file.split(args.path+'lib/', 1)[1].split('/', 1)[0]
    lib = lib.replace('-','_').replace('9pfs','d9pfs')
    f = open(file, "r")
    wraps+=("\n#if " + lib.upper() + "\n");
    enums+=("#if " + lib.upper() + "\n" + "\tE_"+lib.upper()+",\n"+"#endif\n");
    transs+=("\t\"lib"+lib+".so\",\n");
    dots+=("if(remap[E_"+lib.upper()+"] > 0) \n\tprintf(\"Node%d [shape=record,label=\\\"{%s}\\\"];\\n\", E_"+lib.upper()+", \"lib"+lib+"\");\n");
    mods+=("#define " + lib.upper() + ' 1\n');
    inits+=("\n#if " + lib.upper() + "\n" + "\tremap[E_"+lib.upper()+"] = get_module_id(\"lib"+lib+".so\");\n");
    hooks_table+=("#if " + lib.upper() + '\n');
    for name in [l for l in (line.strip() for line in f) if l and l[0]!="#" and l!="none"]:
#    for name in ["clock_getres"]:
      for ll_line in open(lib_path).readlines():
       if re.match(r'\Adefine (.*) @'+name+r'\b', ll_line):
        if 'internal' in ll_line or '...' in ll_line:
         continue
#        print(ll_line)
        tp=ll_line.split('@')[0].split('define')[1]
        ag = re.findall(r'\((.*)\)', ll_line)[0]
        narg = 0 if ''==ag else ag.count(',') + 1
        comma = '' if narg==0 else ','
#        print("[%s] %s %s(%s) [%d]\n" % (lib,tp, name, ag, narg) )
        wraps+=(msg1.format(
		name=name, ftype=get_type(tp), args=conv_args(ag), args1=conv_args1(ag),
		args2=conv_args2(narg), narg=str(narg+2), comma=comma, lib=lib, module=change_enum(lib, name).upper(), 
		pre_hook=get_hook(lib, name),
		post_wrap=gen_post_wrap(lib, name),
		pre_wrap=gen_pre_wrap(lib, name),
			    ) + '\n')
        inits+=(msg2.format(name=name,module=change_module(lib,name).upper(), module_low=change_module(lib,name), m_enum=change_enum(lib, name).upper()))
        hooks_table+='\t{\"'+name+'\", \"'+change_module(lib,name)+'\", (unsigned long) hooked_'+name+'},\n'

    wraps+=("#endif // " + lib.upper() +'\n')
    inits+=("#endif // " + lib.upper() +'\n')
    hooks_table+=("#endif // " + lib.upper() +'\n')
    f.close()

print(msg0)
print(mods)

print("enum modules {")
print("\tE_APP,")
print(enums)
print("\tE_END,\n} prev_module[16];")

print("\nchar *modnames[] = {")
print("\t\"app.so\",\n")
print(transs)
print("};")


print("char remap[E_END];");

print(wraps)
print(msg3.format(hooks=inits,hooks_table=hooks_table,dots=dots))