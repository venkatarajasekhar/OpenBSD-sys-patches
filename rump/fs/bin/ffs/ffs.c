/*	$NetBSD: ffs.c,v 1.6 2007/11/07 15:51:07 pooka Exp $	*/

/*
 * Copyright (c) 2007 Antti Kantee.  All Rights Reserved.
 *
 * Development of this software was supported by Google Summer of Code.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/types.h>
#include <sys/mount.h>

#include <ufs/ufs/ufsmount.h>

#include <err.h>
#include <puffs.h>
#include <stdlib.h>
#include <unistd.h>

#include "p2k.h"

/* XXX: figure out a proper way to share code/integrate with mount_ffs */
static const struct mntopt ffsmopts[] = {
	MOPT_STDOPTS,
	MOPT_ASYNC,
	MOPT_SYNC,
	MOPT_UPDATE,
	MOPT_RELOAD,
	MOPT_NOATIME,
	MOPT_NODEVMTIME,
	MOPT_FORCE,
	MOPT_SOFTDEP,
	MOPT_GETARGS,
	MOPT_NULL,
};

static void
usage(void)
{

	errx(1, "usage: %s [-o opts] dev mountpath", getprogname());
}

int
main(int argc, char *argv[])
{
	struct ufs_args args;
	mntoptparse_t mp;
	int mntflags, pflags;
	int rv, ch;

	setprogname(argv[0]);
	getmnt_silent = 1;

	mntflags = pflags = 0;
	while ((ch = getopt(argc, argv, "o:")) != -1) {
		switch (ch) {
		case 'o':
			mp = getmntopts(optarg, puffsmopts, &mntflags, &pflags);
			if (mp == NULL) {
				mp = getmntopts(optarg, ffsmopts, &mntflags, 0);
				if (mp == NULL)
					err(1, "getmntops");
			}
			freemntopts(mp);
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	}
	argc -= optind;
	argv += optind;
	if (argc != 2)
		usage();

	memset(&args, 0, sizeof(args));
	args.fspec = argv[0];

	rv = p2k_run_fs(MOUNT_FFS, argv[0], argv[1], mntflags,
	    &args, sizeof(args), pflags);
	if (rv)
		err(1, "mount");

	return 0;
}
