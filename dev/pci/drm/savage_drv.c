/* savage_drv.c -- Savage DRI driver
 */
/*-
 * Copyright 2005 Eric Anholt
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * ERIC ANHOLT BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Eric Anholt <anholt@FreeBSD.org>
 */

#include "drmP.h"
#include "drm.h"
#include "savage_drm.h"
#include "savage_drv.h"
#include "drm_pciids.h"

/* drv_PCI_IDs comes from drm_pciids.h, generated from drm_pciids.txt. */
static drm_pci_id_list_t savage_pciidlist[] = {
	savage_PCI_IDS
};

struct drm_ioctl_desc savage_ioctls[] = {
	DRM_IOCTL_DEF(DRM_SAVAGE_BCI_INIT, savage_bci_init, DRM_AUTH|DRM_MASTER|DRM_ROOT_ONLY),
	DRM_IOCTL_DEF(DRM_SAVAGE_BCI_CMDBUF, savage_bci_cmdbuf, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_SAVAGE_BCI_EVENT_EMIT, savage_bci_event_emit, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_SAVAGE_BCI_EVENT_WAIT, savage_bci_event_wait, DRM_AUTH),
};

int savage_max_ioctl = DRM_ARRAY_SIZE(savage_ioctls);

static const struct drm_driver_info savage_driver = {
	.buf_priv_size		= sizeof(drm_savage_buf_priv_t),
	.load			= savage_driver_load,
	.firstopen		= savage_driver_firstopen,
	.lastclose		= savage_driver_lastclose,
	.unload			= savage_driver_unload,
	.reclaim_buffers_locked = savage_reclaim_buffers,
	.dma_ioctl		= savage_bci_buffers,

	.ioctls			= savage_ioctls,
	.max_ioctl		= DRM_ARRAY_SIZE(savage_ioctls),

	.name			= DRIVER_NAME,
	.desc			= DRIVER_DESC,
	.date			= DRIVER_DATE,
	.major			= DRIVER_MAJOR,
	.minor			= DRIVER_MINOR,
	.patchlevel		= DRIVER_PATCHLEVEL,

	.use_agp		= 1,
	.use_mtrr		= 1,
	.use_pci_dma		= 1,
	.use_dma		= 1,
};

int	savagedrm_probe(struct device *, void *, void *);
void	savagedrm_attach(struct device *, struct device *, void *);

int
savagedrm_probe(struct device *parent, void *match, void *aux)
{
	return drm_probe((struct pci_attach_args *)aux, savage_pciidlist);
}

void
savagedrm_attach(struct device *parent, struct device *self, void *aux)
{
	struct pci_attach_args *pa = aux;
	struct drm_device *dev = (struct drm_device *)self;

	dev->driver = &savage_driver;
	return drm_attach(parent, self, pa, savage_pciidlist);
}

struct cfattach savagedrm_ca = {
	sizeof(struct drm_device), savagedrm_probe, savagedrm_attach,
	drm_detach, drm_activate
};

struct cfdriver savagedrm_cd = {
	0, "savagedrm", DV_DULL
};
