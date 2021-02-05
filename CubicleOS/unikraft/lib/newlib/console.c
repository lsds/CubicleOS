/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * libnewlib glue code
 *
 * Authors: Felipe Huici <felipe.huici@neclab.eu>
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

#include <errno.h>
#include <sys/stat.h>
#include <uk/essentials.h>
#include <uk/print.h>

int isatty(int fd)
{
#ifdef LLVM
	struct stat buf;
#else
	struct pad_s {
		char tmp[16];
		struct stat buf;
		char empty[4096 - sizeof(struct stat) - 16];
	} __attribute__ ((aligned (4096))) pad;
	extern void app_ipc_add_stack(int i, void *begin, size_t size, int mask);
	app_ipc_add_stack(4, &pad.buf, sizeof(struct stat), (1 << 21) | ( 1 << 0));
#endif

#ifdef LLVM
	if (fstat(fd, &buf) < 0) {
#else
	if (fstat(fd, &pad.buf) < 0) {
#endif
		errno = EBADF;
		return 0;
	}
#ifdef LLVM
	if (S_ISCHR(buf.st_mode))
#else
	if (S_ISCHR(pad.buf.st_mode))
#endif
		return 1;

	errno = ENOTTY;
	return 0;
}

char *ttyname(int fd __unused)
{
	return 0;
}

char *ctermid(char *s __unused)
{
	return 0;
}

int grantpt(int fd)
{
	WARN_STUBBED();
	errno = ENOTSUP;
	return -1;
}
