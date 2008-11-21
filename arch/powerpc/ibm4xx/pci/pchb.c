/*	$NetBSD: pchb.c,v 1.5 2005/12/11 12:18:43 christos Exp $	*/

/*-
 * Copyright (c) 1996 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Jason R. Thorpe.
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
 *        This product includes software developed by the NetBSD
 *        Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: pchb.c,v 1.5 2005/12/11 12:18:43 christos Exp $");

#include "pci.h"
#include "opt_pci.h"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>
#include <sys/extent.h>
#include <sys/malloc.h>

#define _IBM4XX_BUS_DMA_PRIVATE

#include <powerpc/ibm4xx/ibm405gp.h>
#include <powerpc/ibm4xx/dev/plbvar.h>

#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/pcidevs.h>
#include <dev/pci/pciconf.h>

static int	pchbmatch(struct device *, struct cfdata *, void *);
static void	pchbattach(struct device *, struct device *, void *);
static int	pchbprint(void *, const char *);
static pci_chipset_tag_t	alloc_chipset_tag(int);

CFATTACH_DECL(pchb, sizeof(struct device),
    pchbmatch, pchbattach, NULL, NULL);

static int pcifound = 0;

/* IO window located @ e8000000 and maps to 0-0xffff */
static struct powerpc_bus_space pchb_io_tag = {
	_BUS_SPACE_LITTLE_ENDIAN | _BUS_SPACE_IO_TYPE,
	IBM405GP_PLB_PCI_IO_START,		/* offset */
	IBM405GP_PCI_PCI_IO_START,		/* extent base */
	IBM405GP_PCI_PCI_IO_START + 0xffff,	/* extent limit */
};

/* PCI memory window is directly mapped */
static struct powerpc_bus_space pchb_mem_tag = {
	_BUS_SPACE_LITTLE_ENDIAN | _BUS_SPACE_MEM_TYPE,
	0x00000000,					/* offset */
	IBM405GP_PCI_MEM_START,			/* extent base */
	IBM405GP_PCI_MEM_START + 0x1fffffff,	/* extent limit */
};


static int
pchbmatch(struct device *parent, struct cfdata *cf, void *aux)
{
	struct plb_attach_args *paa = aux;
	/* XXX chipset tag unused by walnut, so just pass 0 */
	pci_chipset_tag_t pc = alloc_chipset_tag(0);
	pcitag_t tag; 
	int class, id;

	/* match only pchb devices */
	if (strcmp(paa->plb_name, cf->cf_name) != 0)
		return 0;

	pci_machdep_init();
	tag = pci_make_tag(pc, 0, 0, 0);

	class = pci_conf_read(pc, tag, PCI_CLASS_REG);
	id = pci_conf_read(pc, tag, PCI_ID_REG);

	/*
	 * Match all known PCI host chipsets.
	 */
	if (PCI_CLASS(class) == PCI_CLASS_BRIDGE &&
	    PCI_SUBCLASS(class) == PCI_SUBCLASS_BRIDGE_HOST) {
		switch (PCI_VENDOR(id)) {
		case PCI_VENDOR_IBM:
			switch (PCI_PRODUCT(id)) {
			case PCI_PRODUCT_IBM_405GP:
				return (!pcifound);
			}
			break;
		}
	}
	return (0);
}

static void
pchbattach(struct device *parent, struct device *self, void *aux)
{
	struct plb_attach_args *paa = aux;
	struct pcibus_attach_args pba;
	char devinfo[256];
#ifdef PCI_CONFIGURE_VERBOSE
	extern int pci_conf_debug;

	pci_conf_debug = 1;
#endif
	pci_chipset_tag_t pc = alloc_chipset_tag(0);
	pcitag_t tag; 
	int class, id;

	pci_machdep_init();
	tag = pci_make_tag(pc, 0, 0, 0);

	class = pci_conf_read(pc, tag, PCI_CLASS_REG);
	id = pci_conf_read(pc, tag, PCI_ID_REG);

	printf("\n");
	pcifound++;
	/*
	 * All we do is print out a description.  Eventually, we
	 * might want to add code that does something that's
	 * possibly chipset-specific.
	 */

	pci_devinfo(id, class, 0, devinfo, sizeof(devinfo));
	printf("%s: %s (rev. 0x%02x)\n", self->dv_xname, devinfo,
	    PCI_REVISION(class));

	pci_machdep_init(); /* Redundant... */
	ibm4xx_setup_pci();
#ifdef PCI_CONFIGURE_VERBOSE
	ibm4xx_show_pci_map();
#endif

	if (bus_space_init(&pchb_io_tag, "pchbio", NULL, 0))
		panic("pchbattach: can't init IO tag");
	if (bus_space_init(&pchb_mem_tag, "pchbmem", NULL, 0))
		panic("pchbattach: can't init MEM tag");

#ifdef PCI_NETBSD_CONFIGURE
	pc->memext = extent_create("pcimem", IBM405GP_PCI_MEM_START,
	    IBM405GP_PCI_MEM_START + 0x1fffffff, M_DEVBUF, NULL, 0,
	    EX_NOWAIT);
	pc->ioext = extent_create("pciio", IBM405GP_PCI_PCI_IO_START,
	    IBM405GP_PCI_PCI_IO_START + 0xffff, M_DEVBUF, NULL, 0, EX_NOWAIT);
	pci_configure_bus(pc, pc->ioext, pc->memext, NULL, 0, 32);
#endif /* PCI_NETBSD_CONFIGURE */

#ifdef PCI_CONFIGURE_VERBOSE
	printf("running config_found PCI\n");
#endif
	/* IO window located @ e8000000 and maps to 0-0xffff */
	pba.pba_iot = &pchb_io_tag;
	/* PCI memory window is directly mapped */
	pba.pba_memt = &pchb_mem_tag;
	pba.pba_dmat = paa->plb_dmat;
	pba.pba_dmat64 = NULL;
	pba.pba_pc = pc;
	pba.pba_bus = 0;
	pba.pba_bridgetag = NULL;
	pba.pba_flags = PCI_FLAGS_MEM_ENABLED | PCI_FLAGS_IO_ENABLED;
	config_found_ia(self, "pcibus", &pba, pchbprint);
}


static int
pchbprint(void *aux, const char *p)
{

	if (p == NULL)
		return (UNCONF);
	return (QUIET);
}

#if 0
static void
scan_pci_bus(void)
{
	pcitag_t tag;
	int i, x;

	for (i=0;i<32;i++){
		tag = pci_make_tag(0, 0, i, 0);
		x = pci_conf_read(0, tag, 0);
		printf("%d tag=%08x : %08x\n", i, tag, x);
#if 0
		if (PCI_VENDOR(x) == PCI_VENDOR_INTEL
		    && PCI_PRODUCT(x) == PCI_PRODUCT_INTEL_80960_RP) {
			/* Do not configure PCI bus analyzer */
			continue;
		}
		x = pci_conf_read(0, tag, PCI_COMMAND_STATUS_REG);
		x |= PCI_COMMAND_IO_ENABLE | PCI_COMMAND_MEM_ENABLE | PCI_COMMAND_MASTER_ENABLE;
		pci_conf_write(0, tag, PCI_COMMAND_STATUS_REG, x);
#endif
	}
}
#endif

static pci_chipset_tag_t
alloc_chipset_tag(int node)
{
	pci_chipset_tag_t npc;

	npc = malloc(sizeof *npc, M_DEVBUF, M_NOWAIT);
	if (npc == NULL)
		panic("could not allocate pci_chipset_tag_t");
	npc->rootnode = node;

	return (npc);
}
