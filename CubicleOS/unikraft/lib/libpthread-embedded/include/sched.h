/* SPDX-License-Identifier: LGPL-2.0-or-later */
/*
 *      Unikraft port of POSIX Threads Library for embedded systems
 *      Copyright(C) 2019, University Politehnica of Bucharest
 *
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License as published by the Free Software Foundation; either
 *      version 2 of the License, or (at your option) any later version.
 *
 *      This library is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 *
 *      You should have received a copy of the GNU Lesser General Public
 *      License along with this library in the file COPYING.LIB;
 *      if not, write to the Free Software Foundation, Inc.,
 *      59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#ifndef __PTHREAD_EMBEDDED__GLUE__SCHED_H__
#define __PTHREAD_EMBEDDED__GLUE__SCHED_H__

#include <stddef.h>
#include_next <sched.h>

#define CPU_SETSIZE 128

typedef struct cpu_set_t {
	unsigned long __bits[CPU_SETSIZE / sizeof(long)];
} cpu_set_t;

#endif
