/*
 * Copyright (c) 2012 Stefan Fritsch.
 * Copyright (c) 2010 Minoura Makoto.
 * Copyright (c) 1998, 2001 Manuel Bouyer.
 * All rights reserved.
 *
 * This code is based in part on the NetBSD ld_virtio driver and the
 * OpenBSD vdsk driver.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *	notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *	notice, this list of conditions and the following disclaimer in the
 *	documentation and/or other materials provided with the distribution.
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

/*
 * Copyright (c) 2009, 2011 Mark Kettenis
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

#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <machine/bus.h>

#include <sys/device.h>
#include <sys/stat.h>
#include <sys/buf.h>
#include <sys/timeout.h>
#include <dev/pci/pcivar.h>
#include <dev/pci/virtioreg.h>
#include <dev/pci/virtiovar.h>
#include <dev/pci/vioblkreg.h>

#define VIOBLK_DEBUG 0

#include <scsi/scsi_all.h>
#include <scsi/scsi_disk.h>
#include <scsi/scsiconf.h>

#define VIOBLK_DONE	-1

struct virtio_feature_name vioblk_feature_names[] = {
	{ VIRTIO_BLK_F_BARRIER,		"Barrier" },
	{ VIRTIO_BLK_F_SIZE_MAX,	"SizeMax" },
	{ VIRTIO_BLK_F_SEG_MAX,		"SegMax" },
	{ VIRTIO_BLK_F_GEOMETRY,	"Geometry" },
	{ VIRTIO_BLK_F_RO,		"RO" },
	{ VIRTIO_BLK_F_BLK_SIZE,	"BlkSize" },
	{ VIRTIO_BLK_F_SCSI,		"SCSI" },
	{ VIRTIO_BLK_F_FLUSH,		"Flush" },
	{ VIRTIO_BLK_F_TOPOLOGY,	"Topology" },
	{ 0,				NULL }
};

struct virtio_blk_req {
	struct virtio_blk_req_hdr	 vr_hdr;
	uint8_t				 vr_status;
	struct scsi_xfer		*vr_xs;
	int				 vr_len;
	bus_dmamap_t			 vr_cmdsts;
	bus_dmamap_t			 vr_payload;
};

struct vioblk_softc {
	struct device		 sc_dev;
	struct virtio_softc	*sc_virtio;

	struct virtqueue         sc_vq[1];
	struct virtio_blk_req   *sc_reqs;
	bus_dma_segment_t        sc_reqs_segs[1];

	struct scsi_adapter	 sc_switch;
	struct scsi_link	 sc_link;

	int			 sc_readonly;
	int			 sc_notify_on_empty;

	uint32_t		 sc_queued;

	struct timeout		 sc_timeout;

	/* device configuration */
	uint64_t		 sc_capacity;
	uint32_t		 sc_size_max;
	uint32_t		 sc_seg_max;

	uint32_t		 sc_blk_size;
};

int	vioblk_match(struct device *, void *, void *);
void	vioblk_attach(struct device *, struct device *, void *);
int	vioblk_alloc_reqs(struct vioblk_softc *, int);
int	vioblk_vq_done(struct virtqueue *);
void	vioblk_vq_done1(struct vioblk_softc *, struct virtio_softc *,
			struct virtqueue *, int);
void	vioblk_timeout(void *);

void	vioblk_scsi_cmd(struct scsi_xfer *);
int	vioblk_dev_probe(struct scsi_link *);
void	vioblk_dev_free(struct scsi_link *);

void	vioblk_scsi_inq(struct scsi_xfer *);
void	vioblk_scsi_inquiry(struct scsi_xfer *);
void	vioblk_scsi_capacity(struct scsi_xfer *);
void	vioblk_scsi_capacity16(struct scsi_xfer *);
void	vioblk_scsi_done(struct scsi_xfer *, int);

struct cfattach vioblk_ca = {
	sizeof(struct vioblk_softc),
	vioblk_match,
	vioblk_attach,
	NULL
};

struct cfdriver vioblk_cd = {
	NULL, "vioblk", DV_DULL
};


int vioblk_match(struct device *parent, void *match, void *aux)
{
	struct virtio_softc *va = aux;
	if (va->sc_childdevid == PCI_PRODUCT_VIRTIO_BLOCK)
		return 1;
	return 0;
}

#if VIOBLK_DEBUG > 0

#define DBGPRINT(fmt, args...) printf("%s: " fmt "\n", __func__, ## args)

void vioblk_dumpdesc(struct vioblk_softc *, struct virtqueue *, int, int,
		     const char *);
void vioblk_dumpreq(struct vioblk_softc *, struct virtqueue *, int,
		    const char *);

void
vioblk_dumpdesc(struct vioblk_softc *sc, struct virtqueue *vq, int start, int len, const char *prefix)
{
	struct vring_desc *desc;
	int idx = start & vq->vq_mask;
	int i;
	len = (len & vq->vq_mask) + 4; // XXX ????
	for (i = 0; i < (len & vq->vq_mask); i++) {
		desc = &vq->vq_desc[idx];
		printf("%s desc %hu: flags: %s%s%s len: %u addr: 0x%08llx\n",
			prefix, idx,
			desc->flags & VRING_DESC_F_WRITE ? "W" : "R",
			desc->flags & VRING_DESC_F_NEXT ? "N" : " ",
			desc->flags & VRING_DESC_F_INDIRECT ? "I" : " ",
			desc->len, desc->addr);
#if 0
		if (desc->flags & VRING_DESC_F_INDIRECT) {
			int j;
			struct vring_desc *idesc;
			for (j = 0; j < desc->len / sizeof(*desc); j++) {
				uint32_t c = (uint32_t)desc->addr; // XXX: broken: this is the phys address
				idesc = (struct vring_desc *)c + j;
				printf("%s indirect desc %hu: flags: %s%s%s len: %u addr: 0x%08llx\n",
					j,
					desc->flags & VRING_DESC_F_WRITE ? "W" : "R",
					desc->flags & VRING_DESC_F_NEXT ? "N" : " ",
					desc->flags & VRING_DESC_F_INDIRECT ? "I" : " ",
					idesc->len, desc->addr);
			}
		}
#endif
		if (!(desc->flags & VRING_DESC_F_NEXT))
			break;
		idx = desc->next & vq->vq_mask;
	}
}

#define REQ_CASE(res, t)  case t: res = #t; break;

void
vioblk_dumpreq(struct vioblk_softc *sc, struct virtqueue *vq, int idx, const char *prefix)
{
	struct virtio_blk_req *vr = sc->sc_reqs + (idx & vq->vq_mask);
	const char *type;
	switch (vr->vr_hdr.type) {
		REQ_CASE(type, VIRTIO_BLK_T_IN)
		REQ_CASE(type, VIRTIO_BLK_T_OUT)
		REQ_CASE(type, VIRTIO_BLK_T_SCSI_CMD)
		REQ_CASE(type, VIRTIO_BLK_T_SCSI_CMD_OUT)
		REQ_CASE(type, VIRTIO_BLK_T_FLUSH)
		REQ_CASE(type, VIRTIO_BLK_T_FLUSH_OUT)
	}
	printf("%s virtio blk req %d: type: %s start_sector: %llu "
	       "status: %hu header: %p\n",
	       prefix, idx, type, vr->vr_hdr.sector, vr->vr_status,
	       &vr->vr_hdr);
}
#else

#define vioblk_dumpreq(...)		do {} while (0)
#define vioblk_dumpdesc(...)		do {} while (0)
#define DBGPRINT(fmt, args...)		do {} while (0)

#endif

void
vioblk_timeout(void *v)
{
	struct vioblk_softc *sc = v;
	struct virtqueue *vq = &sc->sc_vq[0];
	printf("virtio timeout %s: sc_queued %u vq_num %u\n",
	       sc->sc_dev.dv_xname, sc->sc_queued, vq->vq_num);
	DBGPRINT("vq_avail_idx: %hu vq_avail->idx: %hu vq_avail->flags: %hu",
		 vq->vq_avail_idx, vq->vq_avail->idx, vq->vq_avail->flags);
	DBGPRINT("vq_used_idx:  %hu vq_used->idx:  %hu vq_used->flags:  %hu",
		 vq->vq_used_idx,  vq->vq_used->idx,  vq->vq_used->flags);
	if (vq->vq_used_idx != vq->vq_used->idx) {
		int idx, i = vq->vq_used_idx & vq->vq_mask;
		struct vring_used_elem *e = &vq->vq_used->ring[i];
		DBGPRINT("used elem %hu: id %u len %u", i, e->id,
			 e->len & vq->vq_mask);
		idx = e->id & vq->vq_mask;
		vioblk_dumpreq(sc, vq, idx, __func__);
		vioblk_dumpdesc(sc, vq, idx, e->len, __func__);
	}
	// XXX can we recover somehow???
}


void
vioblk_attach(struct device *parent, struct device *self, void *aux)
{
	struct vioblk_softc *sc = (struct vioblk_softc *)self;
	struct virtio_softc *vsc = (struct virtio_softc *)parent;
	struct scsibus_attach_args saa;
	uint32_t features;
	int qsize;

	vsc->sc_vqs = &sc->sc_vq[0];
	vsc->sc_nvqs = 1;
	vsc->sc_config_change = 0;
	if (vsc->sc_child)
		panic("already attached to something else");
	vsc->sc_child = self;
	vsc->sc_ipl = IPL_BIO;
	vsc->sc_intrhand = virtio_vq_intr;
	sc->sc_virtio = vsc;

        features = virtio_negotiate_features(vsc, ( VIRTIO_BLK_F_RO |
						VIRTIO_F_NOTIFY_ON_EMPTY|
						VIRTIO_BLK_F_SIZE_MAX|
						VIRTIO_BLK_F_SEG_MAX|
						VIRTIO_BLK_F_BLK_SIZE|
						VIRTIO_BLK_F_FLUSH),
						vioblk_feature_names);

	if (features & VIRTIO_BLK_F_RO)
		sc->sc_readonly = 1;
	else
		sc->sc_readonly = 0;

	if (features & VIRTIO_BLK_F_SIZE_MAX)
		sc->sc_size_max = virtio_read_device_config_2(vsc,
		    VIRTIO_BLK_CONFIG_SIZE_MAX);
	else
		sc->sc_size_max = MAXPHYS;

	if (features & VIRTIO_BLK_F_SEG_MAX)
		sc->sc_seg_max = virtio_read_device_config_2(vsc,
		    VIRTIO_BLK_CONFIG_SEG_MAX);
	else
		sc->sc_seg_max = 1;
	sc->sc_seg_max = MIN(sc->sc_seg_max, MAXPHYS/NBPG + 2);
	sc->sc_size_max = MIN(sc->sc_size_max, sc->sc_seg_max * NBPG);

	sc->sc_capacity = virtio_read_device_config_8(vsc,
						VIRTIO_BLK_CONFIG_CAPACITY);

#if 0
	if (features & VIRTIO_BLK_F_BLK_SIZE) {
		ld->sc_blk_size = virtio_read_device_config_4(vsc,
					VIRTIO_BLK_CONFIG_BLK_SIZE);
	}
#endif


	if (virtio_alloc_vq(vsc, &sc->sc_vq[0], 0, sc->sc_size_max,
			    sc->sc_seg_max, "I/O request") != 0) {
		printf("\nCan't alloc virtqueue\n");
		goto err;
	}
	qsize = sc->sc_vq[0].vq_num;
	sc->sc_vq[0].vq_done = vioblk_vq_done;
	if (vioblk_alloc_reqs(sc, qsize) < 0) {
		printf("\nCan't alloc reqs\n");
		goto err;
	}

	if (features & VIRTIO_F_NOTIFY_ON_EMPTY) {
		virtio_stop_vq_intr(vsc, &sc->sc_vq[0]);
		sc->sc_notify_on_empty = 1;
	}
	else {
		sc->sc_notify_on_empty = 0;
	}

	timeout_set(&sc->sc_timeout, vioblk_timeout, sc);
	sc->sc_queued = 0;

	sc->sc_switch.scsi_cmd = vioblk_scsi_cmd;
	sc->sc_switch.scsi_minphys = scsi_minphys;
	sc->sc_switch.dev_probe = vioblk_dev_probe;
	sc->sc_switch.dev_free = vioblk_dev_free;

	sc->sc_link.adapter = &sc->sc_switch;
	sc->sc_link.adapter_softc = self;
	sc->sc_link.adapter_buswidth = 2;
	sc->sc_link.luns = 1;
	sc->sc_link.adapter_target = 2;
	sc->sc_link.openings = qsize;

	bzero(&saa, sizeof(saa));
	saa.saa_sc_link = &sc->sc_link;
	printf("\n");
	config_found(self, &saa, scsiprint);

	return;
err:
	vsc->sc_child = (void*)1;
	return;
}

int
vioblk_vq_done(struct virtqueue *vq)
{
	struct virtio_softc *vsc = vq->vq_owner;
	struct vioblk_softc *sc = (struct vioblk_softc *)vsc->sc_child;
	int slot;
	int ret = 0;

	if (!sc->sc_notify_on_empty)
		virtio_stop_vq_intr(vsc, vq);
	for (;;) {
		if (virtio_dequeue(vsc, vq, &slot, NULL) != 0) {
			if (sc->sc_notify_on_empty)
				break;
			virtio_start_vq_intr(vsc, vq);
			if (virtio_dequeue(vsc, vq, &slot, NULL) != 0)
				break;
		}
		vioblk_vq_done1(sc, vsc, vq, slot);
		ret = 1;
	}
	return ret;
}

void
vioblk_vq_done1(struct vioblk_softc *sc, struct virtio_softc *vsc,
    struct virtqueue *vq, int slot)
{
	struct virtio_blk_req *vr = &sc->sc_reqs[slot];
	struct scsi_xfer *xs = vr->vr_xs;

	membar_consumer();
	bus_dmamap_sync(vsc->sc_dmat, vr->vr_cmdsts,
			0, sizeof(struct virtio_blk_req_hdr),
			BUS_DMASYNC_POSTWRITE);
	if (vr->vr_hdr.type != VIRTIO_BLK_T_FLUSH) {
		bus_dmamap_sync(vsc->sc_dmat, vr->vr_payload, 0, vr->vr_len,
		    (vr->vr_hdr.type == VIRTIO_BLK_T_IN) ? BUS_DMASYNC_POSTREAD
							 : BUS_DMASYNC_POSTWRITE);
	}
	bus_dmamap_sync(vsc->sc_dmat, vr->vr_cmdsts,
			sizeof(struct virtio_blk_req_hdr), sizeof(uint8_t),
			BUS_DMASYNC_POSTREAD);


	if (vr->vr_status != VIRTIO_BLK_S_OK) {
		DBGPRINT("EIO");
		xs->error = XS_DRIVER_STUFFUP;
		xs->resid = xs->datalen;
	} else {
		xs->error = XS_NOERROR;
		xs->resid = xs->datalen - vr->vr_len;
	}
	scsi_done(xs);
	vr->vr_len = VIOBLK_DONE;

	virtio_dequeue_commit(vsc, vq, slot);

	if (--sc->sc_queued == 0)
		timeout_del(&sc->sc_timeout);
}

void
vioblk_scsi_cmd(struct scsi_xfer *xs)
{
	struct scsi_rw *rw;
	struct scsi_rw_big *rwb;
	u_int64_t lba = 0;
	u_int32_t sector_count;
	uint8_t operation;
	int isread;

	switch (xs->cmd->opcode) {
	case READ_BIG:
	case READ_COMMAND:
		operation = VIRTIO_BLK_T_IN;
		isread = 1;
		break;
	case WRITE_BIG:
	case WRITE_COMMAND:
		operation = VIRTIO_BLK_T_OUT;
		isread = 0;
		break;

	case SYNCHRONIZE_CACHE:
		operation = VIRTIO_BLK_T_FLUSH;
		break;

	case INQUIRY:
		vioblk_scsi_inq(xs);
		return;
	case READ_CAPACITY:
		vioblk_scsi_capacity(xs);
		return;
	case READ_CAPACITY_16:
		vioblk_scsi_capacity16(xs);
		return;

	case TEST_UNIT_READY:
	case START_STOP:
	case PREVENT_ALLOW:
		vioblk_scsi_done(xs, XS_NOERROR);
		return;

	default:
		printf("%s cmd 0x%02x\n", __func__, xs->cmd->opcode);
	case MODE_SENSE:
	case MODE_SENSE_BIG:
	case REPORT_LUNS:
		vioblk_scsi_done(xs, XS_DRIVER_STUFFUP);
		return;
	}

	if (xs->cmdlen == 6) {
		rw = (struct scsi_rw *)xs->cmd;
		lba = _3btol(rw->addr) & (SRW_TOPADDR << 16 | 0xffff);
		sector_count = rw->length ? rw->length : 0x100;
	} else {
		rwb = (struct scsi_rw_big *)xs->cmd;
		lba = _4btol(rwb->addr);
		sector_count = _2btol(rwb->length);
	}

{
	struct vioblk_softc *sc = xs->sc_link->adapter_softc;
	struct virtqueue *vq = &sc->sc_vq[0];
	struct virtio_blk_req *vr;
	struct virtio_softc *vsc = sc->sc_virtio;
	int len, s;
	int timeout;
	int slot, ret, nsegs;

	s = splbio();
	ret = virtio_enqueue_prep(vsc, vq, &slot);
	if (ret) {
		DBGPRINT("virtio_enqueue_prep: %d, vq_num: %d, sc_queued: %d",
		    ret, vq->vq_num, sc->sc_queued);
		vioblk_scsi_done(xs, XS_NO_CCB);
		splx(s);
		return;
	}
	vr = &sc->sc_reqs[slot];
	if (operation != VIRTIO_BLK_T_FLUSH) {
		len = MIN(xs->datalen, sector_count * VIRTIO_BLK_SECTOR_SIZE);
		// XXX: xs->data can be uio???
		ret = bus_dmamap_load(vsc->sc_dmat, vr->vr_payload,
		    xs->data, len, NULL,
		    ((isread?BUS_DMA_READ:BUS_DMA_WRITE) | BUS_DMA_NOWAIT));
		if (ret) {
			DBGPRINT("bus_dmamap_load: %d", ret);
			vioblk_scsi_done(xs, XS_NO_CCB);
			return;
		}
		nsegs = vr->vr_payload->dm_nsegs + 2;
	} else {
		len = 0;
		nsegs = 2;
	}
	ret = virtio_enqueue_reserve(vsc, vq, slot, nsegs);
	if (ret) {
		DBGPRINT("virtio_enqueue_reserve: %d", ret);
		bus_dmamap_unload(vsc->sc_dmat, vr->vr_payload);
		vioblk_scsi_done(xs, XS_NO_CCB);
		return;
	}
	vr->vr_xs = xs;
	vr->vr_hdr.type = operation;
	vr->vr_hdr.ioprio = 0;
	vr->vr_hdr.sector = lba;
	vr->vr_len = len;

	bus_dmamap_sync(vsc->sc_dmat, vr->vr_cmdsts,
			0, sizeof(struct virtio_blk_req_hdr),
			BUS_DMASYNC_PREWRITE);
	if (operation != VIRTIO_BLK_T_FLUSH) {
		bus_dmamap_sync(vsc->sc_dmat, vr->vr_payload,
				0, len,
				isread?BUS_DMASYNC_PREREAD:BUS_DMASYNC_PREWRITE);
	}
	bus_dmamap_sync(vsc->sc_dmat, vr->vr_cmdsts,
			offsetof(struct virtio_blk_req, vr_status),
			sizeof(uint8_t),
			BUS_DMASYNC_PREREAD);

	virtio_enqueue_p(vsc, vq, slot, vr->vr_cmdsts,
			0, sizeof(struct virtio_blk_req_hdr),
			1);
	if (operation != VIRTIO_BLK_T_FLUSH)
		virtio_enqueue(vsc, vq, slot, vr->vr_payload, !isread);
	virtio_enqueue_p(vsc, vq, slot, vr->vr_cmdsts,
			offsetof(struct virtio_blk_req, vr_status),
			sizeof(uint8_t),
			0);
	//vioblk_dumpdesc(sc, vq, slot, nsegs, __func__);
	//vioblk_dumpreq(sc, vq, slot, __func__);
	virtio_enqueue_commit(vsc, vq, slot, 1);
	sc->sc_queued++;
	timeout_add_sec(&sc->sc_timeout, 1);

	if (!ISSET(xs->flags, SCSI_POLL)) {
		splx(s);
		return;
	}

	timeout = 1000;
	do {
		if (virtio_intr(sc->sc_virtio) && vr->vr_len == VIOBLK_DONE)
			break;

		delay(1000);
	} while(--timeout > 0);
	splx(s);
}
}

void
vioblk_scsi_inq(struct scsi_xfer *xs)
{
	struct scsi_inquiry *inq = (struct scsi_inquiry *)xs->cmd;

	if (ISSET(inq->flags, SI_EVPD))
		vioblk_scsi_done(xs, XS_DRIVER_STUFFUP);
	else
		vioblk_scsi_inquiry(xs);
}

void
vioblk_scsi_inquiry(struct scsi_xfer *xs)
{
	struct scsi_inquiry_data inq;

	bzero(&inq, sizeof(inq));

	inq.device = T_DIRECT;
	inq.version = 0x05; /* SPC-3 */
	inq.response_format = 2;
	inq.additional_length = 32;
	inq.flags |= SID_CmdQue;
	bcopy("VirtIO  ", inq.vendor, sizeof(inq.vendor));
	bcopy("Block Device    ", inq.product, sizeof(inq.product));

	bcopy(&inq, xs->data, MIN(sizeof(inq), xs->datalen));

	vioblk_scsi_done(xs, XS_NOERROR);
}

void
vioblk_scsi_capacity(struct scsi_xfer *xs)
{
	struct vioblk_softc *sc = xs->sc_link->adapter_softc;
	struct scsi_read_cap_data rcd;
	uint64_t capacity;

	bzero(&rcd, sizeof(rcd));

	capacity = sc->sc_capacity - 1;
	if (capacity > 0xffffffff)
		capacity = 0xffffffff;

	_lto4b(capacity, rcd.addr);
	_lto4b(VIRTIO_BLK_SECTOR_SIZE, rcd.length);

	bcopy(&rcd, xs->data, MIN(sizeof(rcd), xs->datalen));

	vioblk_scsi_done(xs, XS_NOERROR);
}

void
vioblk_scsi_capacity16(struct scsi_xfer *xs)
{
	struct vioblk_softc *sc = xs->sc_link->adapter_softc;
	struct scsi_read_cap_data_16 rcd;

	bzero(&rcd, sizeof(rcd));

	_lto8b(sc->sc_capacity - 1, rcd.addr);
	_lto4b(VIRTIO_BLK_SECTOR_SIZE, rcd.length);

	bcopy(&rcd, xs->data, MIN(sizeof(rcd), xs->datalen));

	vioblk_scsi_done(xs, XS_NOERROR);
}

void
vioblk_scsi_done(struct scsi_xfer *xs, int error)
{
	xs->error = error;

	scsi_done(xs);
}

int
vioblk_dev_probe(struct scsi_link *link)
{
	KASSERT(link->lun == 0);

	if (link->target == 0)
		return (0);

	return (ENODEV);
}

void
vioblk_dev_free(struct scsi_link *link)
{
	printf("%s\n", __func__);
}

int
vioblk_alloc_reqs(struct vioblk_softc *sc, int qsize)
{
	int allocsize, r, rsegs, i;
	void *vaddr;

	allocsize = sizeof(struct virtio_blk_req) * qsize;
	r = bus_dmamem_alloc(sc->sc_virtio->sc_dmat, allocsize, 0, 0,
			     &sc->sc_reqs_segs[0], 1, &rsegs, BUS_DMA_NOWAIT);
	if (r != 0) {
		printf("DMA memory allocation failed, size %d, error %d\n",
		       allocsize, r);
		goto err_none;
	}
	r = bus_dmamem_map(sc->sc_virtio->sc_dmat,
			   &sc->sc_reqs_segs[0], 1, allocsize,
			   (caddr_t *)&vaddr, BUS_DMA_NOWAIT);
	if (r != 0) {
		printf("DMA memory map failed, error %d\n", r);
		goto err_dmamem_alloc;
	}
	sc->sc_reqs = vaddr;
	memset(vaddr, 0, allocsize);
	for (i = 0; i < qsize; i++) {
		struct virtio_blk_req *vr = &sc->sc_reqs[i];
		r = bus_dmamap_create(sc->sc_virtio->sc_dmat,
				      offsetof(struct virtio_blk_req, vr_xs),
				      1,
				      offsetof(struct virtio_blk_req, vr_xs),
				      0,
				      BUS_DMA_NOWAIT|BUS_DMA_ALLOCNOW,
				      &vr->vr_cmdsts);
		if (r != 0) {
			printf("command dmamap creation failed, error %d\n", r);
			goto err_reqs;
		}
		r = bus_dmamap_load(sc->sc_virtio->sc_dmat, vr->vr_cmdsts,
				    &vr->vr_hdr,
				    offsetof(struct virtio_blk_req, vr_xs),
				    NULL, BUS_DMA_NOWAIT);
		if (r != 0) {
			printf("command dmamap load failed, error %d\n", r);
			goto err_reqs;
		}
		r = bus_dmamap_create(sc->sc_virtio->sc_dmat,
				      sc->sc_size_max,
				      sc->sc_seg_max,
				      MAXPHYS,
				      0,
				      BUS_DMA_NOWAIT|BUS_DMA_ALLOCNOW,
				      &vr->vr_payload);
		if (r != 0) {
			printf("payload dmamap creation failed, error %d\n", r);
			goto err_reqs;
		}
	}
	return 0;

err_reqs:
	for (i = 0; i < qsize; i++) {
		struct virtio_blk_req *vr = &sc->sc_reqs[i];
		if (vr->vr_cmdsts) {
			bus_dmamap_destroy(sc->sc_virtio->sc_dmat,
					   vr->vr_cmdsts);
			vr->vr_cmdsts = 0;
		}
		if (vr->vr_payload) {
			bus_dmamap_destroy(sc->sc_virtio->sc_dmat,
					   vr->vr_payload);
			vr->vr_payload = 0;
		}
	}
	bus_dmamem_unmap(sc->sc_virtio->sc_dmat, (caddr_t)sc->sc_reqs, allocsize);
err_dmamem_alloc:
	bus_dmamem_free(sc->sc_virtio->sc_dmat, &sc->sc_reqs_segs[0], 1);
err_none:
	return -1;
}
