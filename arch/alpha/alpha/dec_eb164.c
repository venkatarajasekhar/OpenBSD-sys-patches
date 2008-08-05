/* $OpenBSD: dec_eb164.c,v 1.16 2008/08/02 13:48:09 miod Exp $ */
/* $NetBSD: dec_eb164.c,v 1.33 2000/05/22 20:13:32 thorpej Exp $ */

/*
 * Copyright (c) 1995, 1996, 1997 Carnegie-Mellon University.
 * All rights reserved.
 *
 * Author: Chris G. Demetriou
 * 
 * Permission to use, copy, modify and distribute this software and
 * its documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" 
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND 
 * FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */
/*
 * Additional Copyright (c) 1997 by Matthew Jacob for NASA/Ames Research Center
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>
#include <sys/termios.h>
#include <dev/cons.h>
#include <sys/conf.h>

#include <machine/rpb.h>
#include <machine/autoconf.h>
#include <machine/cpuconf.h>
#include <machine/bus.h>

#include <dev/ic/comreg.h>
#include <dev/ic/comvar.h>

#include <dev/isa/isareg.h>
#include <dev/isa/isavar.h>
#include <dev/ic/i8042reg.h>
#include <dev/ic/pckbcvar.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/pcivar.h>

#include <alpha/pci/ciareg.h>
#include <alpha/pci/ciavar.h>

#include <scsi/scsi_all.h>
#include <scsi/scsiconf.h>
#include <dev/ata/atavar.h>

#include "pckbd.h"

#ifndef CONSPEED
#define CONSPEED TTYDEF_SPEED
#endif
static int comcnrate = CONSPEED;

#define	DR_VERBOSE(f) while (0)

void dec_eb164_init(void);
static void dec_eb164_cons_init(void);
static void dec_eb164_device_register(struct device *, void *);

void
dec_eb164_init()
{

	platform.family = "EB164";

	if ((platform.model = alpha_dsr_sysname()) == NULL) {
		/* XXX Don't know the system variations, yet. */
		platform.model = alpha_unknown_sysname();
	}

	platform.iobus = "cia";
	platform.cons_init = dec_eb164_cons_init;
	platform.device_register = dec_eb164_device_register;
}

static void
dec_eb164_cons_init()
{
	struct ctb *ctb;
	struct cia_config *ccp;
	extern struct cia_config cia_configuration;

	ccp = &cia_configuration;
	cia_init(ccp, 0);

	ctb = (struct ctb *)(((caddr_t)hwrpb) + hwrpb->rpb_ctb_off);

	switch (ctb->ctb_term_type) {
	case CTB_PRINTERPORT: 
		/* serial console ... */
		/* XXX */
		{
			/*
			 * Delay to allow PROM putchars to complete.
			 * FIFO depth * character time,
			 * character time = (1000000 / (defaultrate / 10))
			 */
			DELAY(160000000 / comcnrate);

			if(comcnattach(&ccp->cc_iot, 0x3f8, comcnrate,
			    COM_FREQ,
			    (TTYDEF_CFLAG & ~(CSIZE | PARENB)) | CS8))
				panic("can't init serial console");

			break;
		}

	case CTB_GRAPHICS:
#if NPCKBD > 0
		/* display console ... */
		/* XXX */
		(void) pckbc_cnattach(&ccp->cc_iot, IO_KBD, KBCMDP,
		    PCKBC_KBD_SLOT, 0);

		/*
		 * On at least LX164, SRM reports an isa video board
		 * as a pci slot with 0xff as the bus and slot numbers.
		 * Since these values are not plausible from a pci point
		 * of view, it is safe to check for them.
		 */
		if (CTB_TURBOSLOT_TYPE(ctb->ctb_turboslot) ==
		    CTB_TURBOSLOT_TYPE_ISA ||
		    (CTB_TURBOSLOT_BUS(ctb->ctb_turboslot) == 0xff &&
		     CTB_TURBOSLOT_SLOT(ctb->ctb_turboslot) == 0xff))
			isa_display_console(&ccp->cc_iot, &ccp->cc_memt);
		else
			pci_display_console(&ccp->cc_iot, &ccp->cc_memt,
			    &ccp->cc_pc, CTB_TURBOSLOT_BUS(ctb->ctb_turboslot),
			    CTB_TURBOSLOT_SLOT(ctb->ctb_turboslot), 0);
#else
		panic("not configured to use display && keyboard console");
#endif
		break;

	default:
		printf("ctb->ctb_term_type = 0x%lx\n", ctb->ctb_term_type);
		printf("ctb->ctb_turboslot = 0x%lx\n", ctb->ctb_turboslot);

		panic("consinit: unknown console type %ld",
		    ctb->ctb_term_type);
	}
}

static void
dec_eb164_device_register(dev, aux)
	struct device *dev;
	void *aux;
{
	static int found, initted, diskboot, netboot;
	static struct device *pcidev, *ctrlrdev;
	struct bootdev_data *b = bootdev_data;
	struct device *parent = dev->dv_parent;
	struct cfdata *cf = dev->dv_cfdata;
	struct cfdriver *cd = cf->cf_driver;

	if (found)
		return;

	if (!initted) {
		diskboot = (strncasecmp(b->protocol, "SCSI", 4) == 0) ||
		    (strncasecmp(b->protocol, "IDE", 3) == 0);
		netboot = (strncasecmp(b->protocol, "BOOTP", 5) == 0) ||
		    (strncasecmp(b->protocol, "MOP", 3) == 0);
		DR_VERBOSE(printf("diskboot = %d, netboot = %d\n", diskboot,
		    netboot));
		initted = 1;
	}

	if (pcidev == NULL) {
		if (strcmp(cd->cd_name, "pci"))
			return;
		else {
			struct pcibus_attach_args *pba = aux;

			if ((b->slot / 1000) != pba->pba_bus)
				return;
	
			pcidev = dev;
			DR_VERBOSE(printf("\npcidev = %s\n", dev->dv_xname));
			return;
		}
	}

	if (ctrlrdev == NULL) {
		if (parent != pcidev)
			return;
		else {
			struct pci_attach_args *pa = aux;
			int slot;

			slot = pa->pa_bus * 1000 + pa->pa_function * 100 +
			    pa->pa_device;
			if (b->slot != slot)
				return;
	
			if (netboot) {
				booted_device = dev;
				DR_VERBOSE(printf("\nbooted_device = %s\n",
				    dev->dv_xname));
				found = 1;
			} else {
				ctrlrdev = dev;
				DR_VERBOSE(printf("\nctrlrdev = %s\n",
				    dev->dv_xname));
			}
			return;
		}
	}

	if (!diskboot)
		return;

	if (!strcmp(cd->cd_name, "sd") || !strcmp(cd->cd_name, "st") ||
	    !strcmp(cd->cd_name, "cd")) {
		struct scsi_attach_args *sa = aux;
		struct scsi_link *periph = sa->sa_sc_link;
		int unit;

		if (parent->dv_parent != ctrlrdev)
			return;

		unit = periph->target * 100 + periph->lun;
		if (b->unit != unit)
			return;

		/* we've found it! */
		booted_device = dev;
		DR_VERBOSE(printf("\nbooted_device = %s\n", dev->dv_xname));
		found = 1;
	}

	/*
	 * Support to boot from IDE drives.
	 */
	if (!strcmp(cd->cd_name, "wd")) {
		struct ata_atapi_attach *aa_link = aux;
		int variation = hwrpb->rpb_variation & SV_ST_MASK;

		if ((strncmp("pciide", parent->dv_xname, 6) != 0))
			return;
		if (parent != ctrlrdev)
			return;

		DR_VERBOSE(printf("\nAtapi info: drive: %d, channel %d\n",
		    aa_link->aa_drv_data->drive, aa_link->aa_channel));
		DR_VERBOSE(printf("Bootdev info: unit: %d, channel: %d\n",
		    b->unit, b->channel));
		if (b->unit != aa_link->aa_drv_data->drive)
			return;

		/*
		 * On 164SX, the built-in IDE controller appears as
		 * two distinct pciide devices, both with a single
		 * channel.  However SRM will nevertheless pretend
		 * the second channel is channel #1 of the second
		 * device, while it is really channel #0, so just
		 * ignore the channel number in this case.
		 */
		if (variation >= SV_ST_ALPHAPC164SX_400 &&
		   variation <= SV_ST_ALPHAPC164SX_600 &&
		    b->slot == 0 * 1000 + 2 * 100 + 8) {
			/* nothing */
		} else {
			if (b->channel != aa_link->aa_channel)
				return;
		}

		/* we've found it! */
		booted_device = dev;
		DR_VERBOSE(printf("booted_device = %s\n", dev->dv_xname));
		found = 1;
	}
}
