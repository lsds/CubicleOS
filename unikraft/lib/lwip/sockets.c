/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Sharan Santhanam <sharan.santhanam@neclab.eu>
 *
 * Copyright (c) 2019, NEC Laboratories Europe GmbH, NEC Corporation.
 *                     All rights reserved.
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

/* network stub calls */
#include <uk/config.h>
#include <sys/time.h>
#if CONFIG_LWIP_SOCKET_PPOLL
#include <signal.h>
#endif
#include <vfscore/dentry.h>
#include <vfscore/file.h>
#include <vfscore/fs.h>
#include <vfscore/mount.h>
#include <vfscore/vnode.h>
#include <uk/alloc.h>
#include <uk/essentials.h>
#include <uk/errptr.h>
#include <stdio.h>
#include <errno.h>
#include <lwip/sockets.h>

#include "ipc.h"

#define SOCK_NET_SET_ERRNO(errcode) \
	(errno = -(errcode))

int sock_net_close(struct vnode *s_vnode,
			struct vfscore_file *vfscore_file);
int sock_net_write(struct vnode *s_vnode,
			struct uio *buf, int ioflag __unused);
int sock_net_read(struct vnode *s_vnode,
			struct vfscore_file *vfscore_file __unused,
			struct uio *buf, int ioflag __unused);
int sock_net_ioctl(struct vnode *s_vnode,
			struct vfscore_file *vfscore_file __unused,
			unsigned long request,
			void *buf);

extern int sock_net_close_wrap(struct vnode *s_vnode,
			struct vfscore_file *vfscore_file);
extern int sock_net_write_wrap(struct vnode *s_vnode,
			struct uio *buf, int ioflag __unused);
extern int sock_net_read_wrap(struct vnode *s_vnode,
			struct vfscore_file *vfscore_file __unused,
			struct uio *buf, int ioflag __unused);
extern int sock_net_ioctl_wrap(struct vnode *s_vnode,
			struct vfscore_file *vfscore_file __unused,
			unsigned long request,
			void *buf);


#define sock_net_getattr   ((vnop_getattr_t) vfscore_vop_einval)
#define sock_net_inactive  ((vnop_inactive_t) vfscore_vop_nullop)

static struct vnops sock_net_vnops = {
	.vop_close = sock_net_close_wrap,
	.vop_write = sock_net_write_wrap,
	.vop_read  = sock_net_read_wrap,
	.vop_ioctl = sock_net_ioctl_wrap,
	.vop_getattr = sock_net_getattr,
	.vop_inactive = sock_net_inactive
};

#define sock_net_vget  ((vfsop_vget_t) vfscore_vop_nullop)

static struct vfsops sock_net_vfsops = {
	.vfs_vget = sock_net_vget,
	.vfs_vnops = &sock_net_vnops
};


static uint64_t s_inode = 0;
/*
 * Bogus mount point used by all sockets
 */
static struct mount s_mount = {
	.m_op = &sock_net_vfsops
};

struct sock_net_file {
	struct vfscore_file *vfscore_file;
	int sock_fd;
};

static inline struct sock_net_file *sock_net_file_get(int fd)
{
	struct sock_net_file *file = NULL;
	struct vfscore_file *fos;

	fos = vfscore_get_file(fd);
	if (!fos) {
		LWIP_DEBUGF(SOCKETS_DEBUG,
			    ("failed with invalid descriptor\n"));
		file = ERR2PTR(-EINVAL);
		goto EXIT;
	}
	if (fos->f_dentry->d_vnode->v_type != VSOCK) {
		LWIP_DEBUGF(SOCKETS_DEBUG,
			    ("file descriptor is not a socket\n"));
		file = ERR2PTR(-EBADF);
		goto EXIT;
	}
	file = fos->f_data;
EXIT:
	return file;
}

#include "dlmemreq.h"
#define UK_ALLOC_ORIG

static int sock_fd_alloc(int sock_fd)
{
	int ret = 0;
	int vfs_fd;
	struct sock_net_file *file = NULL;
	struct vfscore_file *vfs_file = NULL;
	struct dentry *s_dentry;
#ifdef LLVM
	struct vnode *s_vnode;
#else
static struct pad1_s {
	struct vnode *s_vnode;
	char pad[4096-sizeof(long)];
} __attribute__ ((aligned (4096))) pad1;
lwip_ipc_add_stack(4, &pad1.s_vnode, sizeof(long), (1 << 17) | ( 1 << 21));
#endif

	/* Reserve file descriptor number */
	vfs_fd = vfscore_alloc_fd();
	if (vfs_fd < 0) {
		ret = -ENFILE;
		LWIP_DEBUGF(SOCKETS_DEBUG,
			    ("Failed to allocate file descriptor number\n"));
		goto ERR_EXIT;
	}

	/* Allocate file, dentry, and vnode */
#ifdef UK_ALLOC_ORIG
	file = uk_calloc(uk_alloc_get_default(), 1, sizeof(*file));
#else
	file = dl_lwip_calloc(1, sizeof(*file));
#endif
	if (!file) {
		ret = -ENOMEM;
		LWIP_DEBUGF(SOCKETS_DEBUG,
			    ("Failed to allocate socket file: Out of memory\n"));
		goto ERR_MALLOC_FILE;
	}
#ifdef UK_ALLOC_ORIG
	vfs_file = uk_calloc(uk_alloc_get_default(), 1, sizeof(*vfs_file));
#else
	vfs_file = dl_lwip_calloc(1, sizeof(*vfs_file));
#endif
	if (!vfs_file) {
		ret = -ENOMEM;
		LWIP_DEBUGF(SOCKETS_DEBUG,
				("Failed to allocate socket vfs_file: Out of memory\n"));
		goto ERR_MALLOC_VFS_FILE;
	}

#ifdef LLVM
	ret = vfscore_vget(&s_mount, s_inode++, &s_vnode);
#else
	ret = vfscore_vget(&s_mount, s_inode++, &pad1.s_vnode);
#endif
	UK_ASSERT(ret == 0); /* we should not find it in cache */

#ifdef LLVM
	if (!s_vnode) {
#else
	if (!pad1.s_vnode) {
#endif
		ret = -ENOMEM;
		LWIP_DEBUGF(SOCKETS_DEBUG,
			    ("Failed to allocate socket vnode: Out of memory\n"));
		goto ERR_ALLOC_VNODE;
	}

//	uk_mutex_unlock(&s_vnode->v_lock);

	/*
	 * it doesn't matter that all the dentries have the
	 * same path since we never lookup for them
	 */
#ifdef LLVM
	s_dentry = dentry_alloc(NULL, s_vnode, "/");
#else
	s_dentry = dentry_alloc(NULL, pad1.s_vnode, "/");
#endif

	if (!s_dentry) {
		ret = -ENOMEM;
		LWIP_DEBUGF(SOCKETS_DEBUG,
			    ("Failed to allocate socket dentry: Out of memory\n"));
		goto ERR_ALLOC_DENTRY;
	}
	lwip_ipc_add_heap(vfs_file, sizeof(*vfs_file), (1 << 17) | ( 1 << 21));
	/* Put things together, and fill out necessary fields */
	vfs_file->fd = vfs_fd;
	vfs_file->f_flags = UK_FWRITE | UK_FREAD;
	vfs_file->f_count = 1;
	vfs_file->f_data = file;
	vfs_file->f_dentry = s_dentry;
	vfs_file->f_vfs_flags = UK_VFSCORE_NOPOS;

#ifdef LLVM
	s_vnode->v_data = file;
	s_vnode->v_type = VSOCK;
#else
	pad1.s_vnode->v_data = file;
	pad1.s_vnode->v_type = VSOCK;
#endif

	file->vfscore_file = vfs_file;
	file->sock_fd = sock_fd;
	LWIP_DEBUGF(SOCKETS_DEBUG, ("Allocated socket %d (%x)\n",
				    file->vfscore_file->fd,
				    file->sock_fd));

	/* Storing the information within the vfs structure */
	ret = vfscore_install_fd(vfs_fd, file->vfscore_file);
	if (ret) {
		LWIP_DEBUGF(SOCKETS_DEBUG,
			    ("Failed to install socket fd\n"));
		goto ERR_VFS_INSTALL;
	}

	/* Only the dentry should hold a reference; release ours */
#ifdef LLVM
	vrele(s_vnode);
#else
	vrele(pad1.s_vnode);
#endif

	/* Return file descriptor of our socket */
	return vfs_fd;

ERR_VFS_INSTALL:
	drele(s_dentry);
ERR_ALLOC_DENTRY:
#ifdef LLVM
	vrele(s_vnode);
#else
	vrele(pad1.s_vnode);
#endif
ERR_ALLOC_VNODE:
#ifdef UK_ALLOC_ORIG
	uk_free(uk_alloc_get_default(), vfs_file);
#else
	dl_lwip_free(vfs_file);
#endif
ERR_MALLOC_VFS_FILE:
#ifdef UK_ALLOC_ORIG
	uk_free(uk_alloc_get_default(), file);
#else
	dl_lwip_free(file);
#endif
ERR_MALLOC_FILE:
	vfscore_put_fd(vfs_fd);
ERR_EXIT:
	UK_ASSERT(ret < 0);
	return ret;
}

int sock_net_close(struct vnode *s_vnode,
			struct vfscore_file *vfscore_file)
{
	int ret;
	struct sock_net_file *file = NULL;

//we should remove this from ipc, but VFSCORE uses it later. 
//	lwip_ipc_remove_heap(vfscore_file);

	file = s_vnode->v_data;
	LWIP_DEBUGF(SOCKETS_DEBUG, ("%s fd:%d lwip_fd:%d\n",
				    __func__,
				    file->vfscore_file->fd,
				    file->sock_fd));

	UK_ASSERT(vfscore_file->f_dentry->d_vnode == s_vnode);
	UK_ASSERT(s_vnode->v_refcnt == 1);

	/* Close and release the lwip socket */
	ret = lwip_close(file->sock_fd);

	/*
	 * Free socket file
	 * The rest of the resources will be freed by vfs
	 *
	 * TODO: vfs ignores close errors right now, so free our file
	 */
	uk_free(uk_alloc_get_default(), file);

	/*
	 * lwip sets errno and returns -1 in case of error, but
	 * vfs expects us to return a positive errno
	 */
	if (ret < 0)
		return errno;

	return ret;
}

int sock_net_write(struct vnode *s_vnode,
			struct uio *buf, int ioflag __unused)
{
	int ret = 0;
	struct sock_net_file *file = NULL;

	file = s_vnode->v_data;
	LWIP_DEBUGF(SOCKETS_DEBUG, ("%s fd:%d lwip_fd:%d\n",
				    __func__,
				    file->vfscore_file->fd,
				    file->sock_fd));
	ret = lwip_writev(file->sock_fd, buf->uio_iov, buf->uio_iovcnt);
	/*
	 * lwip sets errno and returns -1 in case of error, but
	 * vfs expects us to return a positive errno
	 */
	if (ret < 0)
		return errno;

	buf->uio_resid -= ret;
	return 0;
}

int sock_net_read(struct vnode *s_vnode,
			struct vfscore_file *vfscore_file __unused,
			struct uio *buf, int ioflag __unused)
{
	int ret = 0;
	struct sock_net_file *file = NULL;

	file = s_vnode->v_data;
	LWIP_DEBUGF(SOCKETS_DEBUG, ("%s fd:%d lwip_fd:%d\n",
				    __func__,
				    file->vfscore_file->fd,
				    file->sock_fd));
	ret = lwip_readv(file->sock_fd, buf->uio_iov, buf->uio_iovcnt);
	/*
	 * lwip sets errno and returns -1 in case of error, but
	 * vfs expects us to return a positive errno
	 */
	if (ret < 0)
		return errno;

	buf->uio_resid -= ret;
	return 0;
}

int sock_net_ioctl(struct vnode *s_vnode,
			struct vfscore_file *vfscore_file __unused,
			unsigned long request,
			void *buf)
{
	struct sock_net_file *file = NULL;

	file = s_vnode->v_data;
	LWIP_DEBUGF(SOCKETS_DEBUG, ("%s fd:%d lwip_fd:%d\n",
				    __func__,
				    file->vfscore_file->fd,
				    file->sock_fd));
	return lwip_ioctl(file->sock_fd, request, buf);
}

int socket(int domain, int type, int protocol)
{
	int ret = 0;
	int vfs_fd = 0xff;
	int sock_fd = 0;

	/* Create lwip_socket */
	sock_fd = lwip_socket(domain, type, protocol);
	if (sock_fd < 0) {
		LWIP_DEBUGF(SOCKETS_DEBUG, ("failed to create socket %d\n",
					    errno));
		ret = -1;
		goto EXIT;
	}

	/* Allocate the file descriptor */
	vfs_fd = sock_fd_alloc(sock_fd);
	if (vfs_fd < 0) {
		LWIP_DEBUGF(SOCKETS_DEBUG,
			    ("failed to allocate descriptor %d\n",
			     errno));
		ret = -1;
		/* Setting the errno */
		SOCK_NET_SET_ERRNO(vfs_fd);
		goto LWIP_SOCKET_CLEANUP;
	}

	/* Returning the file descriptor to the user */
	ret = vfs_fd;
EXIT:
	return ret;
LWIP_SOCKET_CLEANUP:
	/* Cleanup the lwip socket */
	lwip_close(sock_fd);
	goto EXIT;
}

int accept(int s, struct sockaddr *addr, socklen_t *addrlen)
{
	int ret = 0;
	struct sock_net_file *file;
	int sock_fd, vfs_fd;

	file = sock_net_file_get(s);
	if (PTRISERR(file)) {
		LWIP_DEBUGF(SOCKETS_DEBUG,
			    ("failed to accept incoming connection\n"));
		ret = -1;
		/* Setting the errno */
		SOCK_NET_SET_ERRNO(PTR2ERR(file));
		goto EXIT;
	}

	/* Accept an incoming connection */
	sock_fd = lwip_accept(file->sock_fd, addr, addrlen);
	if (sock_fd < 0) {
		LWIP_DEBUGF(SOCKETS_DEBUG,
			    ("failed to accept incoming connection\n"));
		ret = -1;
		goto EXIT_FDROP;
	}

	/* Allocate the file descriptor for the accepted connection */
	vfs_fd = sock_fd_alloc(sock_fd);
	if (vfs_fd < 0) {
		LWIP_DEBUGF(SOCKETS_DEBUG,
			    ("failed to allocate descriptor for accepted connection\n"));
		ret = -1;
		/* Setting the errno */
		SOCK_NET_SET_ERRNO(vfs_fd);
		goto LWIP_SOCKET_CLEANUP;
	}
	ret = vfs_fd;
EXIT_FDROP:
	vfscore_put_file(file->vfscore_file); /* release refcount */
EXIT:
	return ret;

LWIP_SOCKET_CLEANUP:
	lwip_close(sock_fd);
	goto EXIT_FDROP;
}

int bind(int s, const struct sockaddr *name, socklen_t namelen)
{
	int ret = 0;
	struct sock_net_file *file = NULL;

	file = sock_net_file_get(s);
	if (PTRISERR(file)) {
		LWIP_DEBUGF(SOCKETS_DEBUG,
			    ("failed to identify socket descriptor\n"));
		ret = -1;
		/* Setting the errno */
		SOCK_NET_SET_ERRNO(PTR2ERR(file));
		goto EXIT;
	}
	/* Bind an incoming connection */
	ret = lwip_bind(file->sock_fd, name, namelen);
	if (ret < 0) {
		LWIP_DEBUGF(SOCKETS_DEBUG,
			    ("failed to bind with socket\n"));
		ret = -1;
		goto EXIT_FDROP;
	}
EXIT_FDROP:
	vfscore_put_file(file->vfscore_file); /* release refcount */
EXIT:
	return ret;
}

int poll(struct pollfd fds[], nfds_t nfds, int timeout)
{
	int ret;
	unsigned int i;
	struct sock_net_file *file;
	struct pollfd lwip_fds[nfds];

	for (i = 0; i < nfds; i++) {
		if (fds[i].fd < 0)
			lwip_fds[i].fd = fds[i].fd;
		else {
			file = sock_net_file_get(fds[i].fd);
			if (PTRISERR(file)) {
				LWIP_DEBUGF(SOCKETS_DEBUG,
					    ("failed to identify socket descriptor\n"));
				ret = -1;
				/* Setting the errno */
				SOCK_NET_SET_ERRNO(PTR2ERR(file));
				goto EXIT;
			}
			lwip_fds[i].fd = file->sock_fd;
			lwip_fds[i].events = fds[i].events;
			vfscore_put_file(file->vfscore_file); /* release refcount */
		}
	}

	ret = lwip_poll(lwip_fds, nfds, timeout);
	if (ret < 0)
		goto EXIT;

	for (i = 0; i < nfds; i++) {
		if (fds[i].fd < 0)
			fds[i].revents = 0;
		else
			fds[i].revents = lwip_fds[i].revents;
	}

EXIT:
	return ret;
}

#if CONFIG_LWIP_SOCKET_PPOLL
#if CONFIG_LIBPTHREAD_EMBEDDED
#define __sigmask   pthread_sigmask
#else
#define __sigmask   sigprocmask
#endif
int ppoll(struct pollfd *fds, nfds_t nfds, const struct timespec *tmo_p,
		const sigset_t *sigmask)
{
	sigset_t origmask;
	int timeout, rc, _rc;

	if (!fds) {
		errno = EFAULT;
		rc = -1;
		goto out;
	}

	timeout = (tmo_p == NULL) ? -1 :
		(tmo_p->tv_sec * 1000 + tmo_p->tv_nsec / 1000000);
	rc = __sigmask(SIG_SETMASK, sigmask, &origmask);
	if (rc)
		goto out;
	rc = poll(fds, nfds, timeout);
	_rc = __sigmask(SIG_SETMASK, &origmask, NULL);
	if (rc == 0 && _rc != 0)
		rc = _rc;
out:
	return rc;
}
#endif /* CONFIG_LWIP_SOCKET_PPOLL */

volatile int ok_select = 0;
rdy_select() {
	ok_select = 1;
}

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
		struct timeval *timeout)
{
//uk_pr_crit("************* SELECT %p ***********\n", timeout);
extern void uknetdev_poll_all(void);
	uknetdev_poll_all();
	uint64_t nsecs;
	fd_set rd, wr, xc;
	int i, ret, maxfd;
	struct sock_net_file *file;

	if (nfds == 0 && timeout != NULL) {
		nsecs = timeout->tv_sec * 1000000000;
		nsecs += timeout->tv_usec * 1000;
		uk_sched_thread_sleep(nsecs);
		return 0;
	}

	/* translate the public (vfscore) fds into lwIP socket fds */
	FD_ZERO(&rd);
	FD_ZERO(&wr);
	FD_ZERO(&xc);
	maxfd = 0;
	for (i = 0; i < nfds; i++) {
		if (readfds && FD_ISSET(i, readfds)) {
			file = sock_net_file_get(i);
			if (PTRISERR(file)) {
#if CONFIG_LWIP_SOCKET_SELECT_GENERIC_FDS
				/* We allow other fd types, but we don't support them */
				if (PTR2ERR(file) == -EBADF) {
					FD_CLR(i, readfds);
					continue;
				}
#else
				LWIP_DEBUGF(SOCKETS_DEBUG,
					    ("failed to identify socket descriptor\n"));
				ret = -1;
				/* Setting the errno */
				SOCK_NET_SET_ERRNO(PTR2ERR(file));
				goto EXIT;
#endif
			}
			if (maxfd < file->sock_fd)
				maxfd = file->sock_fd;
			FD_SET(file->sock_fd, &rd);
			vfscore_put_file(file->vfscore_file); /* release refcount */
		}
		if (writefds && FD_ISSET(i, writefds)) {
			file = sock_net_file_get(i);
			if (PTRISERR(file)) {
#if CONFIG_LWIP_SOCKET_SELECT_GENERIC_FDS
				/* We allow other fd types, but we don't support them */
				if (PTR2ERR(file) == -EBADF) {
					FD_CLR(i, writefds);
					continue;
				}
#else
				LWIP_DEBUGF(SOCKETS_DEBUG,
					    ("failed to identify socket descriptor\n"));
				ret = -1;
				/* Setting the errno */
				SOCK_NET_SET_ERRNO(PTR2ERR(file));
				goto EXIT;
#endif
			}
			if (maxfd < file->sock_fd)
				maxfd = file->sock_fd;
			FD_SET(file->sock_fd, &wr);
			vfscore_put_file(file->vfscore_file); /* release refcount */
		}
		if (exceptfds && FD_ISSET(i, exceptfds)) {
			file = sock_net_file_get(i);
			if (PTRISERR(file)) {
#if CONFIG_LWIP_SOCKET_SELECT_GENERIC_FDS
				/* We allow other fd types, but we don't support them */
				if (PTR2ERR(file) == -EBADF) {
					FD_CLR(i, exceptfds);
					continue;
				}
#else
				LWIP_DEBUGF(SOCKETS_DEBUG,
					    ("failed to identify socket descriptor\n"));
				ret = -1;
				/* Setting the errno */
				SOCK_NET_SET_ERRNO(PTR2ERR(file));
				goto EXIT;
#endif
			}
			if (maxfd < file->sock_fd)
				maxfd = file->sock_fd;
			FD_SET(file->sock_fd, &xc);
			vfscore_put_file(file->vfscore_file); /* release refcount */
		}
	}

	ret = lwip_select(maxfd+1, &rd, &wr, &xc, timeout);
	if (ret < 0)
		goto EXIT;

	/* translate back from lwIP socket fds to public (vfscore) fds.
	 * But there's no way to go from lwIP to vfscore, so iterate over
	 * everything again. Check which ones were set originally, and if
	 * they aren't also set in lwip_select()'s return, clear them.
	 */
	for (i = 0; i < nfds; i++) {
		if (readfds && FD_ISSET(i, readfds)) {
			/* This lookup can't fail, or it would already have
			 * failed during the translation above.
			 */
			file = sock_net_file_get(i);
			if (!FD_ISSET(file->sock_fd, &rd))
				FD_CLR(i, readfds);
			vfscore_put_file(file->vfscore_file); /* release refcount */
		}
		if (writefds && FD_ISSET(i, writefds)) {
			/* This lookup can't fail, or it would already have
			 * failed during the translation above.
			 */
			file = sock_net_file_get(i);
			if (!FD_ISSET(file->sock_fd, &wr))
				FD_CLR(i, writefds);
			vfscore_put_file(file->vfscore_file); /* release refcount */
		}
		if (exceptfds && FD_ISSET(i, exceptfds)) {
			/* This lookup can't fail, or it would already have
			 * failed during the translation above.
			 */
			file = sock_net_file_get(i);
			if (!FD_ISSET(file->sock_fd, &xc))
				FD_CLR(i, exceptfds);
			vfscore_put_file(file->vfscore_file); /* release refcount */
		}
	}
//uk_pr_crit("************* SELECT OUT ***********\n");
EXIT:
	return ret;
}

int shutdown(int s, int how)
{
	int ret = 0;
	struct sock_net_file *file = NULL;

	file = sock_net_file_get(s);
	if (PTRISERR(file)) {
		LWIP_DEBUGF(SOCKETS_DEBUG,
			    ("failed to identify socket descriptor\n"));
		ret = -1;
		/* Setting the errno */
		SOCK_NET_SET_ERRNO(PTR2ERR(file));
		goto EXIT;
	}
	/* Shutdown of the descriptor */
	ret = lwip_shutdown(file->sock_fd, how);
	vfscore_put_file(file->vfscore_file); /* release refcount */
EXIT:
	return ret;
}

int getpeername(int s, struct sockaddr *name, socklen_t *namelen)
{
	int ret = 0;
	struct sock_net_file *file = NULL;

	file = sock_net_file_get(s);
	if (PTRISERR(file)) {
		LWIP_DEBUGF(SOCKETS_DEBUG, ("failed to identify socket\n"));
		ret = -1;
		SOCK_NET_SET_ERRNO(PTR2ERR(file));
		goto EXIT;
	}
	ret = lwip_getpeername(file->sock_fd, name, namelen);
	vfscore_put_file(file->vfscore_file); /* release refcount */
EXIT:
	return ret;
}

int getsockname(int s, struct sockaddr *name, socklen_t *namelen)
{
	int ret = 0;
	struct sock_net_file *file = NULL;

	file = sock_net_file_get(s);
	if (PTRISERR(file)) {
		LWIP_DEBUGF(SOCKETS_DEBUG, ("failed to identify socket\n"));
		ret = -1;
		SOCK_NET_SET_ERRNO(PTR2ERR(file));
		goto EXIT;
	}
	ret = lwip_getsockname(file->sock_fd, name, namelen);
	vfscore_put_file(file->vfscore_file); /* release refcount */
EXIT:
	return ret;
}

int getsockopt(int s, int level, int optname, void *optval, socklen_t *optlen)
{
	int ret = 0;
	struct sock_net_file *file = NULL;

	file = sock_net_file_get(s);
	if (PTRISERR(file)) {
		LWIP_DEBUGF(SOCKETS_DEBUG,
			    ("failed to identify socket descriptor\n"));
		ret = -1;
		SOCK_NET_SET_ERRNO(PTR2ERR(file));
		goto EXIT;
	}
	ret = lwip_getsockopt(file->sock_fd, level, optname, optval, optlen);
	vfscore_put_file(file->vfscore_file); /* release refcount */
EXIT:
	return ret;
}

int setsockopt(int s, int level, int optname, const void *optval,
	       socklen_t optlen)
{
	int ret = 0;
	struct sock_net_file *file = NULL;

	file = sock_net_file_get(s);
	if (PTRISERR(file)) {
		LWIP_DEBUGF(SOCKETS_DEBUG,
			    ("failed to identify socket descriptor\n"));
		ret = -1;
		SOCK_NET_SET_ERRNO(PTR2ERR(file));
		goto EXIT;
	}
	ret = lwip_setsockopt(file->sock_fd, level, optname, optval, optlen);
	vfscore_put_file(file->vfscore_file); /* release refcount */
EXIT:
	return ret;
}

int connect(int s, const struct sockaddr *name, socklen_t namelen)
{
	int ret = 0;
	struct sock_net_file *file = NULL;

	file = sock_net_file_get(s);
	if (PTRISERR(file)) {
		LWIP_DEBUGF(SOCKETS_DEBUG,
			    ("failed to identify socket descriptor\n"));
		ret = -1;
		SOCK_NET_SET_ERRNO(PTR2ERR(file));
		goto EXIT;
	}
	ret = lwip_connect(file->sock_fd, name, namelen);
	vfscore_put_file(file->vfscore_file); /* release refcount */
EXIT:
	return ret;
}

int listen(int s, int backlog)
{
	int ret = 0;
	struct sock_net_file *file = NULL;

	file = sock_net_file_get(s);
	if (PTRISERR(file)) {
		LWIP_DEBUGF(SOCKETS_DEBUG,
			    ("failed to identify socket descriptor\n"));
		ret = -1;
		SOCK_NET_SET_ERRNO(PTR2ERR(file));
		goto EXIT;
	}
	ret = lwip_listen(file->sock_fd, backlog);
	vfscore_put_file(file->vfscore_file); /* release refcount */
EXIT:
	return ret;
}

int recv(int s, void *mem, size_t len, int flags)
{
	int ret = 0;
	struct sock_net_file *file = NULL;

	file = sock_net_file_get(s);
	if (PTRISERR(file)) {
		LWIP_DEBUGF(SOCKETS_DEBUG,
			    ("failed to identify socket descriptor\n"));
		ret = -1;
		SOCK_NET_SET_ERRNO(PTR2ERR(file));
		goto EXIT;
	}
	ret = lwip_recv(file->sock_fd, mem, len, flags);
	vfscore_put_file(file->vfscore_file); /* release refcount */
EXIT:
	return ret;
}

int recvfrom(int s, void *mem, size_t len, int flags,
		      struct sockaddr *from, socklen_t *fromlen)
{
	int ret = 0;
	struct sock_net_file *file = NULL;

	file = sock_net_file_get(s);
	if (PTRISERR(file)) {
		LWIP_DEBUGF(SOCKETS_DEBUG,
			    ("failed to identify socket descriptor\n"));
		ret = -1;
		SOCK_NET_SET_ERRNO(PTR2ERR(file));
		goto EXIT;
	}
	ret = lwip_recvfrom(file->sock_fd, mem, len, flags, from, fromlen);
	vfscore_put_file(file->vfscore_file); /* release refcount */
EXIT:
	return ret;
}

int recvmsg(int s, struct msghdr *msg, int flags)
{
	int ret = 0;
	struct sock_net_file *file = NULL;

	file = sock_net_file_get(s);
	if (PTRISERR(file)) {
		LWIP_DEBUGF(SOCKETS_DEBUG,
			    ("failed to identify socket descriptor\n"));
		ret = -1;
		SOCK_NET_SET_ERRNO(PTR2ERR(file));
		goto EXIT;
	}
	ret = lwip_recvmsg(file->sock_fd, msg, flags);
	vfscore_put_file(file->vfscore_file); /* release refcount */
EXIT:
	return ret;
}

int send(int s, const void *dataptr, size_t size, int flags)
{
	int ret = 0;
	struct sock_net_file *file = NULL;

	file = sock_net_file_get(s);
	if (PTRISERR(file)) {
		LWIP_DEBUGF(SOCKETS_DEBUG,
			    ("failed to identify socket descriptor\n"));
		ret = -1;
		SOCK_NET_SET_ERRNO(PTR2ERR(file));
		goto EXIT;
	}
	ret = lwip_send(file->sock_fd, dataptr, size, flags);
	vfscore_put_file(file->vfscore_file); /* release refcount */
EXIT:
	return ret;
}

int sendmsg(int s, const struct msghdr *message, int flags)
{
	int ret = 0;
	struct sock_net_file *file = NULL;

	file = sock_net_file_get(s);
	if (PTRISERR(file)) {
		LWIP_DEBUGF(SOCKETS_DEBUG,
			    ("failed to identify socket descriptor\n"));
		ret = -1;
		SOCK_NET_SET_ERRNO(PTR2ERR(file));
		goto EXIT;
	}
	ret = lwip_sendmsg(file->sock_fd, message, flags);
	vfscore_put_file(file->vfscore_file); /* release refcount */
EXIT:
	return ret;
}

int sendto(int s, const void *dataptr, size_t size, int flags,
		    const struct sockaddr *to, socklen_t tolen)
{
	int ret = 0;
	struct sock_net_file *file = NULL;

	file = sock_net_file_get(s);
	if (PTRISERR(file)) {
		LWIP_DEBUGF(SOCKETS_DEBUG,
			    ("failed to identify socket descriptor\n"));
		ret = -1;
		SOCK_NET_SET_ERRNO(PTR2ERR(file));
		goto EXIT;
	}
	ret = lwip_sendto(file->sock_fd, dataptr, size, flags, to, tolen);
	vfscore_put_file(file->vfscore_file); /* release refcount */
EXIT:
	return ret;
}

int socketpair(int domain, int type, int protocol, int sv[2])
{
	errno = ENOTSUP;
	return -1;
}
