/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2005-2007, Kohsuke Ohtani
 * Copyright (C) 2014 Cloudius Systems, Ltd.
 * Copyright (c) 2019, NEC Europe Ltd., NEC Corporation.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * vfs_mount.c - mount operations
 */

#define _BSD_SOURCE

#include <sys/param.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "vfs.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <uk/list.h>
#include <uk/mutex.h>
#include <vfscore/prex.h>
#include <vfscore/dentry.h>
#include <vfscore/vnode.h>

#include "ipc.h"

/*
 * List for VFS mount points.
 */

UK_LIST_HEAD(mount_list);

/*
 * Global lock to access mount point.
 */
#ifdef CONFIG_LIBUKSCHED1
static struct uk_mutex mount_lock = UK_MUTEX_INITIALIZER(mount_lock);
#else
static int mount_lock = 0;
#define uk_mutex_lock(A)
#define uk_mutex_unlock(A)
#define uk_mutex_init(A)
#endif

#if 1
struct vfscore_fs_type *fses[10];
int fses_cnt = 0;

static struct vfscore_fs_type *uk_fslist_start2 = &fses[0];
static struct vfscore_fs_type *uk_fslist_end2 = &fses[0];

extern struct vfscore_fs_type fs_devfs;
extern struct vfscore_fs_type fs_ramfs;

#ifdef CONFIG_LIBRAMFS

int ramfs_rename2(struct vnode *dvp1, struct vnode *vp1, char *name1,
			 struct vnode *dvp2, struct vnode *vp2, char *name2) {
	return ramfs_rename(dvp1, vp1, name1, dvp2, vp2, name2);
}

int ramfs_mkdir2(struct vnode *dvp, char *name, mode_t mode) {
	return ramfs_mkdir(dvp, name, mode);
}

int ramfs_symlink2(struct vnode *dvp, char *name, char *link) {
	return ramfs_symlink(dvp, name, link);
}

int ramfs_rmdir2(struct vnode *dvp, struct vnode *vp, char *name __unused) {
	return ramfs_rmdir(dvp, vp, name);
}

int ramfs_remove2(struct vnode *dvp, struct vnode *vp, char *name __maybe_unused) {
	return ramfs_remove(dvp, vp, name);
}

int ramfs_truncate2(struct vnode *vp, off_t length) {
	return ramfs_truncate(vp, length);
}

int ramfs_getattr2(struct vnode *vnode, struct vattr *attr) {
	return ramfs_getattr(vnode, attr);
}

int ramfs_setattr2(struct vnode *vnode, struct vattr *attr) {
	return ramfs_setattr(vnode, attr);
}

int ramfs_read2(struct vnode *vp, struct vfscore_file *fp, struct uio *uio, int ioflag) {
	return ramfs_read(vp, fp, uio, ioflag);
}

int ramfs_write2(struct vnode *vp, struct uio *uio, int ioflag) {
	return ramfs_write(vp, uio, ioflag);
}

int ramfs_readdir2(struct vnode *vp, struct vfscore_file *fp, struct dirent *dir) {
	return ramfs_readdir(vp, fp, dir);
}

int ramfs_lookup2(struct vnode *dvp, char *name, struct vnode **vpp) {
	return ramfs_lookup(dvp, name, vpp);
}

int ramfs_create2(struct vnode *dvp, char *name, mode_t mode) {
	return ramfs_create(dvp, name, mode);
}

int ramfs_readlink2(struct vnode *vp, struct uio *uio) {
	return ramfs_readlink(vp, uio);
}

struct vnops ramfs_vnops = {
		((vnop_open_t)vfscore_vop_nullop),             /* open */
		((vnop_close_t)vfscore_vop_nullop),            /* close */
		ramfs_read2,             /* read */
		ramfs_write2,            /* write */
		((vnop_seek_t)vfscore_vop_nullop),             /* seek */
		((vnop_ioctl_t)vfscore_vop_einval),            /* ioctl */
		((vnop_fsync_t)vfscore_vop_nullop),            /* fsync */
		ramfs_readdir2,          /* readdir */
		ramfs_lookup2,           /* lookup */
		ramfs_create2,           /* create */
		ramfs_remove2,           /* remove */
		ramfs_rename2,           /* remame */
		ramfs_mkdir2,            /* mkdir */
		ramfs_rmdir2,            /* rmdir */
		ramfs_getattr2,          /* getattr */
		ramfs_setattr2,          /* setattr */
		((vnop_inactive_t)vfscore_vop_nullop),         /* inactive */
		ramfs_truncate2,         /* truncate */
		((vnop_link_t)vfscore_vop_eperm),             /* link */
		(vnop_cache_t) NULL,    /* arc */
		((vnop_fallocate_t)vfscore_vop_nullop),        /* fallocate */
		ramfs_readlink2,         /* read link */
		ramfs_symlink2,          /* symbolic link */
};

int ramfs_mount2(struct mount *mp, const char *dev, int flags, const void *data) {
	return ramfs_mount(mp, dev, flags, data);
}

int ramfs_unmount2(struct mount *mp, int flags) {
	return ramfs_unmount(mp, flags);
}

struct vfsops ramfs_vfsops = {
		ramfs_mount2,       /* mount */
		ramfs_unmount2,     /* unmount */
		((vfsop_sync_t)vfscore_nullop),        /* sync */
		((vfsop_sync_t)vfscore_nullop),        /* vget */
		((vfsop_sync_t)vfscore_nullop),      /* statfs */
		&ramfs_vnops,      /* vnops */
};

struct vfscore_fs_type fs_ramfs2 = {
	.vs_name = "ramfs",
	.vs_init = NULL,
	.vs_op = &ramfs_vfsops,
};

#endif

////////////////////////////
#ifdef CONFIG_LIBDEVFS

static int vop_einval(void)
{
	return EINVAL;
}

static int vop_eperm(void)
{
	return EPERM;
}

int devfs_unmount2(struct mount *mp, int flags) {
	return devfs_unmount(mp, flags);
}

int devfs_open2(struct vfscore_file *fp) {
	return devfs_open(fp);
}

int devfs_close2(struct vnode *vp, struct vfscore_file *fp) {
	return devfs_close(vp, fp);
}

int devfs_read2(struct vnode *vp, struct vfscore_file *fp, struct uio *uio, int ioflags) {
	return devfs_read(vp, fp, uio, ioflags);
}

int devfs_write2(struct vnode *vp, struct uio *uio, int ioflags) {
	return devfs_write(vp, uio, ioflags);
}

int devfs_ioctl2(struct vnode *vp, struct vfscore_file *fp, unsigned long cmd, void *arg) {
	return devfs_ioctl(vp, fp, cmd, arg);
}

int devfs_lookup2(struct vnode *dvp, char *name, struct vnode **vpp) {
	return devfs_lookup(dvp, name, vpp);
}

int devfs_readdir2(struct vnode *vp, struct vfscore_file *fp, struct dirent *dir) {
	return devfs_readdir(vp, fp, dir);
}

int devfs_getattr2(struct vnode *vnode, struct vattr *attr) {
	return devfs_getattr(vnode, attr);
}

struct vnops devfs_vnops = {
	devfs_open2,		/* open */
	devfs_close2,		/* close */
	devfs_read2,		/* read */
	devfs_write2,		/* write */
	((vnop_seek_t)vfscore_vop_nullop),		/* seek */
	devfs_ioctl2,		/* ioctl */
	((vnop_fsync_t)vfscore_vop_nullop),		/* fsync */
	devfs_readdir2,		/* readdir */
	devfs_lookup2,		/* lookup */
	((vnop_create_t)vop_einval),		/* create */
	((vnop_remove_t)vop_einval),		/* remove */
	((vnop_rename_t)vop_einval),		/* remame */
	((vnop_mkdir_t)vop_einval),		/* mkdir */
	((vnop_rmdir_t)vop_einval),		/* rmdir */
	devfs_getattr2,		/* getattr */
	((vnop_setattr_t)vop_eperm),		/* setattr */
	((vnop_inactive_t)vfscore_vop_nullop),		/* inactive */
	((vnop_truncate_t)vfscore_vop_nullop),		/* truncate */
	((vnop_link_t)vop_eperm),		/* link */
	(vnop_cache_t) NULL, /* arc */
	((vnop_fallocate_t)vfscore_vop_nullop),	/* fallocate */
	((vnop_readlink_t)vfscore_vop_nullop),		/* read link */
	((vnop_symlink_t)vfscore_vop_nullop),		/* symbolic link */
};


/*
 * File system operations
 */
struct vfsops devfs_vfsops = {
	((vfsop_mount_t)vfscore_nullop),		/* mount */
	devfs_unmount2,		/* unmount */
	((vfsop_sync_t)vfscore_nullop),		/* sync */
	((vfsop_vget_t)vfscore_nullop),		/* vget */
	((vfsop_statfs_t)vfscore_nullop),		/* statfs */
	&devfs_vnops,		/* vnops */
};

struct vfscore_fs_type fs_devfs2 = {
	.vs_name = "devfs",
	.vs_init = NULL,
	.vs_op = &devfs_vfsops,
};

#endif


void add_fs() {
	fses_cnt = 0;
#ifdef CONFIG_LIBDEVFS
	fses[fses_cnt++] = &fs_devfs2;
#endif
#ifdef CONFIG_LIBRAMFS
	fses[fses_cnt++] = &fs_ramfs2;
#endif
	uk_fslist_end2 = &fses[fses_cnt];
}


#define for_each_fs(iter)			\
	for (iter = uk_fslist_start2;	\
	     iter < uk_fslist_end2;		\
	     iter++)

#else

extern const struct vfscore_fs_type *uk_fslist_start;
extern const struct vfscore_fs_type *uk_fslist_end;

#define for_each_fs(iter)			\
	for (iter = &uk_fslist_start;	\
	     iter < &uk_fslist_end;		\
	     iter++)
#endif

/*
 * Lookup file system.
 */
static const struct vfscore_fs_type *
fs_getfs(const char *name)
{
	const struct vfscore_fs_type *fs = NULL, **__fs;

	UK_ASSERT(name != NULL);

	for_each_fs(__fs) {
		fs = *__fs;
		if (!fs || !fs->vs_name)
			continue;

		if (strncmp(name, fs->vs_name, FSMAXNAMES) == 0)
			return fs;
	}

	return NULL;
}

int device_open(const char *name __unused, int mode __unused,
		struct device **devp __unused)
{
	UK_CRASH("%s is not implemented (%s)\n", __func__, name);
	return 0;
}

int device_close(struct device *dev)
{
	(void) dev;
	UK_CRASH("%s not implemented", __func__);
	return 0;
}

int
mount(const char *dev, const char *dir, const char *fsname, unsigned long flags,
      const void *data)
{
	const struct vfscore_fs_type *fs;
	struct mount *mp;
	struct device *device;
	struct dentry *dp_covered = NULL;
	struct vnode *vp = NULL;
	int error;

	uk_pr_crit("VFS: mounting %s at %s\n", fsname, dir);

	if (!dir || *dir == '\0')
		return ENOENT;

	/* Find a file system. */
	if (!(fs = fs_getfs(fsname)))
		return ENODEV;  /* No such file system */

	/* Open device. NULL can be specified as a device. */
	// Allow device_open() to fail, in which case dev is interpreted
	// by the file system mount routine (e.g zfs pools)
	device = 0;
	if (dev && strncmp(dev, "/dev/", 5) == 0)
		device_open(dev + 5, DO_RDWR, &device);

	/* Check if device or directory has already been mounted. */
	// We need to avoid the situation where after we already verified that
	// the mount point is free, but before we actually add it to mount_list,
	// another concurrent mount adds it. So we use a new mutex to ensure
	// that only one mount() runs at a time. We cannot reuse the existing
	// mount_lock for this purpose: If we take mount_lock and then do
	// lookups, this is lock order inversion and can result in deadlock.

	/* TODO: protect the function from reentrance, as described in
	 * the comment above */
	/* static mutex sys_mount_lock; */
	/* SCOPE_LOCK(sys_mount_lock); */

	uk_mutex_lock(&mount_lock);
	uk_list_for_each_entry(mp, &mount_list, mnt_list) {
		if (!strcmp(mp->m_path, dir) ||
		    (device && mp->m_dev == device)) {
			error = EBUSY;  /* Already mounted */
			uk_mutex_unlock(&mount_lock);
			goto err1;
		}
	}
	uk_mutex_unlock(&mount_lock);
	/*
	 * Create VFS mount entry.
	 */
	mp = malloc(sizeof(struct mount));
	if (!mp) {
		error = ENOMEM;
		goto err1;
	}
//FEMTO
	vfs_ipc_add_heap(mp, sizeof(struct mount), (1 << 21) | ( 1 << 2));
//
	mp->m_count = 0;
	mp->m_op = fs->vs_op;
	mp->m_flags = flags;
	mp->m_dev = device;
	mp->m_data = NULL;
	strlcpy(mp->m_path, dir, sizeof(mp->m_path));
	strlcpy(mp->m_special, dev, sizeof(mp->m_special));

	/*
	 * Get vnode to be covered in the upper file system.
	 */
	if (*dir == '/' && *(dir + 1) == '\0') {
		/* Ignore if it mounts to global root directory. */
		dp_covered = NULL;
	} else {
		if ((error = namei(dir, &dp_covered)) != 0) {

			error = ENOENT;
			goto err2;
		}
		if (dp_covered->d_vnode->v_type != VDIR) {
			error = ENOTDIR;
			goto err3;
		}
	}
	mp->m_covered = dp_covered;

	/*
	 * Create a root vnode for this file system.
	 */
	vfscore_vget(mp, 0, &vp);
	if (vp == NULL) {
		error = ENOMEM;
		goto err3;
	}

	vp->v_type = VDIR;
	vp->v_flags = VROOT;
	vp->v_mode = S_IFDIR | S_IRUSR | S_IWUSR | S_IXUSR;

	mp->m_root = dentry_alloc(NULL, vp, "/");
	if (!mp->m_root) {
		vput(vp);
		goto err3;
	}
	vput(vp);

	/*
	 * Call a file system specific routine.
	 */
	if ((error = VFS_MOUNT(mp, dev, flags, data)) != 0)
		goto err4;

	if (mp->m_flags & MNT_RDONLY)
		vp->v_mode &=~S_IWUSR;

	/*
	 * Insert to mount list
	 */
	uk_mutex_lock(&mount_lock);
	uk_list_add_tail(&mp->mnt_list, &mount_list);
	uk_mutex_unlock(&mount_lock);

	return 0;   /* success */
 err4:
	drele(mp->m_root);
 err3:
	if (dp_covered)
		drele(dp_covered);
 err2:
	free(mp);
 err1:
	if (device)
		device_close(device);

	return error;
}

void
vfscore_release_mp_dentries(struct mount *mp)
{
	/* Decrement referece count of root vnode */
	if (mp->m_covered) {
		drele(mp->m_covered);
	}

	/* Release root dentry */
	drele(mp->m_root);
}

int
umount2(const char *path, int flags)
{
	struct mount *mp, *tmp;
	int error, pathlen;

	uk_pr_info("VFS: unmounting %s\n", path);

	uk_mutex_lock(&mount_lock);

	pathlen = strlen(path);
	if (pathlen >= MAXPATHLEN) {
		error = ENAMETOOLONG;
		goto out;
	}

	/* Get mount entry */
	uk_list_for_each_entry(tmp, &mount_list, mnt_list) {
		if (!strcmp(path, tmp->m_path)) {
			mp = tmp;
			goto found;
		}
	}

	error = EINVAL;
	goto out;

found:
	/*
	 * Root fs can not be unmounted.
	 */
	if (mp->m_covered == NULL && !(flags & MNT_FORCE)) {
		error = EINVAL;
		goto out;
	}

	if ((error = VFS_UNMOUNT(mp, flags)) != 0)
		goto out;
	uk_list_del_init(&mp->mnt_list);

#ifdef HAVE_BUFFERS
	/* Flush all buffers */
	binval(mp->m_dev);
#endif

	if (mp->m_dev)
		device_close(mp->m_dev);
	free(mp);
 out:
	uk_mutex_unlock(&mount_lock);
	return error;
}

int
umount(const char *path)
{
	return umount2(path, 0);
}

#if 0
int
sys_pivot_root(const char *new_root, const char *put_old)
{
	struct mount *newmp = NULL, *oldmp = NULL;
	int error;

	WITH_LOCK(mount_lock) {
		for (auto&& mp : mount_list) {
			if (!strcmp(mp->m_path, new_root)) {
				newmp = mp;
			}
			if (!strcmp(mp->m_path, put_old)) {
				oldmp = mp;
			}
		}
		if (!newmp || !oldmp || newmp == oldmp) {
			return EINVAL;
		}
		for (auto&& mp : mount_list) {
			if (mp == newmp || mp == oldmp) {
				continue;
			}
			if (!strncmp(mp->m_path, put_old, strlen(put_old))) {
				return EBUSY;
			}
		}
		if ((error = VFS_UNMOUNT(oldmp, 0)) != 0) {
			return error;
		}
		mount_list.remove(oldmp);

		newmp->m_root->d_vnode->v_mount = newmp;

		if (newmp->m_covered) {
			drele(newmp->m_covered);
		}
		newmp->m_covered = NULL;

		if (newmp->m_root->d_parent) {
			drele(newmp->m_root->d_parent);
		}
		newmp->m_root->d_parent = NULL;

		strlcpy(newmp->m_path, "/", sizeof(newmp->m_path));
	}
	return 0;
}
#endif

void sync(void)
{
	struct mount *mp;
	uk_mutex_lock(&mount_lock);

	/* Call each mounted file system. */
	uk_list_for_each_entry(mp, &mount_list, mnt_list) {
		VFS_SYNC(mp);
	}
#ifdef HAVE_BUFFERS
	bio_sync();
#endif
	uk_mutex_unlock(&mount_lock);
}

/*
 * Compare two path strings. Return matched length.
 * @path: target path.
 * @root: vfs root path as mount point.
 */
static size_t
count_match(const char *path, char *mount_root)
{
	size_t len = 0;

	while (*path && *mount_root) {
		if (*path != *mount_root)
			break;

		path++;
		mount_root++;
		len++;
	}
	if (*mount_root != '\0')
		return 0;

	if (len == 1 && *(path - 1) == '/')
		return 1;

	if (*path == '\0' || *path == '/')
		return len;
	return 0;
}

/*
 * Get the root directory and mount point for specified path.
 * @path: full path.
 * @mp: mount point to return.
 * @root: pointer to root directory in path.
 */
int
vfs_findroot(const char *path, struct mount **mp, char **root)
{
	struct mount *m = NULL, *tmp;
	size_t len, max_len = 0;

	if (!path)
		return -1;

	/* Find mount point from nearest path */
	uk_mutex_lock(&mount_lock);
	uk_list_for_each_entry(tmp, &mount_list, mnt_list) {
		len = count_match(path, tmp->m_path);
		if (len > max_len) {
			max_len = len;
			m = tmp;
		}
	}
	uk_mutex_unlock(&mount_lock);
	if (m == NULL)
		return -1;
	*root = (char *)(path + max_len);
	if (**root == '/')
		(*root)++;
	*mp = m;
	return 0;
}

/*
 * Mark a mount point as busy.
 */
void
vfs_busy(struct mount *mp)
{
	/* The m_count is not really checked anywhere
	 * currently. Atomic is enough. But it could be that obtaining
	 * mount_lock will be needed in the future */
	ukarch_inc(&mp->m_count);
}


/*
 * Mark a mount point as busy.
 */
void
vfs_unbusy(struct mount *mp)
{
	/* The m_count is not really checked anywhere
	 * currently. Atomic is enough. But it could be that obtaining
	 * mount_lock will be needed in the future */
	ukarch_dec(&mp->m_count);
}

int vfscore_nullop(void)
{
	return 0;
}

int
vfs_einval(void)
{
	return EINVAL;
}

#ifdef DEBUG_VFS
void
vfscore_mount_dump(void)
{
	struct mount *mp;
	uk_mutex_lock(&mount_lock);

	uk_pr_debug("vfscore_mount_dump\n");
	uk_pr_debug("dev      count root\n");
	uk_pr_debug("-------- ----- --------\n");

	uk_list_for_each_entry(mp, &mount_list, mnt_list) {
		uk_pr_debug("%8p %5d %s\n", mp->m_dev, mp->m_count, mp->m_path);
	}
	uk_mutex_unlock(&mount_lock);
}
#endif

#ifdef CONFIG_LIBLWIP

extern int sock_net_close(struct vnode *s_vnode,
			struct vfscore_file *vfscore_file);
extern int sock_net_write(struct vnode *s_vnode,
			struct uio *buf, int ioflag __unused);
extern int sock_net_read(struct vnode *s_vnode,
			struct vfscore_file *vfscore_file __unused,
			struct uio *buf, int ioflag __unused);
extern int sock_net_ioctl(struct vnode *s_vnode,
			struct vfscore_file *vfscore_file __unused,
			unsigned long request,
			void *buf);

int sock_net_close_wrap(struct vnode *s_vnode,
			struct vfscore_file *vfscore_file) {
	return sock_net_close(s_vnode, vfscore_file);
}

int sock_net_write_wrap(struct vnode *s_vnode,
			struct uio *buf, int ioflag __unused) {
	return sock_net_write(s_vnode, buf, ioflag);
}

int sock_net_read_wrap(struct vnode *s_vnode,
			struct vfscore_file *vfscore_file __unused,
			struct uio *buf, int ioflag __unused) {
	return sock_net_read(s_vnode, vfscore_file, buf, ioflag);
}

int sock_net_ioctl_wrap(struct vnode *s_vnode,
			struct vfscore_file *vfscore_file __unused,
			unsigned long request,
			void *buf) {
	return sock_net_ioctl(s_vnode, vfscore_file, request, buf);
}
#endif