/*	$OpenBSD: abtn.c,v 1.9 2003/10/16 03:54:48 deraadt Exp $	*/
/*	$NetBSD: abtn.c,v 1.1 1999/07/12 17:48:26 tsubai Exp $	*/

/*-
 * Copyright (c) 2002, Miodrag Vallat.
 * Copyright (C) 1999 Tsubai Masanari.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/param.h>
#include <sys/device.h>
#include <sys/systm.h>

#include <machine/bus.h>

#include <dev/ofw/openfirm.h>
#include <macppc/macppc/ofw_machdep.h>

#include <macppc/dev/adbvar.h>

#define ABTN_HANDLER_ID 31

struct abtn_softc {
	struct device sc_dev;

	int origaddr;		/* ADB device type */
	int adbaddr;		/* current ADB address */
	int handler_id;
};

int abtn_match(struct device *, void *, void *);
void abtn_attach(struct device *, struct device *, void *);
void abtn_adbcomplete(caddr_t, caddr_t, int);

struct cfattach abtn_ca = {
	sizeof(struct abtn_softc), abtn_match, abtn_attach
};
struct cfdriver abtn_cd = {
	NULL, "abtn", DV_DULL
};

int
abtn_match(struct device *parent, void *cf, void *aux)
{
	struct adb_attach_args *aa = aux;

	if (aa->origaddr == ADBADDR_MISC &&
	    aa->handler_id == ABTN_HANDLER_ID)
		return 1;

	return 0;
}

void
abtn_attach(struct device *parent, struct device *self, void *aux)
{
	struct abtn_softc *sc = (struct abtn_softc *)self;
	struct adb_attach_args *aa = aux;
	ADBSetInfoBlock adbinfo;

	printf("brightness/volume/eject buttons\n");

	sc->origaddr = aa->origaddr;
	sc->adbaddr = aa->adbaddr;
	sc->handler_id = aa->handler_id;

	adbinfo.siServiceRtPtr = (Ptr)abtn_adbcomplete;
	adbinfo.siDataAreaAddr = (caddr_t)sc;

	SetADBInfo(&adbinfo, sc->adbaddr);
}

void
abtn_adbcomplete(caddr_t buffer, caddr_t data, int adb_command)
{
	u_int cmd, brightness;

	cmd = buffer[1];

	switch (cmd) {
	case 0x0a:	/* decrease brightness */
		brightness = cons_brightness;
		if (brightness == MAX_BRIGHTNESS)
			brightness++;		/* get round values */
		brightness -= STEP_BRIGHTNESS;
		of_setbrightness(brightness);
		break;

	case 0x09:	/* increase brightness */
		brightness = cons_brightness + STEP_BRIGHTNESS;
		of_setbrightness(brightness);
		break;

#ifdef DEBUG
	case 0x08:	/* mute */
	case 0x01:	/* mute, AV hardware */
	case 0x07:	/* decrease volume */
	case 0x02:	/* decrease volume, AV hardware */
	case 0x06:	/* increase volume */
	case 0x03:	/* increase volume, AV hardware */
		/* Need callback to do something with these */
		break;

	case 0x0c:	/* mirror display key */
		/* Need callback to do something with this */
		break;

	case 0x0b:	/* eject tray */
		/* Need callback to do something with this */
		break;

	case 0x7f:	/* numlock */
		/* Need callback to do something with this */
		break;

	default:
		if ((cmd & ~0x7f) == 0)
			printf("unknown ADB button 0x%x\n", cmd);
		break;
#endif
	}
}
