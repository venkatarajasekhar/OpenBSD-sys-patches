/*	$NetBSD: autoconf.c,v 1.17 2006/01/13 22:18:59 uwe Exp $	*/

/*
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department and Ralph Campbell.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * from: Utah Hdr: autoconf.c 1.31 91/01/21
 *
 *	@(#)autoconf.c	8.1 (Berkeley) 6/10/93
 */
/*
 * Copyright (c) 1988 University of Utah.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department and Ralph Campbell.
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
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * from: Utah Hdr: autoconf.c 1.31 91/01/21
 *
 *	@(#)autoconf.c	8.1 (Berkeley) 6/10/93
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: autoconf.c,v 1.17 2006/01/13 22:18:59 uwe Exp $");

#include "opt_md.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/disklabel.h>
#include <sys/device.h>

#include <sh3/exception.h>
#include <machine/bus.h>
#include <machine/intr.h>

#include <machine/config_hook.h>
#include <machine/autoconf.h>

#include <hpcsh/dev/hd64461/hd64461var.h>
#include <hpcsh/dev/hd64465/hd64465var.h>

static char booted_device_name[16];
#ifndef MEMORY_DISK_IS_ROOT
static void get_device(char *name);
#endif

void
cpu_configure()
{

	config_hook_init();
	softintr_init();
	hd6446x_intr_init();
#ifdef SH3
	if (CPU_IS_SH3)	/* HD64461 (Jornada 690, HP620LX, HPW-50PA) */
		intc_intr_establish(SH7709_INTEVT2_IRQ4, IST_LEVEL, IPL_TTY,
		    (void *)1/* fake. see intc_intr(). */, 0);
#endif
#ifdef SH4
	if (CPU_IS_SH4)	/* HD64465 (HPW-650PA) */
		intc_intr_establish(SH_INTEVT_IRL11, IST_LEVEL, IPL_TTY,
		    (void *)1/* fake. see intc_intr(). */, 0);
#endif

	/* Kick off autoconfiguration. */
	splhigh();
	if (config_rootfound("mainbus", NULL) == NULL)
		panic("no mainbus found");

	/* Configuration is finished, turn on interrupts. */
	spl0();
}

void
cpu_rootconf()
{

#ifndef MEMORY_DISK_IS_ROOT
	get_device(booted_device_name);

	printf("boot device: %s\n",
	       booted_device ? booted_device->dv_xname : "<unknown>");
#endif
	setroot(booted_device, booted_partition);
}

void
makebootdev(const char *cp)
{

	strncpy(booted_device_name, cp, 16);
}

#ifndef MEMORY_DISK_IS_ROOT
static void
get_device(char *name)
{
	int unit, part;
	char devname[16], buf[32], *cp;
	struct device *dv;

	if (strncmp(name, "/dev/", 5) == 0)
		name += 5;

	if (devsw_name2blk(name, devname, sizeof(devname)) == -1)
		return;

	name += strlen(devname);
	unit = part = 0;

	cp = name;
	while (*cp >= '0' && *cp <= '9')
		unit = (unit * 10) + (*cp++ - '0');
	if (cp == name)
		return;

	if (*cp >= 'a' && *cp <= ('a' + MAXPARTITIONS))
		part = *cp - 'a';
	else if (*cp != '\0' && *cp != ' ')
		return;
	sprintf(buf, "%s%d", devname, unit);
	for (dv = alldevs.tqh_first; dv != NULL; dv = dv->dv_list.tqe_next) {
		if (strcmp(buf, dv->dv_xname) == 0) {
			booted_device = dv;
			booted_partition = part;
			return;
		}
	}
}
#endif /* MEMORY_DISK_IS_ROOT */
