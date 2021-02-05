/* SPDX-License-Identifier: LGPL-2.0-or-later */
/*
 *      Unikraft port of POSIX Threads Library for embedded systems
 *      Copyright(C) 2019 Costin Lupu, University Politehnica of Bucharest
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

#include <uk/print.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>


int pthread_condattr_getclock(const pthread_condattr_t *__restrict attr,
		clockid_t *__restrict clock_id)
{
	WARN_STUBBED();
	errno = ENOTSUP;
	return -1;
}

int pthread_condattr_setclock(pthread_condattr_t *attr, clockid_t clock_id)
{
	WARN_STUBBED();
	errno = ENOTSUP;
	return -1;
}
