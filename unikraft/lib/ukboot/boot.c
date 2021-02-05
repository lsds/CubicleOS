/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Unikraft bootstrapping
 *
 * Authors: Simon Kuenzer <simon.kuenzer@neclab.eu>
 *
 *
 * Copyright (c) 2017, NEC Europe Ltd., NEC Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * THIS HEADER MAY NOT BE EXTRACTED OR MODIFIED IN ANY WAY.
 */

#include <uk/config.h>

#include <stddef.h>
#include <stdio.h>
#include <errno.h>

#if CONFIG_LIBUKALLOC && CONFIG_LIBUKALLOCBBUDDY && CONFIG_LIBUKBOOT_INITALLOC
#include <uk/allocbbuddy.h>
#endif
#if CONFIG_LIBUKSCHED
#include <uk/sched.h>
#endif
#include <uk/arch/lcpu.h>
#include <uk/plat/bootstrap.h>
#include <uk/plat/memory.h>
#include <uk/plat/lcpu.h>
#include <uk/plat/irq.h>
#include <uk/plat/time.h>
#include <uk/essentials.h>
#include <uk/print.h>
#include <uk/ctors.h>
#include <uk/init.h>
#include <uk/argparse.h>
#ifdef CONFIG_LIBUKLIBPARAM
#include <uk/libparam.h>
#endif /* CONFIG_LIBUKLIBPARAM */

#include "ipc.h"

int main(int argc, char *argv[]) __weak;

void main_thread_func(void *arg) __noreturn;

struct thread_main_arg {
	int argc;
	char **argv;
};

//#define INIT

extern void uknetdev_poll_all(void);

static void main_thread_poll(void *arg) {
	while(1) {
		uknetdev_poll_all();
		uk_sched_yield();
//		usleep(500);
	}
}

void main_thread_func(void *arg)
{
//		uk_pr_crit("MAIN ptr = %p,\n", uk_thread_current());

	int i;
	int ret;
	struct thread_main_arg *tma = arg;
	uk_ctor_func_t *ctorfn;
	uk_init_func_t *initfn;

	/**
	 * Run init table
	 */
#ifdef INIT
	uk_pr_crit("Init Table @ %p - %p\n",
		   &uk_inittab_start[0], &uk_inittab_end);
	uk_inittab_foreach(initfn, uk_inittab_start, uk_inittab_end) {
		UK_ASSERT(*initfn);
		uk_pr_debug("Call init function: %p()...\n", *initfn);
		ret = (*initfn)();
		if (ret < 0) {
			uk_pr_err("Init function at %p returned error %d\n",
				  *initfn, ret);
			ret = UKPLAT_CRASH;
			goto exit;
		}
	}
#else
#if CONFIG_LIBLWIP
	net_add();
//	liblwip_init();
#endif
	vfscore_rootfs();
#if CONFIG_LIBPTHREAD_EMBEDDED
	pthread_initcall();
#endif

#endif

#if CONFIG_LIBUKBOOT_BANNER
	printf("Welcome to  _ __             _____\n");
	printf(" __ _____  (_) /__ _______ _/ _/ /_\n");
	printf("/ // / _ \\/ /  '_// __/ _ `/ _/ __/\n");
	printf("\\_,_/_//_/_/_/\\_\\/_/  \\_,_/_/ \\__/\n");
	printf("%35s\n",
	       STRINGIFY(UK_CODENAME) " " STRINGIFY(UK_FULLVERSION));
	fflush(stdout);
#endif
	/*
	 * Application
	 *
	 * We are calling the application constructors right before calling
	 * the application's main(). All of our Unikraft systems, VFS,
	 * networking stack is initialized at this point. This way we closely
	 * mimic what a regular user application (e.g., BSD, Linux) would expect
	 * from its OS being initialized.
	 */
#ifdef INIT
//FIXME
	uk_pr_crit("Pre-init table at %p - %p\n",
		   &__preinit_array_start[0], &__preinit_array_end);
	uk_ctortab_foreach(ctorfn,
			   __preinit_array_start, __preinit_array_end) {
		if (!*ctorfn)
			continue;

		uk_pr_debug("Call pre-init constructor: %p()...\n", *ctorfn);
		(*ctorfn)();
	}

	uk_pr_crit("Constructor table at %p - %p\n",
		   &__init_array_start[0], &__init_array_end);
	uk_ctortab_foreach(ctorfn, __init_array_start, __init_array_end) {
		if (!*ctorfn)
			continue;

		uk_pr_debug("Call constructor: %p()...\n", *ctorfn);
		(*ctorfn)();
	}
#endif
	uk_pr_crit("Calling main(%d, [", tma->argc);
	for (i = 0; i < tma->argc; ++i) {
		uk_pr_crit("'%s'", tma->argv[i]);
		if ((i + 1) < tma->argc)
			uk_pr_crit(", ");
	}
	uk_pr_crit("])\n");

//	long *stack = ukarch_read_sp() & STACK_MASK_TOP;
//	printf("tac[0] = %lx, %lx, %lx\n", stack[0], ukarch_read_sp(), stack);
////	long *path = (void *) stack[0];
//	char *path2 = (void *) *path;
//	printf("path = %lx, %s, %p, %p\n", path, (char *) path2, uk_thread_current(), uk_thread_current()->name);

	ret = main(tma->argc, tma->argv);
	uk_pr_crit("main returned %d, halting system\n", ret);
	ret = (ret != 0) ? UKPLAT_CRASH : UKPLAT_HALT;

exit:
	ukplat_terminate(ret); /* does not return */
}

/* defined in <uk/plat.h> */
void ukplat_entry_argp(char *arg0, char *argb, __sz argb_len)
{
	static char *argv[CONFIG_LIBUKBOOT_MAXNBARGS];
	int argc = 0;

	if (arg0) {
		argv[0] = arg0;
		argc += 1;
	}
	if (argb && argb_len) {
		argc += uk_argnparse(argb, argb_len, arg0 ? &argv[1] : &argv[0],
				     arg0 ? (CONFIG_LIBUKBOOT_MAXNBARGS - 1)
					  : CONFIG_LIBUKBOOT_MAXNBARGS);
	}
	ukplat_entry(argc, argv);
}

/* defined in <uk/plat.h> */
void ukplat_entry(int argc, char *argv[])
{
	struct thread_main_arg tma;
	int kern_args = 0;
	int rc __maybe_unused = 0;
#if CONFIG_LIBUKALLOC
	struct uk_alloc *a = NULL;
#endif
#if CONFIG_LIBUKALLOC && CONFIG_LIBUKALLOCBBUDDY && CONFIG_LIBUKBOOT_INITALLOC
#if 0
	struct ukplat_memregion_desc md;
#else
//FEMTO
	struct pad_md_s {
		struct  ukplat_memregion_desc md;
		char pad[4096-sizeof(struct ukplat_memregion_desc)];
	} __attribute__ ((aligned (4096))) pad;
	UKBOOT_ipc_tab_stack[0].begin = &pad.md;
#endif
#endif
#if CONFIG_LIBUKSCHED
	struct uk_sched *s = NULL;
	struct uk_thread *main_thread = NULL;
#endif

	uk_ctor_func_t *ctorfn;

#ifdef INIT
	uk_pr_crit("Unikraft constructor table at %p - %p\n",
		   &uk_ctortab_start[0], &uk_ctortab_end);
	uk_ctortab_foreach(ctorfn, uk_ctortab_start, uk_ctortab_end) {
		UK_ASSERT(*ctorfn);
		uk_pr_debug("Call constructor: %p())...\n", *ctorfn);
		(*ctorfn)();
	}
#else
	fdtable_init();
	vfscore_init();
	init_posix_user();
	add_fs();
#endif

#ifdef CONFIG_LIBUKLIBPARAM
	rc = (argc > 1) ? uk_libparam_parse(argv[0], argc - 1, &argv[1]) : 0;
	if (unlikely(rc < 0))
		uk_pr_crit("Failed to parse the kernel argument\n");
	else {
		kern_args = rc;
		uk_pr_crit("Found %d library args\n", kern_args);
	}
#endif /* CONFIG_LIBUKLIBPARAM */

#if CONFIG_LIBUKALLOC && CONFIG_LIBUKALLOCBBUDDY && CONFIG_LIBUKBOOT_INITALLOC
	/* initialize memory allocator
	 * FIXME: ukallocbbuddy is hard-coded for now
	 */
	uk_pr_crit("Initialize memory allocator...\n");
	ukplat_memregion_foreach(&pad.md, UKPLAT_MEMRF_ALLOCATABLE) {
#if CONFIG_UKPLAT_MEMRNAME
		uk_pr_debug("Try memory region: %p - %p (flags: 0x%02x, name: %s)...\n",
			    pad.md.base, (void *)((size_t)pad.md.base + pad.md.len),
			    pad.md.flags, pad.md.name);
#else
		uk_pr_debug("Try memory region: %p - %p (flags: 0x%02x)...\n",
			    pad.md.base, (void *)((size_t)pad.md.base + pad.md.len),
			    pad.md.flags);
#endif

		/* try to use memory region to initialize allocator
		 * if it fails, we will try  again with the next region.
		 * As soon we have an allocator, we simply add every
		 * subsequent region to it
		 */
		if (unlikely(!a))
			a = uk_allocbbuddy_init(pad.md.base, pad.md.len);
		else
			uk_alloc_addmem(a, pad.md.base, pad.md.len);
	}
	if (unlikely(!a))
		uk_pr_warn("No suitable memory region for memory allocator. Continue without heap\n");
	else {
		rc = ukplat_memallocator_set(a);
		if (unlikely(rc != 0))
			UK_CRASH("Could not set the platform memory allocator\n");
	}
#endif

#if CONFIG_LIBUKALLOC
	uk_pr_crit("Initialize IRQ subsystem...\n");
	rc = ukplat_irq_init(a);
	if (unlikely(rc != 0))
		UK_CRASH("Could not initialize the platform IRQ subsystem\n");
#endif

	/* On most platforms the timer depend on an initialized IRQ subsystem */
	uk_pr_crit("Initialize platform time...\n");

//	FEMTO: called by another thread
//	ukplat_time_init();

#if CONFIG_LIBUKSCHED
	/* Init scheduler. */
	s = uk_sched_default_init(a);
	if (unlikely(!s))
		UK_CRASH("Could not initialize the scheduler\n");
#endif

	tma.argc = argc - kern_args;
	tma.argv = &argv[kern_args];

#if CONFIG_LIBUKSCHED
//	uk_thread_create("net-poll", main_thread_poll, NULL);
//	extern void *main_thread_func1(void *);
	main_thread = uk_thread_create("main", main_thread_func, &tma);
	uk_pr_crit("MAIN THREAD %p\n", main_thread);

	long *stack = ukarch_read_sp() & STACK_MASK_TOP;
	uk_pr_crit("stack - %p  %p\n", stack, ukarch_read_sp());
	stack[0]=&main_thread;
//	memcpy(stack, &main_thread, sizeof(long));
	uk_pr_crit("tac[0] = %lx, %lx, %lx\n", stack[0], ukarch_read_sp(), stack);
//while(1);
	if (unlikely(!main_thread))
		UK_CRASH("Could not create main thread\n");
//	uk_sched_start(s);
	ukplat_lcpu_enable_irq();
	main_thread_func(&tma);
#else
	/* Enable interrupts before starting the application */
	ukplat_lcpu_enable_irq();
	main_thread_func(&tma);
#endif
}
