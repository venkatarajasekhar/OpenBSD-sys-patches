/*-
 * Copyright (c) 2001 Atsushi Onoe
 * Copyright (c) 2002-2009 Sam Leffler, Errno Consulting
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
__FBSDID("$FreeBSD: src/sys/net80211/ieee80211_input.c,v 1.123 2009/01/08 17:12:47 sam Exp $");

#include "opt_wlan.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>   
#include <sys/malloc.h>
#include <sys/endian.h>
#include <sys/kernel.h>
 
#include <sys/socket.h>
 
#include <net/ethernet.h>
#include <net/if.h>
#include <net/if_llc.h>
#include <net/if_media.h>
#include <net/if_vlan_var.h>

#include <net80211/ieee80211_var.h>
#include <net80211/ieee80211_input.h>

#include <net/bpf.h>

#ifdef INET
#include <netinet/in.h>
#include <net/ethernet.h>
#endif

int
ieee80211_input_all(struct ieee80211com *ic,
	struct mbuf *m, int rssi, int noise, u_int32_t rstamp)
{
	struct ieee80211vap *vap;
	int type = -1;

	/* XXX locking */
	TAILQ_FOREACH(vap, &ic->ic_vaps, iv_next) {
		struct ieee80211_node *ni;
		struct mbuf *mcopy;

		/*
		 * WDS vap's only receive directed traffic from the
		 * station at the ``far end''.  That traffic should
		 * be passed through the AP vap the station is associated
		 * to--so don't spam them with mcast frames.
		 */
		if (vap->iv_opmode == IEEE80211_M_WDS)
			continue;
		if (TAILQ_NEXT(vap, iv_next) != NULL) {
			/*
			 * Packet contents are changed by ieee80211_decap
			 * so do a deep copy of the packet.
			 */
			mcopy = m_dup(m, M_DONTWAIT);
			if (mcopy == NULL) {
				/* XXX stat+msg */
				continue;
			}
		} else {
			mcopy = m;
			m = NULL;
		}
		ni = ieee80211_ref_node(vap->iv_bss);
		type = ieee80211_input(ni, mcopy, rssi, noise, rstamp);
		ieee80211_free_node(ni);
	}
	if (m != NULL)			/* no vaps, reclaim mbuf */
		m_freem(m);
	return type;
}

/*
 * This function reassemble fragments.
 *
 * XXX should handle 3 concurrent reassemblies per-spec.
 */
struct mbuf *
ieee80211_defrag(struct ieee80211_node *ni, struct mbuf *m, int hdrspace)
{
	struct ieee80211vap *vap = ni->ni_vap;
	struct ieee80211_frame *wh = mtod(m, struct ieee80211_frame *);
	struct ieee80211_frame *lwh;
	uint16_t rxseq;
	uint8_t fragno;
	uint8_t more_frag = wh->i_fc[1] & IEEE80211_FC1_MORE_FRAG;
	struct mbuf *mfrag;

	KASSERT(!IEEE80211_IS_MULTICAST(wh->i_addr1), ("multicast fragm?"));

	rxseq = le16toh(*(uint16_t *)wh->i_seq);
	fragno = rxseq & IEEE80211_SEQ_FRAG_MASK;

	/* Quick way out, if there's nothing to defragment */
	if (!more_frag && fragno == 0 && ni->ni_rxfrag[0] == NULL)
		return m;

	/*
	 * Remove frag to insure it doesn't get reaped by timer.
	 */
	if (ni->ni_table == NULL) {
		/*
		 * Should never happen.  If the node is orphaned (not in
		 * the table) then input packets should not reach here.
		 * Otherwise, a concurrent request that yanks the table
		 * should be blocked by other interlocking and/or by first
		 * shutting the driver down.  Regardless, be defensive
		 * here and just bail
		 */
		/* XXX need msg+stat */
		m_freem(m);
		return NULL;
	}
	IEEE80211_NODE_LOCK(ni->ni_table);
	mfrag = ni->ni_rxfrag[0];
	ni->ni_rxfrag[0] = NULL;
	IEEE80211_NODE_UNLOCK(ni->ni_table);

	/*
	 * Validate new fragment is in order and
	 * related to the previous ones.
	 */
	if (mfrag != NULL) {
		uint16_t last_rxseq;

		lwh = mtod(mfrag, struct ieee80211_frame *);
		last_rxseq = le16toh(*(uint16_t *)lwh->i_seq);
		/* NB: check seq # and frag together */
		if (rxseq != last_rxseq+1 ||
		    !IEEE80211_ADDR_EQ(wh->i_addr1, lwh->i_addr1) ||
		    !IEEE80211_ADDR_EQ(wh->i_addr2, lwh->i_addr2)) {
			/*
			 * Unrelated fragment or no space for it,
			 * clear current fragments.
			 */
			m_freem(mfrag);
			mfrag = NULL;
		}
	}

 	if (mfrag == NULL) {
		if (fragno != 0) {		/* !first fragment, discard */
			vap->iv_stats.is_rx_defrag++;
			IEEE80211_NODE_STAT(ni, rx_defrag);
			m_freem(m);
			return NULL;
		}
		mfrag = m;
	} else {				/* concatenate */
		m_adj(m, hdrspace);		/* strip header */
		m_cat(mfrag, m);
		/* NB: m_cat doesn't update the packet header */
		mfrag->m_pkthdr.len += m->m_pkthdr.len;
		/* track last seqnum and fragno */
		lwh = mtod(mfrag, struct ieee80211_frame *);
		*(uint16_t *) lwh->i_seq = *(uint16_t *) wh->i_seq;
	}
	if (more_frag) {			/* more to come, save */
		ni->ni_rxfragstamp = ticks;
		ni->ni_rxfrag[0] = mfrag;
		mfrag = NULL;
	}
	return mfrag;
}

void
ieee80211_deliver_data(struct ieee80211vap *vap,
	struct ieee80211_node *ni, struct mbuf *m)
{
	struct ether_header *eh = mtod(m, struct ether_header *);
	struct ifnet *ifp = vap->iv_ifp;

	/* NB: see hostap_deliver_data, this path doesn't handle hostap */
	KASSERT(vap->iv_opmode != IEEE80211_M_HOSTAP, ("gack, hostap"));
	/*
	 * Do accounting.
	 */
	ifp->if_ipackets++;
	IEEE80211_NODE_STAT(ni, rx_data);
	IEEE80211_NODE_STAT_ADD(ni, rx_bytes, m->m_pkthdr.len);
	if (ETHER_IS_MULTICAST(eh->ether_dhost)) {
		m->m_flags |= M_MCAST;		/* XXX M_BCAST? */
		IEEE80211_NODE_STAT(ni, rx_mcast);
	} else
		IEEE80211_NODE_STAT(ni, rx_ucast);
	m->m_pkthdr.rcvif = ifp;

	/* clear driver/net80211 flags before passing up */
	m->m_flags &= ~M_80211_RX;

	if (ni->ni_vlan != 0) {
		/* attach vlan tag */
		m->m_pkthdr.ether_vtag = ni->ni_vlan;
		m->m_flags |= M_VLANTAG;
	}
	ifp->if_input(ifp, m);
}

struct mbuf *
ieee80211_decap(struct ieee80211vap *vap, struct mbuf *m, int hdrlen)
{
	struct ieee80211_qosframe_addr4 wh;	/* Max size address frames */
	struct ether_header *eh;
	struct llc *llc;

	if (m->m_len < hdrlen + sizeof(*llc) &&
	    (m = m_pullup(m, hdrlen + sizeof(*llc))) == NULL) {
		/* XXX stat, msg */
		return NULL;
	}
	memcpy(&wh, mtod(m, caddr_t), hdrlen);
	llc = (struct llc *)(mtod(m, caddr_t) + hdrlen);
	if (llc->llc_dsap == LLC_SNAP_LSAP && llc->llc_ssap == LLC_SNAP_LSAP &&
	    llc->llc_control == LLC_UI && llc->llc_snap.org_code[0] == 0 &&
	    llc->llc_snap.org_code[1] == 0 && llc->llc_snap.org_code[2] == 0 &&
	    /* NB: preserve AppleTalk frames that have a native SNAP hdr */
	    !(llc->llc_snap.ether_type == htons(ETHERTYPE_AARP) ||
	      llc->llc_snap.ether_type == htons(ETHERTYPE_IPX))) {
		m_adj(m, hdrlen + sizeof(struct llc) - sizeof(*eh));
		llc = NULL;
	} else {
		m_adj(m, hdrlen - sizeof(*eh));
	}
	eh = mtod(m, struct ether_header *);
	switch (wh.i_fc[1] & IEEE80211_FC1_DIR_MASK) {
	case IEEE80211_FC1_DIR_NODS:
		IEEE80211_ADDR_COPY(eh->ether_dhost, wh.i_addr1);
		IEEE80211_ADDR_COPY(eh->ether_shost, wh.i_addr2);
		break;
	case IEEE80211_FC1_DIR_TODS:
		IEEE80211_ADDR_COPY(eh->ether_dhost, wh.i_addr3);
		IEEE80211_ADDR_COPY(eh->ether_shost, wh.i_addr2);
		break;
	case IEEE80211_FC1_DIR_FROMDS:
		IEEE80211_ADDR_COPY(eh->ether_dhost, wh.i_addr1);
		IEEE80211_ADDR_COPY(eh->ether_shost, wh.i_addr3);
		break;
	case IEEE80211_FC1_DIR_DSTODS:
		IEEE80211_ADDR_COPY(eh->ether_dhost, wh.i_addr3);
		IEEE80211_ADDR_COPY(eh->ether_shost, wh.i_addr4);
		break;
	}
#ifdef ALIGNED_POINTER
	if (!ALIGNED_POINTER(mtod(m, caddr_t) + sizeof(*eh), uint32_t)) {
		struct mbuf *n, *n0, **np;
		caddr_t newdata;
		int off, pktlen;

		n0 = NULL;
		np = &n0;
		off = 0;
		pktlen = m->m_pkthdr.len;
		while (pktlen > off) {
			if (n0 == NULL) {
				MGETHDR(n, M_DONTWAIT, MT_DATA);
				if (n == NULL) {
					m_freem(m);
					return NULL;
				}
				M_MOVE_PKTHDR(n, m);
				n->m_len = MHLEN;
			} else {
				MGET(n, M_DONTWAIT, MT_DATA);
				if (n == NULL) {
					m_freem(m);
					m_freem(n0);
					return NULL;
				}
				n->m_len = MLEN;
			}
			if (pktlen - off >= MINCLSIZE) {
				MCLGET(n, M_DONTWAIT);
				if (n->m_flags & M_EXT)
					n->m_len = n->m_ext.ext_size;
			}
			if (n0 == NULL) {
				newdata =
				    (caddr_t)ALIGN(n->m_data + sizeof(*eh)) -
				    sizeof(*eh);
				n->m_len -= newdata - n->m_data;
				n->m_data = newdata;
			}
			if (n->m_len > pktlen - off)
				n->m_len = pktlen - off;
			m_copydata(m, off, n->m_len, mtod(n, caddr_t));
			off += n->m_len;
			*np = n;
			np = &n->m_next;
		}
		m_freem(m);
		m = n0;
	}
#endif /* ALIGNED_POINTER */
	if (llc != NULL) {
		eh = mtod(m, struct ether_header *);
		eh->ether_type = htons(m->m_pkthdr.len - sizeof(*eh));
	}
	return m;
}

/*
 * Decap a frame encapsulated in a fast-frame/A-MSDU.
 */
struct mbuf *
ieee80211_decap1(struct mbuf *m, int *framelen)
{
#define	FF_LLC_SIZE	(sizeof(struct ether_header) + sizeof(struct llc))
	struct ether_header *eh;
	struct llc *llc;

	/*
	 * The frame has an 802.3 header followed by an 802.2
	 * LLC header.  The encapsulated frame length is in the
	 * first header type field; save that and overwrite it 
	 * with the true type field found in the second.  Then
	 * copy the 802.3 header up to where it belongs and
	 * adjust the mbuf contents to remove the void.
	 */
	if (m->m_len < FF_LLC_SIZE && (m = m_pullup(m, FF_LLC_SIZE)) == NULL)
		return NULL;
	eh = mtod(m, struct ether_header *);	/* 802.3 header is first */
	llc = (struct llc *)&eh[1];		/* 802.2 header follows */
	*framelen = ntohs(eh->ether_type)	/* encap'd frame size */
		  + sizeof(struct ether_header) - sizeof(struct llc);
	eh->ether_type = llc->llc_un.type_snap.ether_type;
	ovbcopy(eh, mtod(m, uint8_t *) + sizeof(struct llc),
		sizeof(struct ether_header));
	m_adj(m, sizeof(struct llc));
	return m;
#undef FF_LLC_SIZE
}

/*
 * Decap the encapsulated frame pair and dispatch the first
 * for delivery.  The second frame is returned for delivery
 * via the normal path.
 */
struct mbuf *
ieee80211_decap_fastframe(struct ieee80211_node *ni, struct mbuf *m)
{
#define	MS(x,f)	(((x) & f) >> f##_S)
	struct ieee80211vap *vap = ni->ni_vap;
	uint32_t ath;
	struct mbuf *n;
	int framelen;

	m_copydata(m, 0, sizeof(uint32_t), (caddr_t) &ath);
	if (MS(ath, ATH_FF_PROTO) != ATH_FF_PROTO_L2TUNNEL) {
		IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_ANY,
		    ni->ni_macaddr, "fast-frame",
		    "unsupport tunnel protocol, header 0x%x", ath);
		vap->iv_stats.is_ff_badhdr++;
		m_freem(m);
		return NULL;
	}
	/* NB: skip header and alignment padding */
	m_adj(m, roundup(sizeof(uint32_t) - 2, 4) + 2);

	vap->iv_stats.is_ff_decap++;

	/*
	 * Decap the first frame, bust it apart from the
	 * second and deliver; then decap the second frame
	 * and return it to the caller for normal delivery.
	 */
	m = ieee80211_decap1(m, &framelen);
	if (m == NULL) {
		IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_ANY,
		    ni->ni_macaddr, "fast-frame", "%s", "first decap failed");
		vap->iv_stats.is_ff_tooshort++;
		return NULL;
	}
	n = m_split(m, framelen, M_NOWAIT);
	if (n == NULL) {
		IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_ANY,
		    ni->ni_macaddr, "fast-frame",
		    "%s", "unable to split encapsulated frames");
		vap->iv_stats.is_ff_split++;
		m_freem(m);			/* NB: must reclaim */
		return NULL;
	}
	/* XXX not right for WDS */
	vap->iv_deliver_data(vap, ni, m);	/* 1st of pair */

	/*
	 * Decap second frame.
	 */
	m_adj(n, roundup2(framelen, 4) - framelen);	/* padding */
	n = ieee80211_decap1(n, &framelen);
	if (n == NULL) {
		IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_ANY,
		    ni->ni_macaddr, "fast-frame", "%s", "second decap failed");
		vap->iv_stats.is_ff_tooshort++;
	}
	/* XXX verify framelen against mbuf contents */
	return n;				/* 2nd delivered by caller */
#undef MS
}

/*
 * Install received rate set information in the node's state block.
 */
int
ieee80211_setup_rates(struct ieee80211_node *ni,
	const uint8_t *rates, const uint8_t *xrates, int flags)
{
	struct ieee80211vap *vap = ni->ni_vap;
	struct ieee80211_rateset *rs = &ni->ni_rates;

	memset(rs, 0, sizeof(*rs));
	rs->rs_nrates = rates[1];
	memcpy(rs->rs_rates, rates + 2, rs->rs_nrates);
	if (xrates != NULL) {
		uint8_t nxrates;
		/*
		 * Tack on 11g extended supported rate element.
		 */
		nxrates = xrates[1];
		if (rs->rs_nrates + nxrates > IEEE80211_RATE_MAXSIZE) {
			nxrates = IEEE80211_RATE_MAXSIZE - rs->rs_nrates;
			IEEE80211_NOTE(vap, IEEE80211_MSG_XRATE, ni,
			    "extended rate set too large; only using "
			    "%u of %u rates", nxrates, xrates[1]);
			vap->iv_stats.is_rx_rstoobig++;
		}
		memcpy(rs->rs_rates + rs->rs_nrates, xrates+2, nxrates);
		rs->rs_nrates += nxrates;
	}
	return ieee80211_fix_rate(ni, rs, flags);
}

/*
 * Send a management frame error response to the specified
 * station.  If ni is associated with the station then use
 * it; otherwise allocate a temporary node suitable for
 * transmitting the frame and then free the reference so
 * it will go away as soon as the frame has been transmitted.
 */
void
ieee80211_send_error(struct ieee80211_node *ni,
	const uint8_t mac[IEEE80211_ADDR_LEN], int subtype, int arg)
{
	struct ieee80211vap *vap = ni->ni_vap;
	int istmp;

	if (ni == vap->iv_bss) {
		if (vap->iv_state != IEEE80211_S_RUN) {
			/*
			 * XXX hack until we get rid of this routine.
			 * We can be called prior to the vap reaching
			 * run state under certain conditions in which
			 * case iv_bss->ni_chan will not be setup.
			 * Check for this explicitly and and just ignore
			 * the request.
			 */
			return;
		}
		ni = ieee80211_tmp_node(vap, mac);
		if (ni == NULL) {
			/* XXX msg */
			return;
		}
		istmp = 1;
	} else
		istmp = 0;
	IEEE80211_SEND_MGMT(ni, subtype, arg);
	if (istmp)
		ieee80211_free_node(ni);
}

int
ieee80211_alloc_challenge(struct ieee80211_node *ni)
{
	if (ni->ni_challenge == NULL)
		ni->ni_challenge = (uint32_t *) malloc(IEEE80211_CHALLENGE_LEN,
		    M_80211_NODE, M_NOWAIT);
	if (ni->ni_challenge == NULL) {
		IEEE80211_NOTE(ni->ni_vap,
		    IEEE80211_MSG_DEBUG | IEEE80211_MSG_AUTH, ni,
		    "%s", "shared key challenge alloc failed");
		/* XXX statistic */
	}
	return (ni->ni_challenge != NULL);
}

void
ieee80211_parse_ath(struct ieee80211_node *ni, uint8_t *ie)
{
	const struct ieee80211_ath_ie *ath =
		(const struct ieee80211_ath_ie *) ie;

	ni->ni_ath_flags = ath->ath_capability;
	ni->ni_ath_defkeyix = LE_READ_2(&ath->ath_defkeyix);
}

/*
 * Parse a Beacon or ProbeResponse frame and return the
 * useful information in an ieee80211_scanparams structure.
 * Status is set to 0 if no problems were found; otherwise
 * a bitmask of IEEE80211_BPARSE_* items is returned that
 * describes the problems detected.
 */
int
ieee80211_parse_beacon(struct ieee80211_node *ni, struct mbuf *m,
	struct ieee80211_scanparams *scan)
{
	struct ieee80211vap *vap = ni->ni_vap;
	struct ieee80211com *ic = ni->ni_ic;
	struct ieee80211_frame *wh;
	uint8_t *frm, *efrm;

	wh = mtod(m, struct ieee80211_frame *);
	frm = (uint8_t *)&wh[1];
	efrm = mtod(m, uint8_t *) + m->m_len;
	scan->status = 0;
	/*
	 * beacon/probe response frame format
	 *	[8] time stamp
	 *	[2] beacon interval
	 *	[2] capability information
	 *	[tlv] ssid
	 *	[tlv] supported rates
	 *	[tlv] country information
	 *	[tlv] parameter set (FH/DS)
	 *	[tlv] erp information
	 *	[tlv] extended supported rates
	 *	[tlv] WME
	 *	[tlv] WPA or RSN
	 *	[tlv] HT capabilities
	 *	[tlv] HT information
	 *	[tlv] Atheros capabilities
	 */
	IEEE80211_VERIFY_LENGTH(efrm - frm, 12,
	    return (scan->status = IEEE80211_BPARSE_BADIELEN));
	memset(scan, 0, sizeof(*scan));
	scan->tstamp  = frm;				frm += 8;
	scan->bintval = le16toh(*(uint16_t *)frm);	frm += 2;
	scan->capinfo = le16toh(*(uint16_t *)frm);	frm += 2;
	scan->bchan = ieee80211_chan2ieee(ic, ic->ic_curchan);
	scan->chan = scan->bchan;
	scan->ies = frm;
	scan->ies_len = efrm - frm;

	while (efrm - frm > 1) {
		IEEE80211_VERIFY_LENGTH(efrm - frm, frm[1] + 2,
		    return (scan->status = IEEE80211_BPARSE_BADIELEN));
		switch (*frm) {
		case IEEE80211_ELEMID_SSID:
			scan->ssid = frm;
			break;
		case IEEE80211_ELEMID_RATES:
			scan->rates = frm;
			break;
		case IEEE80211_ELEMID_COUNTRY:
			scan->country = frm;
			break;
		case IEEE80211_ELEMID_FHPARMS:
			if (ic->ic_phytype == IEEE80211_T_FH) {
				scan->fhdwell = LE_READ_2(&frm[2]);
				scan->chan = IEEE80211_FH_CHAN(frm[4], frm[5]);
				scan->fhindex = frm[6];
			}
			break;
		case IEEE80211_ELEMID_DSPARMS:
			/*
			 * XXX hack this since depending on phytype
			 * is problematic for multi-mode devices.
			 */
			if (ic->ic_phytype != IEEE80211_T_FH)
				scan->chan = frm[2];
			break;
		case IEEE80211_ELEMID_TIM:
			/* XXX ATIM? */
			scan->tim = frm;
			scan->timoff = frm - mtod(m, uint8_t *);
			break;
		case IEEE80211_ELEMID_IBSSPARMS:
		case IEEE80211_ELEMID_CFPARMS:
		case IEEE80211_ELEMID_PWRCNSTR:
			/* NB: avoid debugging complaints */
			break;
		case IEEE80211_ELEMID_XRATES:
			scan->xrates = frm;
			break;
		case IEEE80211_ELEMID_ERP:
			if (frm[1] != 1) {
				IEEE80211_DISCARD_IE(vap,
				    IEEE80211_MSG_ELEMID, wh, "ERP",
				    "bad len %u", frm[1]);
				vap->iv_stats.is_rx_elem_toobig++;
				break;
			}
			scan->erp = frm[2] | 0x100;
			break;
		case IEEE80211_ELEMID_HTCAP:
			scan->htcap = frm;
			break;
		case IEEE80211_ELEMID_RSN:
			scan->rsn = frm;
			break;
		case IEEE80211_ELEMID_HTINFO:
			scan->htinfo = frm;
			break;
		case IEEE80211_ELEMID_VENDOR:
			if (iswpaoui(frm))
				scan->wpa = frm;
			else if (iswmeparam(frm) || iswmeinfo(frm))
				scan->wme = frm;
			else if (isatherosoui(frm))
				scan->ath = frm;
#ifdef IEEE80211_SUPPORT_TDMA
			else if (istdmaoui(frm))
				scan->tdma = frm;
#endif
			else if (vap->iv_flags_ext & IEEE80211_FEXT_HTCOMPAT) {
				/*
				 * Accept pre-draft HT ie's if the
				 * standard ones have not been seen.
				 */
				if (ishtcapoui(frm)) {
					if (scan->htcap == NULL)
						scan->htcap = frm;
				} else if (ishtinfooui(frm)) {
					if (scan->htinfo == NULL)
						scan->htcap = frm;
				}
			}
			break;
		default:
			IEEE80211_DISCARD_IE(vap, IEEE80211_MSG_ELEMID,
			    wh, "unhandled",
			    "id %u, len %u", *frm, frm[1]);
			vap->iv_stats.is_rx_elem_unknown++;
			break;
		}
		frm += frm[1] + 2;
	}
	IEEE80211_VERIFY_ELEMENT(scan->rates, IEEE80211_RATE_MAXSIZE,
	    scan->status |= IEEE80211_BPARSE_RATES_INVALID);
	if (scan->rates != NULL && scan->xrates != NULL) {
		/*
		 * NB: don't process XRATES if RATES is missing.  This
		 * avoids a potential null ptr deref and should be ok
		 * as the return code will already note RATES is missing
		 * (so callers shouldn't otherwise process the frame).
		 */
		IEEE80211_VERIFY_ELEMENT(scan->xrates,
		    IEEE80211_RATE_MAXSIZE - scan->rates[1],
		    scan->status |= IEEE80211_BPARSE_XRATES_INVALID);
	}
	IEEE80211_VERIFY_ELEMENT(scan->ssid, IEEE80211_NWID_LEN,
	    scan->status |= IEEE80211_BPARSE_SSID_INVALID);
	if (scan->chan != scan->bchan && ic->ic_phytype != IEEE80211_T_FH) {
		/*
		 * Frame was received on a channel different from the
		 * one indicated in the DS params element id;
		 * silently discard it.
		 *
		 * NB: this can happen due to signal leakage.
		 *     But we should take it for FH phy because
		 *     the rssi value should be correct even for
		 *     different hop pattern in FH.
		 */
		IEEE80211_DISCARD(vap,
		    IEEE80211_MSG_ELEMID | IEEE80211_MSG_INPUT,
		    wh, NULL, "for off-channel %u", scan->chan);
		vap->iv_stats.is_rx_chanmismatch++;
		scan->status |= IEEE80211_BPARSE_OFFCHAN;
	}
	if (!(IEEE80211_BINTVAL_MIN <= scan->bintval &&
	      scan->bintval <= IEEE80211_BINTVAL_MAX)) {
		IEEE80211_DISCARD(vap,
		    IEEE80211_MSG_ELEMID | IEEE80211_MSG_INPUT,
		    wh, NULL, "bogus beacon interval", scan->bintval);
		vap->iv_stats.is_rx_badbintval++;
		scan->status |= IEEE80211_BPARSE_BINTVAL_INVALID;
	}
	if (scan->country != NULL) {
		/*
		 * Validate we have at least enough data to extract
		 * the country code.  Not sure if we should return an
		 * error instead of discarding the IE; consider this
		 * being lenient as we don't depend on the data for
		 * correct operation.
		 */
		IEEE80211_VERIFY_LENGTH(scan->country[1], 3 * sizeof(uint8_t),
		    scan->country = NULL);
	}
	/*
	 * Process HT ie's.  This is complicated by our
	 * accepting both the standard ie's and the pre-draft
	 * vendor OUI ie's that some vendors still use/require.
	 */
	if (scan->htcap != NULL) {
		IEEE80211_VERIFY_LENGTH(scan->htcap[1],
		     scan->htcap[0] == IEEE80211_ELEMID_VENDOR ?
			 4 + sizeof(struct ieee80211_ie_htcap)-2 :
			 sizeof(struct ieee80211_ie_htcap)-2,
		     scan->htcap = NULL);
	}
	if (scan->htinfo != NULL) {
		IEEE80211_VERIFY_LENGTH(scan->htinfo[1],
		     scan->htinfo[0] == IEEE80211_ELEMID_VENDOR ?
			 4 + sizeof(struct ieee80211_ie_htinfo)-2 :
			 sizeof(struct ieee80211_ie_htinfo)-2,
		     scan->htinfo = NULL);
	}
	return scan->status;
}

/*
 * Parse an Action frame.  Return 0 on success, non-zero on failure.
 */
int
ieee80211_parse_action(struct ieee80211_node *ni, struct mbuf *m)
{
	struct ieee80211vap *vap = ni->ni_vap;
	const struct ieee80211_action *ia;
	struct ieee80211_frame *wh;
	uint8_t *frm, *efrm;

	/*
	 * action frame format:
	 *	[1] category
	 *	[1] action
	 *	[tlv] parameters
	 */
	wh = mtod(m, struct ieee80211_frame *);
	frm = (u_int8_t *)&wh[1];
	efrm = mtod(m, u_int8_t *) + m->m_len;
	IEEE80211_VERIFY_LENGTH(efrm - frm,
		sizeof(struct ieee80211_action), return EINVAL);
	ia = (const struct ieee80211_action *) frm;

	vap->iv_stats.is_rx_action++;
	IEEE80211_NODE_STAT(ni, rx_action);

	/* verify frame payloads but defer processing */
	/* XXX maybe push this to method */
	switch (ia->ia_category) {
	case IEEE80211_ACTION_CAT_BA:
		switch (ia->ia_action) {
		case IEEE80211_ACTION_BA_ADDBA_REQUEST:
			IEEE80211_VERIFY_LENGTH(efrm - frm,
			    sizeof(struct ieee80211_action_ba_addbarequest),
			    return EINVAL);
			break;
		case IEEE80211_ACTION_BA_ADDBA_RESPONSE:
			IEEE80211_VERIFY_LENGTH(efrm - frm,
			    sizeof(struct ieee80211_action_ba_addbaresponse),
			    return EINVAL);
			break;
		case IEEE80211_ACTION_BA_DELBA:
			IEEE80211_VERIFY_LENGTH(efrm - frm,
			    sizeof(struct ieee80211_action_ba_delba),
			    return EINVAL);
			break;
		}
		break;
	case IEEE80211_ACTION_CAT_HT:
		switch (ia->ia_action) {
		case IEEE80211_ACTION_HT_TXCHWIDTH:
			IEEE80211_VERIFY_LENGTH(efrm - frm,
			    sizeof(struct ieee80211_action_ht_txchwidth),
			    return EINVAL);
			break;
		case IEEE80211_ACTION_HT_MIMOPWRSAVE:
			IEEE80211_VERIFY_LENGTH(efrm - frm,
			    sizeof(struct ieee80211_action_ht_mimopowersave),
			    return EINVAL);
			break;
		}
		break;
	}
	return 0;
}

#ifdef IEEE80211_DEBUG
/*
 * Debugging support.
 */
void
ieee80211_ssid_mismatch(struct ieee80211vap *vap, const char *tag,
	uint8_t mac[IEEE80211_ADDR_LEN], uint8_t *ssid)
{
	printf("[%s] discard %s frame, ssid mismatch: ",
		ether_sprintf(mac), tag);
	ieee80211_print_essid(ssid + 2, ssid[1]);
	printf("\n");
}

/*
 * Return the bssid of a frame.
 */
static const uint8_t *
ieee80211_getbssid(struct ieee80211vap *vap, const struct ieee80211_frame *wh)
{
	if (vap->iv_opmode == IEEE80211_M_STA)
		return wh->i_addr2;
	if ((wh->i_fc[1] & IEEE80211_FC1_DIR_MASK) != IEEE80211_FC1_DIR_NODS)
		return wh->i_addr1;
	if ((wh->i_fc[0] & IEEE80211_FC0_SUBTYPE_MASK) == IEEE80211_FC0_SUBTYPE_PS_POLL)
		return wh->i_addr1;
	return wh->i_addr3;
}

#include <machine/stdarg.h>

void
ieee80211_note(struct ieee80211vap *vap, const char *fmt, ...)
{
	char buf[128];		/* XXX */
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	if_printf(vap->iv_ifp, "%s", buf);	/* NB: no \n */
}

void
ieee80211_note_frame(struct ieee80211vap *vap,
	const struct ieee80211_frame *wh,
	const char *fmt, ...)
{
	char buf[128];		/* XXX */
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	if_printf(vap->iv_ifp, "[%s] %s\n",
		ether_sprintf(ieee80211_getbssid(vap, wh)), buf);
}

void
ieee80211_note_mac(struct ieee80211vap *vap,
	const uint8_t mac[IEEE80211_ADDR_LEN],
	const char *fmt, ...)
{
	char buf[128];		/* XXX */
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	if_printf(vap->iv_ifp, "[%s] %s\n", ether_sprintf(mac), buf);
}

void
ieee80211_discard_frame(struct ieee80211vap *vap,
	const struct ieee80211_frame *wh,
	const char *type, const char *fmt, ...)
{
	va_list ap;

	if_printf(vap->iv_ifp, "[%s] discard ",
		ether_sprintf(ieee80211_getbssid(vap, wh)));
	if (type == NULL) {
		printf("%s frame, ", ieee80211_mgt_subtype_name[
			(wh->i_fc[0] & IEEE80211_FC0_SUBTYPE_MASK) >>
			IEEE80211_FC0_SUBTYPE_SHIFT]);
	} else
		printf("%s frame, ", type);
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	printf("\n");
}

void
ieee80211_discard_ie(struct ieee80211vap *vap,
	const struct ieee80211_frame *wh,
	const char *type, const char *fmt, ...)
{
	va_list ap;

	if_printf(vap->iv_ifp, "[%s] discard ",
		ether_sprintf(ieee80211_getbssid(vap, wh)));
	if (type != NULL)
		printf("%s information element, ", type);
	else
		printf("information element, ");
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	printf("\n");
}

void
ieee80211_discard_mac(struct ieee80211vap *vap,
	const uint8_t mac[IEEE80211_ADDR_LEN],
	const char *type, const char *fmt, ...)
{
	va_list ap;

	if_printf(vap->iv_ifp, "[%s] discard ", ether_sprintf(mac));
	if (type != NULL)
		printf("%s frame, ", type);
	else
		printf("frame, ");
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	printf("\n");
}
#endif /* IEEE80211_DEBUG */
