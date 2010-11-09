/*	$NetBSD: advfsops.c,v 1.32 2007/01/04 15:42:37 elad Exp $	*/

/*
 * Copyright (c) 1994 Christian E. Hopps
 * Copyright (c) 1996 Matthias Scheler
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Christian E. Hopps.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: advfsops.c,v 1.32 2007/01/04 15:42:37 elad Exp $");

#if defined(_KERNEL_OPT)
#include "opt_compat_netbsd.h"
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/sysctl.h>
#include <sys/vnode.h>
#include <sys/mount.h>
#include <sys/proc.h>
#include <sys/time.h>
#include <sys/malloc.h>
#include <sys/pool.h>
#include <sys/disklabel.h>
#include <miscfs/specfs/specdev.h> /* XXX */
#include <sys/fcntl.h>
#include <sys/namei.h>
#include <sys/ioctl.h>
#include <sys/queue.h>
#include <sys/buf.h>
#include <sys/conf.h>
#include <sys/kauth.h>
#include <fs/adosfs/adosfs.h>

void adosfs_init __P((void));
void adosfs_reinit __P((void));
void adosfs_done __P((void));
int adosfs_mount __P((struct mount *, const char *, void *, struct nameidata *,
		      struct lwp *));
int adosfs_start __P((struct mount *, int, struct lwp *));
int adosfs_unmount __P((struct mount *, int, struct lwp *));
int adosfs_root __P((struct mount *, struct vnode **));
int adosfs_quotactl __P((struct mount *, int, uid_t, void *, struct lwp *));
int adosfs_statvfs __P((struct mount *, struct statvfs *, struct lwp *));
int adosfs_sync __P((struct mount *, int, kauth_cred_t, struct lwp *));
int adosfs_vget __P((struct mount *, ino_t, struct vnode **));
int adosfs_fhtovp __P((struct mount *, struct fid *, struct vnode **));
int adosfs_vptofh __P((struct vnode *, struct fid *, size_t *));

int adosfs_mountfs __P((struct vnode *, struct mount *, struct lwp *));
int adosfs_loadbitmap __P((struct adosfsmount *));

struct simplelock adosfs_hashlock;

POOL_INIT(adosfs_node_pool, sizeof(struct anode), 0, 0, 0, "adosndpl",
    &pool_allocator_nointr);

MALLOC_DEFINE(M_ADOSFSMNT, "adosfs mount", "adosfs mount structures");
MALLOC_DEFINE(M_ANODE, "adosfs anode", "adosfs anode structures and tables");
MALLOC_DEFINE(M_ADOSFSBITMAP, "adosfs bitmap", "adosfs bitmap");

static const struct genfs_ops adosfs_genfsops = {
	.gop_size = genfs_size,
};

int (**adosfs_vnodeop_p) __P((void *));

int
adosfs_mount(mp, path, data, ndp, l)
	struct mount *mp;
	const char *path;
	void *data;
	struct nameidata *ndp;
	struct lwp *l;
{
	struct vnode *devvp;
	struct adosfs_args args;
	struct adosfsmount *amp;
	int error;
	mode_t accessmode;

	if (mp->mnt_flag & MNT_GETARGS) {
		amp = VFSTOADOSFS(mp);
		if (amp == NULL)
			return EIO;
		args.uid = amp->uid;
		args.gid = amp->gid;
		args.mask = amp->mask;
		args.fspec = NULL;
		return copyout(&args, data, sizeof(args));
	}
	error = copyin(data, &args, sizeof(struct adosfs_args));
	if (error)
		return(error);

	if ((mp->mnt_flag & MNT_RDONLY) == 0)
		return (EROFS);

	if ((mp->mnt_flag & MNT_UPDATE) && args.fspec == NULL)
		return EOPNOTSUPP;

	/*
	 * Not an update, or updating the name: look up the name
	 * and verify that it refers to a sensible block device.
	 */
	NDINIT(ndp, LOOKUP, FOLLOW, UIO_USERSPACE, args.fspec, l);
	if ((error = namei(ndp)) != 0)
		return (error);
	devvp = ndp->ni_vp;

	if (devvp->v_type != VBLK) {
		vrele(devvp);
		return (ENOTBLK);
	}
	if (bdevsw_lookup(devvp->v_rdev) == NULL) {
		vrele(devvp);
		return (ENXIO);
	}
	/*
	 * If mount by non-root, then verify that user has necessary
	 * permissions on the device.
	 */
	if (kauth_authorize_generic(l->l_cred, KAUTH_GENERIC_ISSUSER, NULL)) {
		accessmode = VREAD;
		if ((mp->mnt_flag & MNT_RDONLY) == 0)
			accessmode |= VWRITE;
		vn_lock(devvp, LK_EXCLUSIVE | LK_RETRY);
		error = VOP_ACCESS(devvp, accessmode, l->l_cred, l);
		if (error) {
			vput(devvp);
			return (error);
		}
		VOP_UNLOCK(devvp, 0);
	}
/* MNT_UPDATE? */
	if ((error = adosfs_mountfs(devvp, mp, l)) != 0) {
		vrele(devvp);
		return (error);
	}
	amp = VFSTOADOSFS(mp);
	amp->uid = args.uid;
	amp->gid = args.gid;
	amp->mask = args.mask;
	return set_statvfs_info(path, UIO_USERSPACE, args.fspec, UIO_USERSPACE,
	    mp, l);
}

int
adosfs_mountfs(devvp, mp, l)
	struct vnode *devvp;
	struct mount *mp;
	struct lwp *l;
{
	struct disklabel dl;
	struct partition *parp;
	struct adosfsmount *amp;
	struct buf *bp;
	struct vnode *rvp;
	int error, part, i;

	part = DISKPART(devvp->v_rdev);
	amp = NULL;

	/*
	 * Disallow multiple mounts of the same device.
	 * Disallow mounting of a device that is currently in use
	 * (except for root, which might share swap device for miniroot).
	 * Flush out any old buffers remaining from a previous use.
	 */
	if ((error = vfs_mountedon(devvp)) != 0)
		return (error);
	if (vcount(devvp) > 1 && devvp != rootvp)
		return (EBUSY);
	if ((error = vinvalbuf(devvp, V_SAVE, l->l_cred, l, 0, 0))
	    != 0)
		return (error);

	/*
	 * open blkdev and read root block
	 */
	if ((error = VOP_OPEN(devvp, FREAD, NOCRED, l)) != 0)
		return (error);
	error = VOP_IOCTL(devvp, DIOCGDINFO, &dl, FREAD, NOCRED, l);
	if (error)
		goto fail;

	parp = &dl.d_partitions[part];
	amp = malloc(sizeof(struct adosfsmount), M_ADOSFSMNT, M_WAITOK);
	memset((char *)amp, 0, (u_long)sizeof(struct adosfsmount));
	amp->mp = mp;
	if (dl.d_type == DTYPE_FLOPPY) {
		amp->bsize = dl.d_secsize;
		amp->secsperblk = 1;
	}
	else {
		amp->bsize = parp->p_fsize * parp->p_frag;
		amp->secsperblk = parp->p_frag;
	}

	/* invalid fs ? */
	if (amp->secsperblk == 0) {
		error = EINVAL;
		goto fail;
	}

	bp = NULL;
	if ((error = bread(devvp, (daddr_t)BBOFF,
			   amp->bsize, NOCRED, &bp)) != 0) {
		brelse(bp);
		goto fail;
	}
	amp->dostype = adoswordn(bp, 0);
	brelse(bp);

	/* basic sanity checks */
	if (amp->dostype < 0x444f5300 || amp->dostype > 0x444f5305) {
		error = EINVAL;
		goto fail;
	}

	amp->rootb = (parp->p_size / amp->secsperblk - 1 + parp->p_cpg) >> 1;
	amp->numblks = parp->p_size / amp->secsperblk - parp->p_cpg;

	amp->nwords = amp->bsize >> 2;
	amp->dbsize = amp->bsize - (IS_FFS(amp) ? 0 : OFS_DATA_OFFSET);
	amp->devvp = devvp;

	mp->mnt_data = amp;
	mp->mnt_stat.f_fsidx.__fsid_val[0] = (long)devvp->v_rdev;
	mp->mnt_stat.f_fsidx.__fsid_val[1] = makefstype(MOUNT_ADOSFS);
	mp->mnt_stat.f_fsid = mp->mnt_stat.f_fsidx.__fsid_val[0];
	mp->mnt_stat.f_namemax = ADMAXNAMELEN;
	mp->mnt_fs_bshift = ffs(amp->bsize) - 1;
	mp->mnt_dev_bshift = DEV_BSHIFT;	/* XXX */
	mp->mnt_flag |= MNT_LOCAL;

	/*
	 * init anode table.
	 */
	for (i = 0; i < ANODEHASHSZ; i++)
		LIST_INIT(&amp->anodetab[i]);

	/*
	 * get the root anode, if not a valid fs this will fail.
	 */
	if ((error = VFS_ROOT(mp, &rvp)) != 0)
		goto fail;
	/* allocate and load bitmap, set free space */
	amp->bitmap = malloc(((amp->numblks + 31) / 32) * sizeof(*amp->bitmap),
	    M_ADOSFSBITMAP, M_WAITOK);
	if (amp->bitmap)
		adosfs_loadbitmap(amp);
	if (mp->mnt_flag & MNT_RDONLY && amp->bitmap) {
		/*
		 * Don't need the bitmap any more if it's read-only.
		 */
		free(amp->bitmap, M_ADOSFSBITMAP);
		amp->bitmap = NULL;
	}
	vput(rvp);

	return(0);

fail:
	vn_lock(devvp, LK_EXCLUSIVE | LK_RETRY);
	(void) VOP_CLOSE(devvp, FREAD, NOCRED, l);
	VOP_UNLOCK(devvp, 0);
	if (amp && amp->bitmap)
		free(amp->bitmap, M_ADOSFSBITMAP);
	if (amp)
		free(amp, M_ADOSFSMNT);
	return (error);
}

int
adosfs_start(mp, flags, l)
	struct mount *mp;
	int flags;
	struct lwp *l;
{

	return (0);
}

int
adosfs_unmount(mp, mntflags, l)
	struct mount *mp;
	int mntflags;
	struct lwp *l;
{
	struct adosfsmount *amp;
	int error, flags;

	flags = 0;
	if (mntflags & MNT_FORCE)
		flags |= FORCECLOSE;
	if ((error = vflush(mp, NULLVP, flags)) != 0)
		return (error);
	amp = VFSTOADOSFS(mp);
	if (amp->devvp->v_type != VBAD)
		amp->devvp->v_specmountpoint = NULL;
	vn_lock(amp->devvp, LK_EXCLUSIVE | LK_RETRY);
	error = VOP_CLOSE(amp->devvp, FREAD, NOCRED, l);
	vput(amp->devvp);
	if (amp->bitmap)
		free(amp->bitmap, M_ADOSFSBITMAP);
	free(amp, M_ADOSFSMNT);
	mp->mnt_data = NULL;
	mp->mnt_flag &= ~MNT_LOCAL;
	return (error);
}

int
adosfs_root(mp, vpp)
	struct mount *mp;
	struct vnode **vpp;
{
	struct vnode *nvp;
	int error;

	if ((error = VFS_VGET(mp, (ino_t)VFSTOADOSFS(mp)->rootb, &nvp)) != 0)
		return (error);
	/* XXX verify it's a root block? */
	*vpp = nvp;
	return (0);
}

int
adosfs_statvfs(mp, sbp, l)
	struct mount *mp;
	struct statvfs *sbp;
	struct lwp *l;
{
	struct adosfsmount *amp;

	amp = VFSTOADOSFS(mp);
	sbp->f_bsize = amp->bsize;
	sbp->f_frsize = amp->bsize;
	sbp->f_iosize = amp->dbsize;
	sbp->f_blocks = amp->numblks;
	sbp->f_bfree = amp->freeblks;
	sbp->f_bavail = amp->freeblks;
	sbp->f_bresvd = 0;
	sbp->f_files = 0;		/* who knows */
	sbp->f_ffree = 0;		/* " " */
	sbp->f_favail = 0;		/* " " */
	sbp->f_fresvd = 0;
	copy_statvfs_info(sbp, mp);
	return (0);
}

/*
 * lookup an anode, check mount's hash table if not found, create
 * return locked and referenced al la vget(vp, 1);
 */
int
adosfs_vget(mp, an, vpp)
	struct mount *mp;
	ino_t an;
	struct vnode **vpp;
{
	struct adosfsmount *amp;
	struct vnode *vp;
	struct anode *ap;
	struct buf *bp;
	char *nam, *tmp;
	int namlen, error;

	error = 0;
	amp = VFSTOADOSFS(mp);
	bp = NULL;

	/*
	 * check hash table. we are done if found
	 */
	if ((*vpp = adosfs_ahashget(mp, an)) != NULL)
		return (0);

	error = getnewvnode(VT_ADOSFS, mp, adosfs_vnodeop_p, &vp);
	if (error)
		return (error);

	/*
	 * setup, insert in hash, and lock before io.
	 */
	vp->v_data = ap = pool_get(&adosfs_node_pool, PR_WAITOK);
	memset(ap, 0, sizeof(struct anode));
	ap->vp = vp;
	ap->amp = amp;
	ap->block = an;
	ap->nwords = amp->nwords;
	adosfs_ainshash(amp, ap);

	if ((error = bread(amp->devvp, an * amp->bsize / DEV_BSIZE,
			   amp->bsize, NOCRED, &bp)) != 0) {
		brelse(bp);
		vput(vp);
		return (error);
	}

	/*
	 * get type and fill rest in based on that.
	 */
	switch (ap->type = adosfs_getblktype(amp, bp)) {
	case AROOT:
		vp->v_type = VDIR;
		vp->v_flag |= VROOT;
		ap->mtimev.days = adoswordn(bp, ap->nwords - 10);
		ap->mtimev.mins = adoswordn(bp, ap->nwords - 9);
		ap->mtimev.ticks = adoswordn(bp, ap->nwords - 8);
		ap->created.days = adoswordn(bp, ap->nwords - 7);
		ap->created.mins = adoswordn(bp, ap->nwords - 6);
		ap->created.ticks = adoswordn(bp, ap->nwords - 5);
		break;
	case ALDIR:
	case ADIR:
		vp->v_type = VDIR;
		break;
	case ALFILE:
	case AFILE:
		vp->v_type = VREG;
		ap->fsize = adoswordn(bp, ap->nwords - 47);
		break;
	case ASLINK:		/* XXX soft link */
		vp->v_type = VLNK;
		/*
		 * convert from BCPL string and
		 * from: "part:dir/file" to: "/part/dir/file"
		 */
		nam = bp->b_data + (6 * sizeof(long));
		namlen = strlen(nam);
		tmp = nam;
		while (*tmp && *tmp != ':')
			tmp++;
		if (*tmp == 0) {
			ap->slinkto = malloc(namlen + 1, M_ANODE, M_WAITOK);
			memcpy(ap->slinkto, nam, namlen);
		} else if (*nam == ':') {
			ap->slinkto = malloc(namlen + 1, M_ANODE, M_WAITOK);
			memcpy(ap->slinkto, nam, namlen);
			ap->slinkto[0] = '/';
		} else {
			ap->slinkto = malloc(namlen + 2, M_ANODE, M_WAITOK);
			ap->slinkto[0] = '/';
			memcpy(&ap->slinkto[1], nam, namlen);
			ap->slinkto[tmp - nam + 1] = '/';
			namlen++;
		}
		ap->slinkto[namlen] = 0;
		ap->fsize = namlen;
		break;
	default:
		brelse(bp);
		vput(vp);
		return (EINVAL);
	}

	/*
	 * Get appropriate data from this block;  hard link needs
	 * to get other data from the "real" block.
	 */

	/*
	 * copy in name (from original block)
	 */
	nam = bp->b_data + (ap->nwords - 20) * sizeof(u_int32_t);
	namlen = *(u_char *)nam++;
	if (namlen > 30) {
#ifdef DIAGNOSTIC
		printf("adosfs: aget: name length too long blk %llu\n",
		    (unsigned long long)an);
#endif
		brelse(bp);
		vput(vp);
		return (EINVAL);
	}
	memcpy(ap->name, nam, namlen);
	ap->name[namlen] = 0;

	/*
	 * if dir alloc hash table and copy it in
	 */
	if (vp->v_type == VDIR) {
		int i;

		ap->tab = malloc(ANODETABSZ(ap) * 2, M_ANODE, M_WAITOK);
		ap->ntabent = ANODETABENT(ap);
		ap->tabi = (int *)&ap->tab[ap->ntabent];
		memset(ap->tabi, 0, ANODETABSZ(ap));
		for (i = 0; i < ap->ntabent; i++)
			ap->tab[i] = adoswordn(bp, i + 6);
	}

	/*
	 * misc.
	 */
	ap->pblock = adoswordn(bp, ap->nwords - 3);
	ap->hashf = adoswordn(bp, ap->nwords - 4);
	ap->linknext = adoswordn(bp, ap->nwords - 10);
	ap->linkto = adoswordn(bp, ap->nwords - 11);

	/*
	 * setup last indirect block cache.
	 */
	ap->lastlindblk = 0;
	if (ap->type == AFILE)  {
		ap->lastindblk = ap->block;
		if (adoswordn(bp, ap->nwords - 10))
			ap->linkto = ap->block;
	} else if (ap->type == ALFILE) {
		ap->lastindblk = ap->linkto;
		brelse(bp);
		bp = NULL;
		error = bread(amp->devvp, ap->linkto * amp->bsize / DEV_BSIZE,
		    amp->bsize, NOCRED, &bp);
		if (error) {
			brelse(bp);
			vput(vp);
			return (error);
		}
		ap->fsize = adoswordn(bp, ap->nwords - 47);
		/*
		 * Should ap->block be set to the real file header block?
		 */
		ap->block = ap->linkto;
	}

	if (ap->type == AROOT) {
		ap->adprot = 15;
		ap->uid = amp->uid;
		ap->gid = amp->gid;
	} else {
		ap->adprot = adoswordn(bp, ap->nwords - 48) ^ 15;
		/*
		 * ADOS directories do not have a `x' protection bit as
		 * it is known in VFS; this functionality is fulfilled
		 * by the ADOS `r' bit.
		 *
		 * To retain the ADOS behaviour, fake execute permissions
		 * in that case.
		 */
		if ((ap->type == ADIR || ap->type == ALDIR) &&
		    (ap->adprot & 0x00000008) == 0)
			ap->adprot &= ~0x00000002;

		/*
		 * Get uid/gid from extensions in file header
		 * (really need to know if this is a muFS partition)
		 */
		ap->uid = (adoswordn(bp, ap->nwords - 49) >> 16) & 0xffff;
		ap->gid = adoswordn(bp, ap->nwords - 49) & 0xffff;
		if (ap->uid || ap->gid) {
			if (ap->uid == 0xffff)
				ap->uid = 0;
			if (ap->gid == 0xffff)
				ap->gid = 0;
			ap->adprot |= 0x40000000;	/* Kludge */
		}
		else {
			/*
			 * uid & gid extension don't exist,
			 * so use the mount-point uid/gid
			 */
			ap->uid = amp->uid;
			ap->gid = amp->gid;
		}
	}
	ap->mtime.days = adoswordn(bp, ap->nwords - 23);
	ap->mtime.mins = adoswordn(bp, ap->nwords - 22);
	ap->mtime.ticks = adoswordn(bp, ap->nwords - 21);

	genfs_node_init(vp, &adosfs_genfsops);
	*vpp = vp;
	brelse(bp);
	vp->v_size = ap->fsize;
	return (0);
}

/*
 * Load the bitmap into memory, and count the number of available
 * blocks.
 * The bitmap will be released if the filesystem is read-only;  it's
 * only needed to find the free space.
 */
int
adosfs_loadbitmap(amp)
	struct adosfsmount *amp;
{
	struct buf *bp, *mapbp;
	u_long bn;
	int blkix, endix, mapix;
	int bmsize;
	int error;

	bp = mapbp = NULL;
	bn = amp->rootb;
	if ((error = bread(amp->devvp, bn * amp->bsize / DEV_BSIZE, amp->bsize,
	    NOCRED, &bp)) != 0) {
		brelse(bp);
		return (error);
	}
	blkix = amp->nwords - 49;
	endix = amp->nwords - 24;
	mapix = 0;
	bmsize = (amp->numblks + 31) / 32;
	while (mapix < bmsize) {
		int n;
		u_long bits;

		if (adoswordn(bp, blkix) == 0)
			break;
		if (mapbp != NULL)
			brelse(mapbp);
		if ((error = bread(amp->devvp,
		    adoswordn(bp, blkix) * amp->bsize / DEV_BSIZE, amp->bsize,
		     NOCRED, &mapbp)) != 0)
			break;
		if (adoscksum(mapbp, amp->nwords)) {
#ifdef DIAGNOSTIC
			printf("adosfs: loadbitmap - cksum of blk %d failed\n",
			    adoswordn(bp, blkix));
#endif
			/* XXX Force read-only?  Set free space 0? */
			break;
		}
		n = 1;
		while (n < amp->nwords && mapix < bmsize) {
			amp->bitmap[mapix++] = bits = adoswordn(mapbp, n);
			++n;
			if (mapix == bmsize && amp->numblks & 31)
				bits &= ~(0xffffffff << (amp->numblks & 31));
			while (bits) {
				if (bits & 1)
					++amp->freeblks;
				bits >>= 1;
			}
		}
		++blkix;
		if (mapix < bmsize && blkix == endix) {
			bn = adoswordn(bp, blkix);
			brelse(bp);
			if ((error = bread(amp->devvp, bn * amp->bsize / DEV_BSIZE,
			    amp->bsize, NOCRED, &bp)) != 0)
				break;
			/*
			 * Why is there no checksum on these blocks?
			 */
			blkix = 0;
			endix = amp->nwords - 1;
		}
	}
	if (bp)
		brelse(bp);
	if (mapbp)
		brelse(mapbp);
	return (error);
}


/*
 * File handle to vnode
 *
 * Have to be really careful about stale file handles:
 * - check that the inode number is in range
 * - call iget() to get the locked inode
 * - check for an unallocated inode (i_mode == 0)
 * - check that the generation number matches
 */

struct ifid {
	ushort	ifid_len;
	ushort	ifid_pad;
	int	ifid_ino;
	long	ifid_start;
};

int
adosfs_fhtovp(mp, fhp, vpp)
	struct mount *mp;
	struct fid *fhp;
	struct vnode **vpp;
{
	struct ifid ifh;
#if 0
	struct anode *ap;
#endif
	struct vnode *nvp;
	int error;

	if (fhp->fid_len != sizeof(struct ifid))
		return EINVAL;

#ifdef ADOSFS_DIAGNOSTIC
	printf("adfhtovp(%x, %x, %x)\n", mp, fhp, vpp);
#endif

	memcpy(&ifh, fhp, sizeof(ifh));

	if ((error = VFS_VGET(mp, ifh.ifid_ino, &nvp)) != 0) {
		*vpp = NULLVP;
		return (error);
	}
#if 0
	ap = VTOA(nvp);
	if (ap->inode.iso_mode == 0) {
		vput(nvp);
		*vpp = NULLVP;
		return (ESTALE);
	}
#endif
	*vpp = nvp;
	return(0);
}

int
adosfs_vptofh(vp, fhp, fh_size)
	struct vnode *vp;
	struct fid *fhp;
	size_t *fh_size;
{
	struct anode *ap = VTOA(vp);
	struct ifid ifh;

	if (*fh_size < sizeof(struct ifid)) {
		*fh_size = sizeof(struct ifid);
		return E2BIG;
	}
	*fh_size = sizeof(struct ifid);

	memset(&ifh, 0, sizeof(ifh));
	ifh.ifid_len = sizeof(struct ifid);
	ifh.ifid_ino = ap->block;
	ifh.ifid_start = ap->block;
	memcpy(fhp, &ifh, sizeof(ifh));

#ifdef ADOSFS_DIAGNOSTIC
	printf("advptofh(%x, %x)\n", vp, fhp);
#endif
	return(0);
}

int
adosfs_quotactl(mp, cmds, uid, arg, l)
	struct mount *mp;
	int cmds;
	uid_t uid;
	void *arg;
	struct lwp *l;
{
	return(EOPNOTSUPP);
}

int
adosfs_sync(mp, waitfor, uc, l)
	struct mount *mp;
	int waitfor;
	kauth_cred_t uc;
	struct lwp *l;
{
#ifdef ADOSFS_DIAGNOSTIC
	printf("ad_sync(%x, %x)\n", mp, waitfor);
#endif
	return(0);
}

void
adosfs_init()
{
#ifdef _LKM
	malloc_type_attach(M_ADOSFSMNT);
	malloc_type_attach(M_ANODE);
	malloc_type_attach(M_ADOSFSBITMAP);
	pool_init(&adosfs_node_pool, sizeof(struct anode), 0, 0, 0, "adosndpl",
	    &pool_allocator_nointr);
#endif
	simple_lock_init(&adosfs_hashlock);
}

void
adosfs_done()
{
#ifdef _LKM
	pool_destroy(&adosfs_node_pool);
	malloc_type_detach(M_ADOSFSBITMAP);
	malloc_type_detach(M_ANODE);
	malloc_type_detach(M_ADOSFSMNT);
#endif
}

SYSCTL_SETUP(sysctl_vfs_adosfs_setup, "sysctl vfs.adosfs subtree setup")
{

	sysctl_createv(clog, 0, NULL, NULL,
		       CTLFLAG_PERMANENT,
		       CTLTYPE_NODE, "vfs", NULL,
		       NULL, 0, NULL, 0,
		       CTL_VFS, CTL_EOL);
	sysctl_createv(clog, 0, NULL, NULL,
		       CTLFLAG_PERMANENT,
		       CTLTYPE_NODE, "adosfs",
		       SYSCTL_DESCR("AmigaDOS file system"),
		       NULL, 0, NULL, 0,
		       CTL_VFS, 16, CTL_EOL);
	/*
	 * XXX the "16" above could be dynamic, thereby eliminating
	 * one more instance of the "number to vfs" mapping problem,
	 * but "16" is the order as taken from sys/mount.h
	 */
}

/*
 * vfs generic function call table
 */

extern const struct vnodeopv_desc adosfs_vnodeop_opv_desc;

const struct vnodeopv_desc *adosfs_vnodeopv_descs[] = {
	&adosfs_vnodeop_opv_desc,
	NULL,
};

struct vfsops adosfs_vfsops = {
	MOUNT_ADOSFS,
	adosfs_mount,
	adosfs_start,
	adosfs_unmount,
	adosfs_root,
	adosfs_quotactl,
	adosfs_statvfs,
	adosfs_sync,
	adosfs_vget,
	adosfs_fhtovp,
	adosfs_vptofh,
	adosfs_init,
	NULL,
	adosfs_done,
	NULL,				/* vfs_mountroot */
	(int (*)(struct mount *, struct vnode *, struct timespec *)) eopnotsupp,
	vfs_stdextattrctl,
	adosfs_vnodeopv_descs,
	0,
	{ NULL, NULL },
};
VFS_ATTACH(adosfs_vfsops);
