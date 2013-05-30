/* $OpenBSD$ */
/*
 * Copyright (c) 2012-2013 Sylvestre Gallon <ccna.syl@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __FUSE_NODE_H__
#define __FUSE_NODE_H__

#include <ufs/ufs/quota.h>
#include <ufs/ufs/inode.h>
#include <ufs/ufs/ufs_extern.h>

enum fufh_type {
	FUFH_INVALID = -1,
	FUFH_RDONLY  = 0,
	FUFH_WRONLY  = 1,
	FUFH_RDWR    = 2,
	FUFH_MAXTYPE = 3,
};

struct fuse_filehandle {
	uint64_t fh_id;
	enum fufh_type fh_type;
};

struct fuse_node {
	struct inode ufs_ino;

	/* fd of fuse session and parent ino_t*/
	uint64_t parent;

	/** I/O **/
	struct     fuse_filehandle fufh[FUFH_MAXTYPE];

	/** meta **/
	struct vattr      cached_attrs;
	off_t             filesize;
	uint64_t          nlookup;
	enum vtype        vtype;
};

#ifdef ITOV
# undef ITOV
#endif
#define ITOV(ip) ((ip)->ufs_ino.i_vnode)

#ifdef VTOI
# undef VTOI
#endif
#define VTOI(vp) ((struct fuse_node *)(vp)->v_data)

uint64_t fuse_fd_get(struct fuse_node *, enum fufh_type);

#endif /* __FUSE_NODE_H__ */
