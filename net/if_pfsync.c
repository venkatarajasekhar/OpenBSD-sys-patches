/*	$OpenBSD: if_pfsync.c,v 1.123 2009/05/13 01:09:05 dlg Exp $	*/

/*
 * Copyright (c) 2002 Michael Shalayeff
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
 * IN NO EVENT SHALL THE AUTHOR OR HIS RELATIVES BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF MIND, USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Copyright (c) 2009 David Gwynne <dlg@openbsd.org>
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

#include <sys/param.h>
#include <sys/proc.h>
#include <sys/systm.h>
#include <sys/time.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/timeout.h>
#include <sys/kernel.h>
#include <sys/sysctl.h>
#include <sys/pool.h>

#include <net/if.h>
#include <net/if_types.h>
#include <net/route.h>
#include <net/bpf.h>
#include <net/netisr.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/tcp.h>
#include <netinet/tcp_seq.h>

#ifdef	INET
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#endif

#ifdef INET6
#include <netinet6/nd6.h>
#endif /* INET6 */

#include "carp.h"
#if NCARP > 0
#include <netinet/ip_carp.h>
#endif

#include <net/pfvar.h>
#include <net/if_pfsync.h>

#include "bpfilter.h"
#include "pfsync.h"

#define PFSYNC_MINPKT ( \
	sizeof(struct ip) + \
	sizeof(struct pfsync_header) + \
	sizeof(struct pfsync_subheader))

struct pfsync_pkt {
	struct ip *ip;
	struct in_addr src;
	u_int8_t flags;
};

int	pfsync_upd_tcp(struct pf_state *, struct pfsync_state_peer *,
	    struct pfsync_state_peer *);

int	pfsync_in_clr(struct pfsync_pkt *, struct mbuf *, int, int);
int	pfsync_in_ins(struct pfsync_pkt *, struct mbuf *, int, int);
int	pfsync_in_iack(struct pfsync_pkt *, struct mbuf *, int, int);
int	pfsync_in_upd(struct pfsync_pkt *, struct mbuf *, int, int);
int	pfsync_in_upd_c(struct pfsync_pkt *, struct mbuf *, int, int);
int	pfsync_in_ureq(struct pfsync_pkt *, struct mbuf *, int, int);
int	pfsync_in_del(struct pfsync_pkt *, struct mbuf *, int, int);
int	pfsync_in_del_c(struct pfsync_pkt *, struct mbuf *, int, int);
int	pfsync_in_bus(struct pfsync_pkt *, struct mbuf *, int, int);
int	pfsync_in_tdb(struct pfsync_pkt *, struct mbuf *, int, int);
int	pfsync_in_eof(struct pfsync_pkt *, struct mbuf *, int, int);

int	pfsync_in_error(struct pfsync_pkt *, struct mbuf *, int, int);

int	(*pfsync_acts[])(struct pfsync_pkt *, struct mbuf *, int, int) = {
	pfsync_in_clr,			/* PFSYNC_ACT_CLR */
	pfsync_in_ins,			/* PFSYNC_ACT_INS */
	pfsync_in_iack,			/* PFSYNC_ACT_INS_ACK */
	pfsync_in_upd,			/* PFSYNC_ACT_UPD */
	pfsync_in_upd_c,		/* PFSYNC_ACT_UPD_C */
	pfsync_in_ureq,			/* PFSYNC_ACT_UPD_REQ */
	pfsync_in_del,			/* PFSYNC_ACT_DEL */
	pfsync_in_del_c,		/* PFSYNC_ACT_DEL_C */
	pfsync_in_error,		/* PFSYNC_ACT_INS_F */
	pfsync_in_error,		/* PFSYNC_ACT_DEL_F */
	pfsync_in_bus,			/* PFSYNC_ACT_BUS */
	pfsync_in_tdb,			/* PFSYNC_ACT_TDB */
	pfsync_in_eof			/* PFSYNC_ACT_EOF */
};

struct pfsync_q {
	void		(*write)(struct pf_state *, void *);
	size_t		len;
	u_int8_t	action;
};

/* we have one of these for every PFSYNC_S_ */
void	pfsync_out_state(struct pf_state *, void *);
void	pfsync_out_iack(struct pf_state *, void *);
void	pfsync_out_upd_c(struct pf_state *, void *);
void	pfsync_out_del(struct pf_state *, void *);

struct pfsync_q pfsync_qs[] = {
	{ pfsync_out_state, sizeof(struct pfsync_state),   PFSYNC_ACT_INS },
	{ pfsync_out_iack,  sizeof(struct pfsync_ins_ack), PFSYNC_ACT_INS_ACK },
	{ pfsync_out_state, sizeof(struct pfsync_state),   PFSYNC_ACT_UPD },
	{ pfsync_out_upd_c, sizeof(struct pfsync_upd_c),   PFSYNC_ACT_UPD_C },
	{ pfsync_out_del,   sizeof(struct pfsync_del_c),   PFSYNC_ACT_DEL_C }
};

void	pfsync_q_ins(struct pf_state *, int);
void	pfsync_q_del(struct pf_state *);

struct pfsync_upd_req_item {
	TAILQ_ENTRY(pfsync_upd_req_item)	ur_entry;
	struct pfsync_upd_req			ur_msg;
};
TAILQ_HEAD(pfsync_upd_reqs, pfsync_upd_req_item);

struct pfsync_deferral {
	TAILQ_ENTRY(pfsync_deferral)		 pd_entry;
	struct pf_state				*pd_st;
	struct mbuf				*pd_m;
	struct timeout				 pd_tmo;
};
TAILQ_HEAD(pfsync_deferrals, pfsync_deferral);

#define PFSYNC_PLSIZE	MAX(sizeof(struct pfsync_upd_req_item), \
			    sizeof(struct pfsync_deferral))

void	pfsync_out_tdb(struct tdb *, void *);

struct pfsync_softc {
	struct ifnet		 sc_if;
	struct ifnet		*sc_sync_if;

	struct pool		 sc_pool;

	struct ip_moptions	 sc_imo;

	struct in_addr		 sc_sync_peer;
	u_int8_t		 sc_maxupdates;

	struct ip		 sc_template;

	struct pf_state_queue	 sc_qs[PFSYNC_S_COUNT];
	size_t			 sc_len;

	struct pfsync_upd_reqs	 sc_upd_req_list;

	struct pfsync_deferrals	 sc_deferrals;
	u_int			 sc_deferred;

	void			*sc_plus;
	size_t			 sc_pluslen;

	u_int32_t		 sc_ureq_sent;
	int			 sc_bulk_tries;
	struct timeout		 sc_bulkfail_tmo;

	u_int32_t		 sc_ureq_received;
	struct pf_state		*sc_bulk_next;
	struct pf_state		*sc_bulk_last;
	struct timeout		 sc_bulk_tmo;

	TAILQ_HEAD(, tdb)	 sc_tdb_q;

	struct timeout		 sc_tmo;
};

struct pfsync_softc	*pfsyncif = NULL;
struct pfsyncstats	 pfsyncstats;

void	pfsyncattach(int);
int	pfsync_clone_create(struct if_clone *, int);
int	pfsync_clone_destroy(struct ifnet *);
int	pfsync_alloc_scrub_memory(struct pfsync_state_peer *,
	    struct pf_state_peer *);
void	pfsync_update_net_tdb(struct pfsync_tdb *);
int	pfsyncoutput(struct ifnet *, struct mbuf *, struct sockaddr *,
	    struct rtentry *);
int	pfsyncioctl(struct ifnet *, u_long, caddr_t);
void	pfsyncstart(struct ifnet *);

struct mbuf *pfsync_if_dequeue(struct ifnet *);
struct mbuf *pfsync_get_mbuf(struct pfsync_softc *);

void	pfsync_deferred(struct pf_state *, int);
void	pfsync_undefer(struct pfsync_deferral *, int);
void	pfsync_defer_tmo(void *);

void	pfsync_request_update(u_int32_t, u_int64_t);
void	pfsync_update_state_req(struct pf_state *);

void	pfsync_drop(struct pfsync_softc *);
void	pfsync_sendout(void);
void	pfsync_send_plus(void *, size_t);
int	pfsync_tdb_sendout(struct pfsync_softc *);
int	pfsync_sendout_mbuf(struct pfsync_softc *, struct mbuf *);
void	pfsync_timeout(void *);
void	pfsync_tdb_timeout(void *);
void	pfsync_send_bus(struct pfsync_softc *, u_int8_t);

void	pfsync_bulk_start(void);
void	pfsync_bulk_status(u_int8_t);
void	pfsync_bulk_update(void *);
void	pfsync_bulk_fail(void *);

#define PFSYNC_MAX_BULKTRIES	12
int	pfsync_sync_ok;

struct if_clone	pfsync_cloner =
    IF_CLONE_INITIALIZER("pfsync", pfsync_clone_create, pfsync_clone_destroy);

void
pfsyncattach(int npfsync)
{
	if_clone_attach(&pfsync_cloner);
}
int
pfsync_clone_create(struct if_clone *ifc, int unit)
{
	struct pfsync_softc *sc;
	struct ifnet *ifp;
	int q;

	if (unit != 0)
		return (EINVAL);

	pfsync_sync_ok = 1;

	sc = malloc(sizeof(*pfsyncif), M_DEVBUF, M_NOWAIT | M_ZERO);
	if (sc == NULL)
		return (ENOMEM);

	for (q = 0; q < PFSYNC_S_COUNT; q++)
		TAILQ_INIT(&sc->sc_qs[q]);

	pool_init(&sc->sc_pool, PFSYNC_PLSIZE, 0, 0, 0, "pfsync", NULL);
	TAILQ_INIT(&sc->sc_upd_req_list);
	TAILQ_INIT(&sc->sc_deferrals);
	sc->sc_deferred = 0;

	TAILQ_INIT(&sc->sc_tdb_q);

	sc->sc_len = PFSYNC_MINPKT;
	sc->sc_maxupdates = 128;

	sc->sc_imo.imo_membership = (struct in_multi **)malloc(
	    (sizeof(struct in_multi *) * IP_MIN_MEMBERSHIPS), M_IPMOPTS,
	    M_WAITOK | M_ZERO);
	sc->sc_imo.imo_max_memberships = IP_MIN_MEMBERSHIPS;

	ifp = &sc->sc_if;
	snprintf(ifp->if_xname, sizeof ifp->if_xname, "pfsync%d", unit);
	ifp->if_softc = sc;
	ifp->if_ioctl = pfsyncioctl;
	ifp->if_output = pfsyncoutput;
	ifp->if_start = pfsyncstart;
	ifp->if_type = IFT_PFSYNC;
	ifp->if_snd.ifq_maxlen = ifqmaxlen;
	ifp->if_hdrlen = sizeof(struct pfsync_header);
	ifp->if_mtu = 1500; /* XXX */
	ifp->if_hardmtu = MCLBYTES; /* XXX */
	timeout_set(&sc->sc_tmo, pfsync_timeout, sc);
	timeout_set(&sc->sc_bulk_tmo, pfsync_bulk_update, sc);
	timeout_set(&sc->sc_bulkfail_tmo, pfsync_bulk_fail, sc);

	if_attach(ifp);
	if_alloc_sadl(ifp);

#if NCARP > 0
	if_addgroup(ifp, "carp");
#endif

#if NBPFILTER > 0
	bpfattach(&sc->sc_if.if_bpf, ifp, DLT_PFSYNC, PFSYNC_HDRLEN);
#endif

	pfsyncif = sc;

	return (0);
}

int
pfsync_clone_destroy(struct ifnet *ifp)
{
	struct pfsync_softc *sc = ifp->if_softc;

	timeout_del(&sc->sc_bulk_tmo);
	timeout_del(&sc->sc_tmo);
#if NCARP > 0
	if (!pfsync_sync_ok)
		carp_group_demote_adj(&sc->sc_if, -1);
#endif
#if NBPFILTER > 0
	bpfdetach(ifp);
#endif
	if_detach(ifp);

	pfsync_drop(sc);

	while (sc->sc_deferred > 0)
		pfsync_undefer(TAILQ_FIRST(&sc->sc_deferrals), 0);

	pool_destroy(&sc->sc_pool);
	free(sc->sc_imo.imo_membership, M_IPMOPTS);
	free(sc, M_DEVBUF);

	pfsyncif = NULL;

	return (0);
}

struct mbuf *
pfsync_if_dequeue(struct ifnet *ifp)
{
	struct mbuf *m;

	IF_DEQUEUE(&ifp->if_snd, m);

	return (m);
}

/*
 * Start output on the pfsync interface.
 */
void
pfsyncstart(struct ifnet *ifp)
{
	struct mbuf *m;
	int s;

	s = splnet();
	while ((m = pfsync_if_dequeue(ifp)) != NULL) {
		IF_DROP(&ifp->if_snd);
		m_freem(m);
	}
	splx(s);
}

int
pfsync_alloc_scrub_memory(struct pfsync_state_peer *s,
    struct pf_state_peer *d)
{
	if (s->scrub.scrub_flag && d->scrub == NULL) {
		d->scrub = pool_get(&pf_state_scrub_pl, PR_NOWAIT | PR_ZERO);
		if (d->scrub == NULL)
			return (ENOMEM);
	}

	return (0);
}

void
pfsync_state_export(struct pfsync_state *sp, struct pf_state *st)
{
	bzero(sp, sizeof(struct pfsync_state));

	/* copy from state key */
	sp->key[PF_SK_WIRE].addr[0] = st->key[PF_SK_WIRE]->addr[0];
	sp->key[PF_SK_WIRE].addr[1] = st->key[PF_SK_WIRE]->addr[1];
	sp->key[PF_SK_WIRE].port[0] = st->key[PF_SK_WIRE]->port[0];
	sp->key[PF_SK_WIRE].port[1] = st->key[PF_SK_WIRE]->port[1];
	sp->key[PF_SK_STACK].addr[0] = st->key[PF_SK_STACK]->addr[0];
	sp->key[PF_SK_STACK].addr[1] = st->key[PF_SK_STACK]->addr[1];
	sp->key[PF_SK_STACK].port[0] = st->key[PF_SK_STACK]->port[0];
	sp->key[PF_SK_STACK].port[1] = st->key[PF_SK_STACK]->port[1];
	sp->proto = st->key[PF_SK_WIRE]->proto;
	sp->af = st->key[PF_SK_WIRE]->af;

	/* copy from state */
	strlcpy(sp->ifname, st->kif->pfik_name, sizeof(sp->ifname));
	bcopy(&st->rt_addr, &sp->rt_addr, sizeof(sp->rt_addr));
	sp->creation = htonl(time_second - st->creation);
	sp->expire = pf_state_expires(st);
	if (sp->expire <= time_second)
		sp->expire = htonl(0);
	else
		sp->expire = htonl(sp->expire - time_second);

	sp->direction = st->direction;
	sp->log = st->log;
	sp->timeout = st->timeout;
	sp->state_flags = st->state_flags;
	if (st->src_node)
		sp->sync_flags |= PFSYNC_FLAG_SRCNODE;
	if (st->nat_src_node)
		sp->sync_flags |= PFSYNC_FLAG_NATSRCNODE;

	bcopy(&st->id, &sp->id, sizeof(sp->id));
	sp->creatorid = st->creatorid;
	pf_state_peer_hton(&st->src, &sp->src);
	pf_state_peer_hton(&st->dst, &sp->dst);

	if (st->rule.ptr == NULL)
		sp->rule = htonl(-1);
	else
		sp->rule = htonl(st->rule.ptr->nr);
	if (st->anchor.ptr == NULL)
		sp->anchor = htonl(-1);
	else
		sp->anchor = htonl(st->anchor.ptr->nr);
	if (st->nat_rule.ptr == NULL)
		sp->nat_rule = htonl(-1);
	else
		sp->nat_rule = htonl(st->nat_rule.ptr->nr);

	pf_state_counter_hton(st->packets[0], sp->packets[0]);
	pf_state_counter_hton(st->packets[1], sp->packets[1]);
	pf_state_counter_hton(st->bytes[0], sp->bytes[0]);
	pf_state_counter_hton(st->bytes[1], sp->bytes[1]);

}

int
pfsync_state_import(struct pfsync_state *sp, u_int8_t flags)
{
	struct pf_state	*st = NULL;
	struct pf_state_key *skw = NULL, *sks = NULL;
	struct pf_rule *r = NULL;
	struct pfi_kif	*kif;
	int pool_flags;
	int error;

	if (sp->creatorid == 0 && pf_status.debug >= PF_DEBUG_MISC) {
		printf("pfsync_state_import: invalid creator id:"
		    " %08x\n", ntohl(sp->creatorid));
		return (EINVAL);
	}

	if ((kif = pfi_kif_get(sp->ifname)) == NULL) {
		if (pf_status.debug >= PF_DEBUG_MISC)
			printf("pfsync_state_import: "
			    "unknown interface: %s\n", sp->ifname);
		if (flags & PFSYNC_SI_IOCTL)
			return (EINVAL);
		return (0);	/* skip this state */
	}

	/*
	 * If the ruleset checksums match or the state is coming from the ioctl,
	 * it's safe to associate the state with the rule of that number.
	 */
	if (sp->rule != htonl(-1) && sp->anchor == htonl(-1) &&
	    (flags & (PFSYNC_SI_IOCTL | PFSYNC_SI_CKSUM)) && ntohl(sp->rule) <
	    pf_main_ruleset.rules[PF_RULESET_FILTER].active.rcount)
		r = pf_main_ruleset.rules[
		    PF_RULESET_FILTER].active.ptr_array[ntohl(sp->rule)];
	else
		r = &pf_default_rule;

	if ((r->max_states && r->states_cur >= r->max_states))
		goto cleanup;

	if (flags & PFSYNC_SI_IOCTL)
		pool_flags = PR_WAITOK | PR_LIMITFAIL | PR_ZERO;
	else
		pool_flags = PR_LIMITFAIL | PR_ZERO;

	if ((st = pool_get(&pf_state_pl, pool_flags)) == NULL)
		goto cleanup;

	if ((skw = pf_alloc_state_key(pool_flags)) == NULL)
		goto cleanup;

	if (PF_ANEQ(&sp->key[PF_SK_WIRE].addr[0],
	    &sp->key[PF_SK_STACK].addr[0], sp->af) ||
	    PF_ANEQ(&sp->key[PF_SK_WIRE].addr[1],
	    &sp->key[PF_SK_STACK].addr[1], sp->af) ||
	    sp->key[PF_SK_WIRE].port[0] != sp->key[PF_SK_STACK].port[0] ||
	    sp->key[PF_SK_WIRE].port[1] != sp->key[PF_SK_STACK].port[1]) {
		if ((sks = pf_alloc_state_key(pool_flags)) == NULL)
			goto cleanup;
	} else
		sks = skw;

	/* allocate memory for scrub info */
	if (pfsync_alloc_scrub_memory(&sp->src, &st->src) ||
	    pfsync_alloc_scrub_memory(&sp->dst, &st->dst))
		goto cleanup;

	/* copy to state key(s) */
	skw->addr[0] = sp->key[PF_SK_WIRE].addr[0];
	skw->addr[1] = sp->key[PF_SK_WIRE].addr[1];
	skw->port[0] = sp->key[PF_SK_WIRE].port[0];
	skw->port[1] = sp->key[PF_SK_WIRE].port[1];
	skw->proto = sp->proto;
	skw->af = sp->af;
	if (sks != skw) {
		sks->addr[0] = sp->key[PF_SK_STACK].addr[0];
		sks->addr[1] = sp->key[PF_SK_STACK].addr[1];
		sks->port[0] = sp->key[PF_SK_STACK].port[0];
		sks->port[1] = sp->key[PF_SK_STACK].port[1];
		sks->proto = sp->proto;
		sks->af = sp->af;
	}

	/* copy to state */
	bcopy(&sp->rt_addr, &st->rt_addr, sizeof(st->rt_addr));
	st->creation = time_second - ntohl(sp->creation);
	st->expire = time_second;
	if (sp->expire) {
		/* XXX No adaptive scaling. */
		st->expire -= r->timeout[sp->timeout] - ntohl(sp->expire);
	}

	st->expire = ntohl(sp->expire) + time_second;
	st->direction = sp->direction;
	st->log = sp->log;
	st->timeout = sp->timeout;
	st->state_flags = sp->state_flags;

	bcopy(sp->id, &st->id, sizeof(st->id));
	st->creatorid = sp->creatorid;
	pf_state_peer_ntoh(&sp->src, &st->src);
	pf_state_peer_ntoh(&sp->dst, &st->dst);

	st->rule.ptr = r;
	st->nat_rule.ptr = NULL;
	st->anchor.ptr = NULL;
	st->rt_kif = NULL;

	st->pfsync_time = time_uptime;
	st->sync_state = PFSYNC_S_NONE;

	/* XXX when we have nat_rule/anchors, use STATE_INC_COUNTERS */
	r->states_cur++;
	r->states_tot++;

	if (!ISSET(flags, PFSYNC_SI_IOCTL))
		SET(st->state_flags, PFSTATE_NOSYNC);

	if ((error = pf_state_insert(kif, skw, sks, st)) != 0) {
		/* XXX when we have nat_rule/anchors, use STATE_DEC_COUNTERS */
		r->states_cur--;
		goto cleanup_state;
	}

	if (!ISSET(flags, PFSYNC_SI_IOCTL)) {
		CLR(st->state_flags, PFSTATE_NOSYNC);
		if (ISSET(st->state_flags, PFSTATE_ACK)) {
			pfsync_q_ins(st, PFSYNC_S_IACK);
			schednetisr(NETISR_PFSYNC);
		}
	}
	CLR(st->state_flags, PFSTATE_ACK);

	return (0);

 cleanup:
	error = ENOMEM;
	if (skw == sks)
		sks = NULL;
	if (skw != NULL)
		pool_put(&pf_state_key_pl, skw);
	if (sks != NULL)
		pool_put(&pf_state_key_pl, sks);

 cleanup_state:	/* pf_state_insert frees the state keys */
	if (st) {
		if (st->dst.scrub)
			pool_put(&pf_state_scrub_pl, st->dst.scrub);
		if (st->src.scrub)
			pool_put(&pf_state_scrub_pl, st->src.scrub);
		pool_put(&pf_state_pl, st);
	}
	return (error);
}

void
pfsync_input(struct mbuf *m, ...)
{
	struct pfsync_softc *sc = pfsyncif;
	struct pfsync_pkt pkt;
	struct ip *ip = mtod(m, struct ip *);
	struct pfsync_header *ph;
	struct pfsync_subheader subh;

	int offset, len;
	int rv;

	pfsyncstats.pfsyncs_ipackets++;

	/* verify that we have a sync interface configured */
	if (sc == NULL || !ISSET(sc->sc_if.if_flags, IFF_RUNNING) ||
	    sc->sc_sync_if == NULL || !pf_status.running)
		goto done;

	/* verify that the packet came in on the right interface */
	if (sc->sc_sync_if != m->m_pkthdr.rcvif) {
		pfsyncstats.pfsyncs_badif++;
		goto done;
	}

	sc->sc_if.if_ipackets++;
	sc->sc_if.if_ibytes += m->m_pkthdr.len;

	/* verify that the IP TTL is 255. */
	if (ip->ip_ttl != PFSYNC_DFLTTL) {
		pfsyncstats.pfsyncs_badttl++;
		goto done;
	}

	offset = ip->ip_hl << 2;
	if (m->m_pkthdr.len < offset + sizeof(*ph)) {
		pfsyncstats.pfsyncs_hdrops++;
		goto done;
	}

	if (offset + sizeof(*ph) > m->m_len) {
		if (m_pullup(m, offset + sizeof(*ph)) == NULL) {
			pfsyncstats.pfsyncs_hdrops++;
			return;
		}
		ip = mtod(m, struct ip *);
	}
	ph = (struct pfsync_header *)((char *)ip + offset);

	/* verify the version */
	if (ph->version != PFSYNC_VERSION) {
		pfsyncstats.pfsyncs_badver++;
		goto done;
	}
	len = ntohs(ph->len) + offset;
	if (m->m_pkthdr.len < len) {
		pfsyncstats.pfsyncs_badlen++;
		goto done;
	}

	/* Cheaper to grab this now than having to mess with mbufs later */
	pkt.ip = ip;
	pkt.src = ip->ip_src;
	pkt.flags = 0;

	if (!bcmp(&ph->pfcksum, &pf_status.pf_chksum, PF_MD5_DIGEST_LENGTH))
		pkt.flags |= PFSYNC_SI_CKSUM;

	offset += sizeof(*ph);
	while (offset <= len - sizeof(subh)) {
		m_copydata(m, offset, sizeof(subh), (caddr_t)&subh);
		offset += sizeof(subh);

		if (subh.action >= PFSYNC_ACT_MAX) {
			pfsyncstats.pfsyncs_badact++;
			goto done;
		}

		rv = (*pfsync_acts[subh.action])(&pkt, m, offset,
		    ntohs(subh.count));
		if (rv == -1)
			return;

		offset += rv;
	}

done:
	m_freem(m);
}

int
pfsync_in_clr(struct pfsync_pkt *pkt, struct mbuf *m, int offset, int count)
{
	struct pfsync_clr *clr;
	struct mbuf *mp;
	int len = sizeof(*clr) * count;
	int i, offp;

	struct pf_state *st, *nexts;
	struct pf_state_key *sk, *nextsk;
	struct pf_state_item *si;
	u_int32_t creatorid;
	int s;

	mp = m_pulldown(m, offset, len, &offp);
	if (mp == NULL) {
		pfsyncstats.pfsyncs_badlen++;
		return (-1);
	}
	clr = (struct pfsync_clr *)(mp->m_data + offp);

	s = splsoftnet();
	for (i = 0; i < count; i++) {
		creatorid = clr[i].creatorid;

		if (clr[i].ifname[0] == '\0') {
			for (st = RB_MIN(pf_state_tree_id, &tree_id);
			    st; st = nexts) {
				nexts = RB_NEXT(pf_state_tree_id, &tree_id, st);
				if (st->creatorid == creatorid) {
					SET(st->state_flags, PFSTATE_NOSYNC);
					pf_unlink_state(st);
				}
			}
		} else {
			if (pfi_kif_get(clr[i].ifname) == NULL)
				continue;

			/* XXX correct? */
			for (sk = RB_MIN(pf_state_tree, &pf_statetbl);
			    sk; sk = nextsk) {
				nextsk = RB_NEXT(pf_state_tree,
				    &pf_statetbl, sk);
				TAILQ_FOREACH(si, &sk->states, entry) {
					if (si->s->creatorid == creatorid) {
						SET(si->s->state_flags,
						    PFSTATE_NOSYNC);
						pf_unlink_state(si->s);
					}
				}
			}
		}
	}
	splx(s);

	return (len);
}

int
pfsync_in_ins(struct pfsync_pkt *pkt, struct mbuf *m, int offset, int count)
{
	struct mbuf *mp;
	struct pfsync_state *sa, *sp;
	int len = sizeof(*sp) * count;
	int i, offp;

	int s;

	mp = m_pulldown(m, offset, len, &offp);
	if (mp == NULL) {
		pfsyncstats.pfsyncs_badlen++;
		return (-1);
	}
	sa = (struct pfsync_state *)(mp->m_data + offp);

	s = splsoftnet();
	for (i = 0; i < count; i++) {
		sp = &sa[i];

		/* check for invalid values */
		if (sp->timeout >= PFTM_MAX ||
		    sp->src.state > PF_TCPS_PROXY_DST ||
		    sp->dst.state > PF_TCPS_PROXY_DST ||
		    sp->direction > PF_OUT ||
		    (sp->af != AF_INET && sp->af != AF_INET6)) {
			if (pf_status.debug >= PF_DEBUG_MISC) {
				printf("pfsync_input: PFSYNC5_ACT_INS: "
				    "invalid value\n");
			}
			pfsyncstats.pfsyncs_badval++;
			continue;
		}

		if (pfsync_state_import(sp, pkt->flags) == ENOMEM) {
			/* drop out, but process the rest of the actions */
			break;
		}
	}
	splx(s);

	return (len);
}

int
pfsync_in_iack(struct pfsync_pkt *pkt, struct mbuf *m, int offset, int count)
{
	struct pfsync_ins_ack *ia, *iaa;
	struct pf_state_cmp id_key;
	struct pf_state *st;

	struct mbuf *mp;
	int len = count * sizeof(*ia);
	int offp, i;
	int s;

	mp = m_pulldown(m, offset, len, &offp);
	if (mp == NULL) {
		pfsyncstats.pfsyncs_badlen++;
		return (-1);
	}
	iaa = (struct pfsync_ins_ack *)(mp->m_data + offp);

	s = splsoftnet();
	for (i = 0; i < count; i++) {
		ia = &iaa[i];

		bcopy(&ia->id, &id_key.id, sizeof(id_key.id));
		id_key.creatorid = ia->creatorid;

		st = pf_find_state_byid(&id_key);
		if (st == NULL)
			continue;

		if (ISSET(st->state_flags, PFSTATE_ACK))
			pfsync_deferred(st, 0);
	}
	splx(s);

	return (len);
}

int
pfsync_upd_tcp(struct pf_state *st, struct pfsync_state_peer *src,
    struct pfsync_state_peer *dst)
{
	int sfail = 0;

	/*
	 * The state should never go backwards except
	 * for syn-proxy states.  Neither should the
	 * sequence window slide backwards.
	 */
	if (st->src.state > src->state &&
	    (st->src.state < PF_TCPS_PROXY_SRC ||
	    src->state >= PF_TCPS_PROXY_SRC))
		sfail = 1;
	else if (SEQ_GT(st->src.seqlo, ntohl(src->seqlo)))
		sfail = 3;
	else if (st->dst.state > dst->state) {
		/* There might still be useful
		 * information about the src state here,
		 * so import that part of the update,
		 * then "fail" so we send the updated
		 * state back to the peer who is missing
		 * our what we know. */
		pf_state_peer_ntoh(src, &st->src);
		/* XXX do anything with timeouts? */
		sfail = 7;
	} else if (st->dst.state >= TCPS_SYN_SENT &&
	    SEQ_GT(st->dst.seqlo, ntohl(dst->seqlo)))
		sfail = 4;

	return (sfail);
}

int
pfsync_in_upd(struct pfsync_pkt *pkt, struct mbuf *m, int offset, int count)
{
	struct pfsync_state *sa, *sp;
	struct pf_state_cmp id_key;
	struct pf_state_key *sk;
	struct pf_state *st;
	int sfail;

	struct mbuf *mp;
	int len = count * sizeof(*sp);
	int offp, i;
	int s;

	mp = m_pulldown(m, offset, len, &offp);
	if (mp == NULL) {
		pfsyncstats.pfsyncs_badlen++;
		return (-1);
	}
	sa = (struct pfsync_state *)(mp->m_data + offp);

	s = splsoftnet();
	for (i = 0; i < count; i++) {
		sp = &sa[i];

		/* check for invalid values */
		if (sp->timeout >= PFTM_MAX ||
		    sp->src.state > PF_TCPS_PROXY_DST ||
		    sp->dst.state > PF_TCPS_PROXY_DST) {
			if (pf_status.debug >= PF_DEBUG_MISC) {
				printf("pfsync_input: PFSYNC_ACT_UPD: "
				    "invalid value\n");
			}
			pfsyncstats.pfsyncs_badval++;
			continue;
		}

		bcopy(sp->id, &id_key.id, sizeof(id_key.id));
		id_key.creatorid = sp->creatorid;

		st = pf_find_state_byid(&id_key);
		if (st == NULL) {
			/* insert the update */
			if (pfsync_state_import(sp, 0))
				pfsyncstats.pfsyncs_badstate++;
			continue;
		}

		if (ISSET(st->state_flags, PFSTATE_ACK))
			pfsync_deferred(st, 1);

		sk = st->key[PF_SK_WIRE];	/* XXX right one? */
		sfail = 0;
		if (sk->proto == IPPROTO_TCP)
			sfail = pfsync_upd_tcp(st, &sp->src, &sp->dst);
		else {
			/*
			 * Non-TCP protocol state machine always go
			 * forwards
			 */
			if (st->src.state > sp->src.state)
				sfail = 5;
			else if (st->dst.state > sp->dst.state)
				sfail = 6;
		}

		if (sfail) {
			if (pf_status.debug >= PF_DEBUG_NOISY) {
				printf("pfsync: %s stale update (%d)"
				    " id: %016llx creatorid: %08x\n",
				    (sfail < 7 ?  "ignoring" : "partial"),
				    sfail, betoh64(st->id),
				    ntohl(st->creatorid));
			}
			pfsyncstats.pfsyncs_stale++;

			pfsync_update_state(st);
			schednetisr(NETISR_PFSYNC);
			continue;
		}
		pfsync_alloc_scrub_memory(&sp->dst, &st->dst);
		pf_state_peer_ntoh(&sp->src, &st->src);
		pf_state_peer_ntoh(&sp->dst, &st->dst);
		st->expire = ntohl(sp->expire) + time_second;
		st->timeout = sp->timeout;
		st->pfsync_time = time_uptime;
	}
	splx(s);

	return (len);
}

int
pfsync_in_upd_c(struct pfsync_pkt *pkt, struct mbuf *m, int offset, int count)
{
	struct pfsync_upd_c *ua, *up;
	struct pf_state_key *sk;
	struct pf_state_cmp id_key;
	struct pf_state *st;

	int len = count * sizeof(*up);
	int sfail;

	struct mbuf *mp;
	int offp, i;
	int s;

	mp = m_pulldown(m, offset, len, &offp);
	if (mp == NULL) {
		pfsyncstats.pfsyncs_badlen++;
		return (-1);
	}
	ua = (struct pfsync_upd_c *)(mp->m_data + offp);

	s = splsoftnet();
	for (i = 0; i < count; i++) {
		up = &ua[i];

		/* check for invalid values */
		if (up->timeout >= PFTM_MAX ||
		    up->src.state > PF_TCPS_PROXY_DST ||
		    up->dst.state > PF_TCPS_PROXY_DST) {
			if (pf_status.debug >= PF_DEBUG_MISC) {
				printf("pfsync_input: "
				    "PFSYNC_ACT_UPD_C: "
				    "invalid value\n");
			}
			pfsyncstats.pfsyncs_badval++;
			continue;
		}

		bcopy(&up->id, &id_key.id, sizeof(id_key.id));
		id_key.creatorid = up->creatorid;

		st = pf_find_state_byid(&id_key);
		if (st == NULL) {
			/* We don't have this state. Ask for it. */
			pfsync_request_update(id_key.creatorid, id_key.id);
			continue;
		}

		if (ISSET(st->state_flags, PFSTATE_ACK))
			pfsync_deferred(st, 1);

		sk = st->key[PF_SK_WIRE]; /* XXX right one? */
		sfail = 0;
		if (sk->proto == IPPROTO_TCP)
			sfail = pfsync_upd_tcp(st, &up->src, &up->dst);
		else {
			/*
			 * Non-TCP protocol state machine always go forwards
			 */
			if (st->src.state > up->src.state)
				sfail = 5;
			else if (st->dst.state > up->dst.state)
				sfail = 6;
		}

		if (sfail) {
			if (pf_status.debug >= PF_DEBUG_NOISY) {
				printf("pfsync: ignoring stale update "
				    "(%d) id: %016llx "
				    "creatorid: %08x\n", sfail,
				    betoh64(st->id),
				    ntohl(st->creatorid));
			}
			pfsyncstats.pfsyncs_stale++;

			pfsync_update_state(st);
			schednetisr(NETISR_PFSYNC);
			continue;
		}
		pfsync_alloc_scrub_memory(&up->dst, &st->dst);
		pf_state_peer_ntoh(&up->src, &st->src);
		pf_state_peer_ntoh(&up->dst, &st->dst);
		st->expire = ntohl(up->expire) + time_second;
		st->timeout = up->timeout;
		st->pfsync_time = time_uptime;
	}
	splx(s);

	return (len);
}

int
pfsync_in_ureq(struct pfsync_pkt *pkt, struct mbuf *m, int offset, int count)
{
	struct pfsync_upd_req *ur, *ura;
	struct mbuf *mp;
	int len = count * sizeof(*ur);
	int i, offp;

	struct pf_state_cmp id_key;
	struct pf_state *st;

	mp = m_pulldown(m, offset, len, &offp);
	if (mp == NULL) {
		pfsyncstats.pfsyncs_badlen++;
		return (-1);
	}
	ura = (struct pfsync_upd_req *)(mp->m_data + offp);

	for (i = 0; i < count; i++) {
		ur = &ura[i];

		bcopy(&ur->id, &id_key.id, sizeof(id_key.id));
		id_key.creatorid = ur->creatorid;

		if (id_key.id == 0 && id_key.creatorid == 0)
			pfsync_bulk_start();
		else {
			st = pf_find_state_byid(&id_key);
			if (st == NULL) {
				pfsyncstats.pfsyncs_badstate++;
				continue;
			}
			if (ISSET(st->state_flags, PFSTATE_NOSYNC))
				continue;

			pfsync_update_state_req(st);
		}
	}

	return (len);
}

int
pfsync_in_del(struct pfsync_pkt *pkt, struct mbuf *m, int offset, int count)
{
	struct mbuf *mp;
	struct pfsync_state *sa, *sp;
	struct pf_state_cmp id_key;
	struct pf_state *st;
	int len = count * sizeof(*sp);
	int offp, i;
	int s;

	mp = m_pulldown(m, offset, len, &offp);
	if (mp == NULL) {
		pfsyncstats.pfsyncs_badlen++;
		return (-1);
	}
	sa = (struct pfsync_state *)(mp->m_data + offp);

	s = splsoftnet();
	for (i = 0; i < count; i++) {
		sp = &sa[i];

		bcopy(sp->id, &id_key.id, sizeof(id_key.id));
		id_key.creatorid = sp->creatorid;

		st = pf_find_state_byid(&id_key);
		if (st == NULL) {
			pfsyncstats.pfsyncs_badstate++;
			continue;
		}
		SET(st->state_flags, PFSTATE_NOSYNC);
		pf_unlink_state(st);
	}
	splx(s);

	return (len);
}

int
pfsync_in_del_c(struct pfsync_pkt *pkt, struct mbuf *m, int offset, int count)
{
	struct mbuf *mp;
	struct pfsync_del_c *sa, *sp;
	struct pf_state_cmp id_key;
	struct pf_state *st;
	int len = count * sizeof(*sp);
	int offp, i;
	int s;

	mp = m_pulldown(m, offset, len, &offp);
	if (mp == NULL) {
		pfsyncstats.pfsyncs_badlen++;
		return (-1);
	}
	sa = (struct pfsync_del_c *)(mp->m_data + offp);

	s = splsoftnet();
	for (i = 0; i < count; i++) {
		sp = &sa[i];

		bcopy(&sp->id, &id_key.id, sizeof(id_key.id));
		id_key.creatorid = sp->creatorid;

		st = pf_find_state_byid(&id_key);
		if (st == NULL) {
			pfsyncstats.pfsyncs_badstate++;
			continue;
		}

		SET(st->state_flags, PFSTATE_NOSYNC);
		pf_unlink_state(st);
	}
	splx(s);

	return (len);
}

int
pfsync_in_bus(struct pfsync_pkt *pkt, struct mbuf *m, int offset, int count)
{
	struct pfsync_softc *sc = pfsyncif;
	struct pfsync_bus *bus;
	struct mbuf *mp;
	int len = count * sizeof(*bus);
	int offp;

	/* If we're not waiting for a bulk update, who cares. */
	if (sc->sc_ureq_sent == 0)
		return (len);

	mp = m_pulldown(m, offset, len, &offp);
	if (mp == NULL) {
		pfsyncstats.pfsyncs_badlen++;
		return (-1);
	}
	bus = (struct pfsync_bus *)(mp->m_data + offp);

	switch (bus->status) {
	case PFSYNC_BUS_START:
		timeout_add(&sc->sc_bulkfail_tmo, 4 * hz +
		    pf_pool_limits[PF_LIMIT_STATES].limit /
		    ((sc->sc_if.if_mtu - PFSYNC_MINPKT) /
		    sizeof(struct pfsync_state)));
		if (pf_status.debug >= PF_DEBUG_MISC)
			printf("pfsync: received bulk update start\n");
		break;

	case PFSYNC_BUS_END:
		if (time_uptime - ntohl(bus->endtime) >=
		    sc->sc_ureq_sent) {
			/* that's it, we're happy */
			sc->sc_ureq_sent = 0;
			sc->sc_bulk_tries = 0;
			timeout_del(&sc->sc_bulkfail_tmo);
#if NCARP > 0
			if (!pfsync_sync_ok)
				carp_group_demote_adj(&sc->sc_if, -1);
#endif
			pfsync_sync_ok = 1;
			if (pf_status.debug >= PF_DEBUG_MISC)
				printf("pfsync: received valid "
				    "bulk update end\n");
		} else {
			if (pf_status.debug >= PF_DEBUG_MISC)
				printf("pfsync: received invalid "
				    "bulk update end: bad timestamp\n");
		}
		break;
	}

	return (len);
}

int
pfsync_in_tdb(struct pfsync_pkt *pkt, struct mbuf *m, int offset, int count)
{
	int len = count * sizeof(struct pfsync_tdb);

#if defined(IPSEC)
	struct pfsync_tdb *tp;
	struct mbuf *mp;
	int offp;
	int i;
	int s;

	mp = m_pulldown(m, offset, len, &offp);
	if (mp == NULL) {
		pfsyncstats.pfsyncs_badlen++;
		return (-1);
	}
	tp = (struct pfsync_tdb *)(mp->m_data + offp);

	s = splsoftnet();
	for (i = 0; i < count; i++)
		pfsync_update_net_tdb(&tp[i]);
	splx(s);
#endif

	return (len);
}

#if defined(IPSEC)
/* Update an in-kernel tdb. Silently fail if no tdb is found. */
void
pfsync_update_net_tdb(struct pfsync_tdb *pt)
{
	struct tdb		*tdb;
	int			 s;

	/* check for invalid values */
	if (ntohl(pt->spi) <= SPI_RESERVED_MAX ||
	    (pt->dst.sa.sa_family != AF_INET &&
	     pt->dst.sa.sa_family != AF_INET6))
		goto bad;

	s = spltdb();
	tdb = gettdb(pt->spi, &pt->dst, pt->sproto);
	if (tdb) {
		pt->rpl = ntohl(pt->rpl);
		pt->cur_bytes = betoh64(pt->cur_bytes);

		/* Neither replay nor byte counter should ever decrease. */
		if (pt->rpl < tdb->tdb_rpl ||
		    pt->cur_bytes < tdb->tdb_cur_bytes) {
			splx(s);
			goto bad;
		}

		tdb->tdb_rpl = pt->rpl;
		tdb->tdb_cur_bytes = pt->cur_bytes;
	}
	splx(s);
	return;

 bad:
	if (pf_status.debug >= PF_DEBUG_MISC)
		printf("pfsync_insert: PFSYNC_ACT_TDB_UPD: "
		    "invalid value\n");
	pfsyncstats.pfsyncs_badstate++;
	return;
}
#endif


int
pfsync_in_eof(struct pfsync_pkt *pkt, struct mbuf *m, int offset, int count)
{
	/* check if we are at the right place in the packet */
	if (offset != m->m_pkthdr.len)
		pfsyncstats.pfsyncs_badlen++;

	/* we're done. free and let the caller return */
	m_freem(m);
	return (-1);
}

int
pfsync_in_error(struct pfsync_pkt *pkt, struct mbuf *m, int offset, int count)
{
	pfsyncstats.pfsyncs_badact++;

	m_freem(m);
	return (-1);
}

int
pfsyncoutput(struct ifnet *ifp, struct mbuf *m, struct sockaddr *dst,
	struct rtentry *rt)
{
	m_freem(m);
	return (0);
}

/* ARGSUSED */
int
pfsyncioctl(struct ifnet *ifp, u_long cmd, caddr_t data)
{
	struct proc *p = curproc;
	struct pfsync_softc *sc = ifp->if_softc;
	struct ifreq *ifr = (struct ifreq *)data;
	struct ip_moptions *imo = &sc->sc_imo;
	struct pfsyncreq pfsyncr;
	struct ifnet    *sifp;
	struct ip *ip;
	int s, error;

	switch (cmd) {
#if 0
	case SIOCSIFADDR:
	case SIOCAIFADDR:
	case SIOCSIFDSTADDR:
#endif
	case SIOCSIFFLAGS:
		s = splnet();
		if (ifp->if_flags & IFF_UP)
			ifp->if_flags |= IFF_RUNNING;
		else {
			ifp->if_flags &= ~IFF_RUNNING;

			/* drop everything */
			timeout_del(&sc->sc_tmo);
			pfsync_drop(sc);

			/* cancel bulk update */
			timeout_del(&sc->sc_bulk_tmo);
			sc->sc_bulk_next = NULL;
			sc->sc_bulk_last = NULL;
		}
		splx(s);
		break;
	case SIOCSIFMTU:
		s = splnet();
		if (ifr->ifr_mtu <= PFSYNC_MINPKT)
			return (EINVAL);
		if (ifr->ifr_mtu > MCLBYTES) /* XXX could be bigger */
			ifr->ifr_mtu = MCLBYTES;
		if (ifr->ifr_mtu < ifp->if_mtu)
			pfsync_sendout();
		ifp->if_mtu = ifr->ifr_mtu;
		splx(s);
		break;
	case SIOCGETPFSYNC:
		bzero(&pfsyncr, sizeof(pfsyncr));
		if (sc->sc_sync_if) {
			strlcpy(pfsyncr.pfsyncr_syncdev,
			    sc->sc_sync_if->if_xname, IFNAMSIZ);
		}
		pfsyncr.pfsyncr_syncpeer = sc->sc_sync_peer;
		pfsyncr.pfsyncr_maxupdates = sc->sc_maxupdates;
		return (copyout(&pfsyncr, ifr->ifr_data, sizeof(pfsyncr)));

	case SIOCSETPFSYNC:
		if ((error = suser(p, p->p_acflag)) != 0)
			return (error);
		if ((error = copyin(ifr->ifr_data, &pfsyncr, sizeof(pfsyncr))))
			return (error);

		s = splnet();

		if (pfsyncr.pfsyncr_syncpeer.s_addr == 0)
			sc->sc_sync_peer.s_addr = INADDR_PFSYNC_GROUP;
		else
			sc->sc_sync_peer.s_addr =
			    pfsyncr.pfsyncr_syncpeer.s_addr;

		if (pfsyncr.pfsyncr_maxupdates > 255) {
			splx(s);
			return (EINVAL);
		}
		sc->sc_maxupdates = pfsyncr.pfsyncr_maxupdates;

		if (pfsyncr.pfsyncr_syncdev[0] == 0) {
			sc->sc_sync_if = NULL;
			if (imo->imo_num_memberships > 0) {
				in_delmulti(imo->imo_membership[
				    --imo->imo_num_memberships]);
				imo->imo_multicast_ifp = NULL;
			}
			splx(s);
			break;
		}

		if ((sifp = ifunit(pfsyncr.pfsyncr_syncdev)) == NULL) {
			splx(s);
			return (EINVAL);
		}

		if (sifp->if_mtu < sc->sc_if.if_mtu ||
		    (sc->sc_sync_if != NULL &&
		    sifp->if_mtu < sc->sc_sync_if->if_mtu) ||
		    sifp->if_mtu < MCLBYTES - sizeof(struct ip))
			pfsync_sendout();
		sc->sc_sync_if = sifp;

		if (imo->imo_num_memberships > 0) {
			in_delmulti(imo->imo_membership[--imo->imo_num_memberships]);
			imo->imo_multicast_ifp = NULL;
		}

		if (sc->sc_sync_if &&
		    sc->sc_sync_peer.s_addr == INADDR_PFSYNC_GROUP) {
			struct in_addr addr;

			if (!(sc->sc_sync_if->if_flags & IFF_MULTICAST)) {
				sc->sc_sync_if = NULL;
				splx(s);
				return (EADDRNOTAVAIL);
			}

			addr.s_addr = INADDR_PFSYNC_GROUP;

			if ((imo->imo_membership[0] =
			    in_addmulti(&addr, sc->sc_sync_if)) == NULL) {
				sc->sc_sync_if = NULL;
				splx(s);
				return (ENOBUFS);
			}
			imo->imo_num_memberships++;
			imo->imo_multicast_ifp = sc->sc_sync_if;
			imo->imo_multicast_ttl = PFSYNC_DFLTTL;
			imo->imo_multicast_loop = 0;
		}

		ip = &sc->sc_template;
		bzero(ip, sizeof(*ip));
		ip->ip_v = IPVERSION;
		ip->ip_hl = sizeof(sc->sc_template) >> 2;
		ip->ip_tos = IPTOS_LOWDELAY;
		/* len and id are set later */
		ip->ip_off = htons(IP_DF);
		ip->ip_ttl = PFSYNC_DFLTTL;
		ip->ip_p = IPPROTO_PFSYNC;
		ip->ip_src.s_addr = INADDR_ANY;
		ip->ip_dst.s_addr = sc->sc_sync_peer.s_addr;

		if (sc->sc_sync_if) {
			/* Request a full state table update. */
			sc->sc_ureq_sent = time_uptime;
#if NCARP > 0
			if (pfsync_sync_ok)
				carp_group_demote_adj(&sc->sc_if, 1);
#endif
			pfsync_sync_ok = 0;
			if (pf_status.debug >= PF_DEBUG_MISC)
				printf("pfsync: requesting bulk update\n");
			timeout_add(&sc->sc_bulkfail_tmo, 4 * hz +
			    pf_pool_limits[PF_LIMIT_STATES].limit /
			    ((sc->sc_if.if_mtu - PFSYNC_MINPKT) /
			    sizeof(struct pfsync_state)));
			pfsync_request_update(0, 0);
		}
		splx(s);

		break;

	default:
		return (ENOTTY);
	}

	return (0);
}

void
pfsync_out_state(struct pf_state *st, void *buf)
{
	struct pfsync_state *sp = buf;

	pfsync_state_export(sp, st);
}

void
pfsync_out_iack(struct pf_state *st, void *buf)
{
	struct pfsync_ins_ack *iack = buf;

	iack->id = st->id;
	iack->creatorid = st->creatorid;
}

void
pfsync_out_upd_c(struct pf_state *st, void *buf)
{
	struct pfsync_upd_c *up = buf;

	up->id = st->id;
	pf_state_peer_hton(&st->src, &up->src);
	pf_state_peer_hton(&st->dst, &up->dst);
	up->creatorid = st->creatorid;

	up->expire = pf_state_expires(st);
	if (up->expire <= time_second)
		up->expire = htonl(0);
	else
		up->expire = htonl(up->expire - time_second);
	up->timeout = st->timeout;

	bzero(up->_pad, sizeof(up->_pad)); /* XXX */
}

void
pfsync_out_del(struct pf_state *st, void *buf)
{
	struct pfsync_del_c *dp = buf;

	dp->id = st->id;
	dp->creatorid = st->creatorid;

	SET(st->state_flags, PFSTATE_NOSYNC);
}

void
pfsync_drop(struct pfsync_softc *sc)
{
	struct pf_state *st;
	struct pfsync_upd_req_item *ur;
	struct tdb *t;
	int q;

	for (q = 0; q < PFSYNC_S_COUNT; q++) {
		if (TAILQ_EMPTY(&sc->sc_qs[q]))
			continue;

		TAILQ_FOREACH(st, &sc->sc_qs[q], sync_list) {
#ifdef PFSYNC_DEBUG
			KASSERT(st->sync_state == q);
#endif
			st->sync_state = PFSYNC_S_NONE;
		}
		TAILQ_INIT(&sc->sc_qs[q]);
	}

	while ((ur = TAILQ_FIRST(&sc->sc_upd_req_list)) != NULL) {
		TAILQ_REMOVE(&sc->sc_upd_req_list, ur, ur_entry);
		pool_put(&sc->sc_pool, ur);
	}

	sc->sc_plus = NULL;

	if (!TAILQ_EMPTY(&sc->sc_tdb_q)) {
		TAILQ_FOREACH(t, &sc->sc_tdb_q, tdb_sync_entry)
			CLR(t->tdb_flags, TDBF_PFSYNC);

		TAILQ_INIT(&sc->sc_tdb_q);
	}

	sc->sc_len = PFSYNC_MINPKT;
}

void
pfsync_sendout(void)
{
	struct pfsync_softc *sc = pfsyncif;
#if NBPFILTER > 0
	struct ifnet *ifp = &sc->sc_if;
#endif
	struct mbuf *m;
	struct ip *ip;
	struct pfsync_header *ph;
	struct pfsync_subheader *subh;
	struct pf_state *st;
	struct pfsync_upd_req_item *ur;
	struct tdb *t;

	int offset;
	int q, count = 0;

	if (sc == NULL || sc->sc_len == PFSYNC_MINPKT)
		return;

	if (!ISSET(sc->sc_if.if_flags, IFF_RUNNING) ||
#if NBPFILTER > 0
	    (ifp->if_bpf == NULL && sc->sc_sync_if == NULL)) {
#else
	    sc->sc_sync_if == NULL) {
#endif
		pfsync_drop(sc);
		return;
	}

	MGETHDR(m, M_DONTWAIT, MT_DATA);
	if (m == NULL) {
		sc->sc_if.if_oerrors++;
		pfsyncstats.pfsyncs_onomem++;
		pfsync_drop(sc);
		return;
	}

	if (max_linkhdr + sc->sc_len > MHLEN) {
		MCLGETI(m, M_DONTWAIT, NULL, max_linkhdr + sc->sc_len);
		if (!ISSET(m->m_flags, M_EXT)) {
			m_free(m);
			sc->sc_if.if_oerrors++;
			pfsyncstats.pfsyncs_onomem++;
			pfsync_drop(sc);
			return;
		}
	}
	m->m_data += max_linkhdr;
	m->m_len = m->m_pkthdr.len = sc->sc_len;

	/* build the ip header */
	ip = (struct ip *)m->m_data;
	bcopy(&sc->sc_template, ip, sizeof(*ip));
	offset = sizeof(*ip);

	ip->ip_len = htons(m->m_pkthdr.len);
	ip->ip_id = htons(ip_randomid());

	/* build the pfsync header */
	ph = (struct pfsync_header *)(m->m_data + offset);
	bzero(ph, sizeof(*ph));
	offset += sizeof(*ph);

	ph->version = PFSYNC_VERSION;
	ph->len = htons(sc->sc_len - sizeof(*ip));
	bcopy(pf_status.pf_chksum, ph->pfcksum, PF_MD5_DIGEST_LENGTH);

	/* walk the queues */
	for (q = 0; q < PFSYNC_S_COUNT; q++) {
		if (TAILQ_EMPTY(&sc->sc_qs[q]))
			continue;

		subh = (struct pfsync_subheader *)(m->m_data + offset);
		offset += sizeof(*subh);

		count = 0;
		TAILQ_FOREACH(st, &sc->sc_qs[q], sync_list) {
#ifdef PFSYNC_DEBUG
			KASSERT(st->sync_state == q);
#endif
			pfsync_qs[q].write(st, m->m_data + offset);
			offset += pfsync_qs[q].len;

			st->sync_state = PFSYNC_S_NONE;
			count++;
		}
		TAILQ_INIT(&sc->sc_qs[q]);

		bzero(subh, sizeof(*subh));
		subh->action = pfsync_qs[q].action;
		subh->count = htons(count);
	}

	if (!TAILQ_EMPTY(&sc->sc_upd_req_list)) {
		subh = (struct pfsync_subheader *)(m->m_data + offset);
		offset += sizeof(*subh);

		count = 0;
		while ((ur = TAILQ_FIRST(&sc->sc_upd_req_list)) != NULL) {
			TAILQ_REMOVE(&sc->sc_upd_req_list, ur, ur_entry);

			bcopy(&ur->ur_msg, m->m_data + offset,
			    sizeof(ur->ur_msg));
			offset += sizeof(ur->ur_msg);

			pool_put(&sc->sc_pool, ur);

			count++;
		}

		bzero(subh, sizeof(*subh));
		subh->action = PFSYNC_ACT_UPD_REQ;
		subh->count = htons(count);
	}

	/* has someone built a custom region for us to add? */
	if (sc->sc_plus != NULL) {
		bcopy(sc->sc_plus, m->m_data + offset, sc->sc_pluslen);
		offset += sc->sc_pluslen;

		sc->sc_plus = NULL;
	}

	if (!TAILQ_EMPTY(&sc->sc_tdb_q)) {
		subh = (struct pfsync_subheader *)(m->m_data + offset);
		offset += sizeof(*subh);

		count = 0;
		TAILQ_FOREACH(t, &sc->sc_tdb_q, tdb_sync_entry) {
			pfsync_out_tdb(t, m->m_data + offset);
			offset += sizeof(struct pfsync_tdb);
			CLR(t->tdb_flags, TDBF_PFSYNC);

			count++;
		}
		TAILQ_INIT(&sc->sc_tdb_q);

		bzero(subh, sizeof(*subh));
		subh->action = PFSYNC_ACT_TDB;
		subh->count = htons(count);
	}

	subh = (struct pfsync_subheader *)(m->m_data + offset);
	offset += sizeof(*subh);

	bzero(subh, sizeof(*subh));
	subh->action = PFSYNC_ACT_EOF;
	subh->count = htons(1);

	/* we're done, let's put it on the wire */
#if NBPFILTER > 0
	if (ifp->if_bpf) {
		m->m_data += sizeof(*ip);
		m->m_len = m->m_pkthdr.len = sc->sc_len - sizeof(*ip);
		bpf_mtap(ifp->if_bpf, m, BPF_DIRECTION_OUT);
		m->m_data -= sizeof(*ip);
		m->m_len = m->m_pkthdr.len = sc->sc_len;
	}

	if (sc->sc_sync_if == NULL) {
		sc->sc_len = PFSYNC_MINPKT;
		m_freem(m);
		return;
	}
#endif

	sc->sc_if.if_opackets++;
	sc->sc_if.if_obytes += m->m_pkthdr.len;

	if (ip_output(m, NULL, NULL, IP_RAWOUTPUT, &sc->sc_imo, NULL) == 0)
		pfsyncstats.pfsyncs_opackets++;
	else
		pfsyncstats.pfsyncs_oerrors++;

	/* start again */
	sc->sc_len = PFSYNC_MINPKT;
}

void
pfsync_insert_state(struct pf_state *st)
{
	struct pfsync_softc *sc = pfsyncif;

	splsoftassert(IPL_SOFTNET);

	if (ISSET(st->rule.ptr->rule_flag, PFRULE_NOSYNC) ||
	    st->key[PF_SK_WIRE]->proto == IPPROTO_PFSYNC) {
		SET(st->state_flags, PFSTATE_NOSYNC);
		return;
	}

	if (sc == NULL || !ISSET(sc->sc_if.if_flags, IFF_RUNNING) ||
	    ISSET(st->state_flags, PFSTATE_NOSYNC))
		return;

#ifdef PFSYNC_DEBUG
	KASSERT(st->sync_state == PFSYNC_S_NONE);
#endif

	if (sc->sc_len == PFSYNC_MINPKT)
		timeout_add_sec(&sc->sc_tmo, 1);

	pfsync_q_ins(st, PFSYNC_S_INS);

	if (ISSET(st->state_flags, PFSTATE_ACK))
		schednetisr(NETISR_PFSYNC);
	else
		st->sync_updates = 0;
}

int defer = 10;

int
pfsync_defer(struct pf_state *st, struct mbuf *m)
{
	return (0);
#ifdef notyet
	struct pfsync_softc *sc = pfsyncif;
	struct pfsync_deferral *pd;

	splsoftassert(IPL_SOFTNET);

	if (sc->sc_deferred >= 128)
		pfsync_undefer(TAILQ_FIRST(&sc->sc_deferrals), 0);

	pd = pool_get(&sc->sc_pool, M_NOWAIT);
	if (pd == NULL)
		return (0);
	sc->sc_deferred++;

	m->m_pkthdr.pf.flags |= PF_TAG_GENERATED;
	SET(st->state_flags, PFSTATE_ACK);

	pd->pd_st = st;
	pd->pd_m = m;

	TAILQ_INSERT_TAIL(&sc->sc_deferrals, pd, pd_entry);
	timeout_set(&pd->pd_tmo, pfsync_defer_tmo, pd);
	timeout_add(&pd->pd_tmo, defer);

	return (1);
#endif
}

void
pfsync_undefer(struct pfsync_deferral *pd, int drop)
{
	struct pfsync_softc *sc = pfsyncif;

	splsoftassert(IPL_SOFTNET);

	TAILQ_REMOVE(&sc->sc_deferrals, pd, pd_entry);
	sc->sc_deferred--;

	CLR(pd->pd_st->state_flags, PFSTATE_ACK);
	timeout_del(&pd->pd_tmo); /* bah */
	if (drop)
		m_freem(pd->pd_m);
	else {
		ip_output(pd->pd_m, (void *)NULL, (void *)NULL, 0,
		    (void *)NULL, (void *)NULL);
	}

	pool_put(&sc->sc_pool, pd);
}

void
pfsync_defer_tmo(void *arg)
{
	int s;

	s = splsoftnet();
	pfsync_undefer(arg, 0);
	splx(s);
}

void
pfsync_deferred(struct pf_state *st, int drop)
{
	struct pfsync_softc *sc = pfsyncif;
	struct pfsync_deferral *pd;

	TAILQ_FOREACH(pd, &sc->sc_deferrals, pd_entry) {
		 if (pd->pd_st == st) {
			pfsync_undefer(pd, drop);
			return;
		}
	}

	panic("pfsync_send_deferred: unable to find deferred state");
}

u_int pfsync_upds = 0;

void
pfsync_update_state(struct pf_state *st)
{
	struct pfsync_softc *sc = pfsyncif;
	int sync = 0;

	splsoftassert(IPL_SOFTNET);

	if (sc == NULL || !ISSET(sc->sc_if.if_flags, IFF_RUNNING))
		return;

	if (ISSET(st->state_flags, PFSTATE_ACK))
		pfsync_deferred(st, 0);
	if (ISSET(st->state_flags, PFSTATE_NOSYNC)) {
		if (st->sync_state != PFSYNC_S_NONE)
			pfsync_q_del(st);
		return;
	}

	if (sc->sc_len == PFSYNC_MINPKT)
		timeout_add_sec(&sc->sc_tmo, 1);

	switch (st->sync_state) {
	case PFSYNC_S_UPD_C:
	case PFSYNC_S_UPD:
	case PFSYNC_S_INS:
		/* we're already handling it */

		if (st->key[PF_SK_WIRE]->proto == IPPROTO_TCP) {
			st->sync_updates++;
			if (st->sync_updates >= sc->sc_maxupdates)
				sync = 1;
		}
		break;

	case PFSYNC_S_IACK:
		pfsync_q_del(st);
	case PFSYNC_S_NONE:
		pfsync_q_ins(st, PFSYNC_S_UPD_C);
		st->sync_updates = 0;
		break;

	default:
		panic("pfsync_update_state: unexpected sync state %d",
		    st->sync_state);
	}

	if (sync || (time_uptime - st->pfsync_time) < 2) {
		pfsync_upds++;
		schednetisr(NETISR_PFSYNC);
	}
}

void
pfsync_request_update(u_int32_t creatorid, u_int64_t id)
{
	struct pfsync_softc *sc = pfsyncif;
	struct pfsync_upd_req_item *item;
	size_t nlen = sizeof(struct pfsync_upd_req);

	/*
	 * this code does nothing to prevent multiple update requests for the
	 * same state being generated.
	 */

	item = pool_get(&sc->sc_pool, PR_NOWAIT);
	if (item == NULL) {
		/* XXX stats */
		return;
	}

	item->ur_msg.id = id;
	item->ur_msg.creatorid = creatorid;

	if (TAILQ_EMPTY(&sc->sc_upd_req_list))
		nlen += sizeof(struct pfsync_subheader);

	if (sc->sc_len + nlen > sc->sc_if.if_mtu) {
		pfsync_sendout();

		nlen = sizeof(struct pfsync_subheader) +
		    sizeof(struct pfsync_upd_req);
	}

	TAILQ_INSERT_TAIL(&sc->sc_upd_req_list, item, ur_entry);
	sc->sc_len += nlen;

	schednetisr(NETISR_PFSYNC);
}

void
pfsync_update_state_req(struct pf_state *st)
{
	struct pfsync_softc *sc = pfsyncif;

	if (sc == NULL)
		panic("pfsync_update_state_req: nonexistant instance");

	if (ISSET(st->state_flags, PFSTATE_NOSYNC)) {
		if (st->sync_state != PFSYNC_S_NONE)
			pfsync_q_del(st);
		return;
	}

	switch (st->sync_state) {
	case PFSYNC_S_UPD_C:
	case PFSYNC_S_IACK:
		pfsync_q_del(st);
	case PFSYNC_S_NONE:
		pfsync_q_ins(st, PFSYNC_S_UPD);
		schednetisr(NETISR_PFSYNC);
		return;

	case PFSYNC_S_INS:
	case PFSYNC_S_UPD:
	case PFSYNC_S_DEL:
		/* we're already handling it */
		return;

	default:
		panic("pfsync_update_state_req: unexpected sync state %d",
		    st->sync_state);
	}
}

void
pfsync_delete_state(struct pf_state *st)
{
	struct pfsync_softc *sc = pfsyncif;

	splsoftassert(IPL_SOFTNET);

	if (sc == NULL || !ISSET(sc->sc_if.if_flags, IFF_RUNNING))
		return;

	if (ISSET(st->state_flags, PFSTATE_ACK))
		pfsync_deferred(st, 1);
	if (ISSET(st->state_flags, PFSTATE_NOSYNC)) {
		if (st->sync_state != PFSYNC_S_NONE)
			pfsync_q_del(st);
		return;
	}

	if (sc->sc_len == PFSYNC_MINPKT)
		timeout_add_sec(&sc->sc_tmo, 1);

	switch (st->sync_state) {
	case PFSYNC_S_INS:
		/* we never got to tell the world so just forget about it */
		pfsync_q_del(st);
		return;

	case PFSYNC_S_UPD_C:
	case PFSYNC_S_UPD:
	case PFSYNC_S_IACK:
		pfsync_q_del(st);
		/* FALLTHROUGH to putting it on the del list */

	case PFSYNC_S_NONE:
		pfsync_q_ins(st, PFSYNC_S_DEL);
		return;

	default:
		panic("pfsync_delete_state: unexpected sync state %d",
		    st->sync_state);
	}
}

void
pfsync_clear_states(u_int32_t creatorid, const char *ifname)
{
	struct pfsync_softc *sc = pfsyncif;
	struct {
		struct pfsync_subheader subh;
		struct pfsync_clr clr;
	} __packed r;

	splsoftassert(IPL_SOFTNET);

	if (sc == NULL || !ISSET(sc->sc_if.if_flags, IFF_RUNNING))
		return;

	bzero(&r, sizeof(r));

	r.subh.action = PFSYNC_ACT_CLR;
	r.subh.count = htons(1);

	strlcpy(r.clr.ifname, ifname, sizeof(r.clr.ifname));
	r.clr.creatorid = creatorid;

	pfsync_send_plus(&r, sizeof(r));
}

void
pfsync_q_ins(struct pf_state *st, int q)
{
	struct pfsync_softc *sc = pfsyncif;
	size_t nlen = pfsync_qs[q].len;

	KASSERT(st->sync_state == PFSYNC_S_NONE);

#if 1 || defined(PFSYNC_DEBUG)
	if (sc->sc_len < PFSYNC_MINPKT)
		panic("pfsync pkt len is too low %d", sc->sc_len);
#endif
	if (TAILQ_EMPTY(&sc->sc_qs[q]))
		nlen += sizeof(struct pfsync_subheader);

	if (sc->sc_len + nlen > sc->sc_if.if_mtu) {
		pfsync_sendout();

		nlen = sizeof(struct pfsync_subheader) + pfsync_qs[q].len;
	}

	sc->sc_len += nlen;
	TAILQ_INSERT_TAIL(&sc->sc_qs[q], st, sync_list);
	st->sync_state = q;
}

void
pfsync_q_del(struct pf_state *st)
{
	struct pfsync_softc *sc = pfsyncif;
	int q = st->sync_state;

	KASSERT(st->sync_state != PFSYNC_S_NONE);

	sc->sc_len -= pfsync_qs[q].len;
	TAILQ_REMOVE(&sc->sc_qs[q], st, sync_list);
	st->sync_state = PFSYNC_S_NONE;

	if (TAILQ_EMPTY(&sc->sc_qs[q]))
		sc->sc_len -= sizeof(struct pfsync_subheader);
}

void
pfsync_update_tdb(struct tdb *t, int output)
{
	struct pfsync_softc *sc = pfsyncif;
	size_t nlen = sizeof(struct pfsync_tdb);

	if (sc == NULL)
		return;

	if (!ISSET(t->tdb_flags, TDBF_PFSYNC)) {
		if (TAILQ_EMPTY(&sc->sc_tdb_q))
			nlen += sizeof(struct pfsync_subheader);

		if (sc->sc_len + nlen > sc->sc_if.if_mtu) {
			pfsync_sendout();

			nlen = sizeof(struct pfsync_subheader) +
			    sizeof(struct pfsync_tdb);
		}

		sc->sc_len += nlen;
		TAILQ_INSERT_TAIL(&sc->sc_tdb_q, t, tdb_sync_entry);
		SET(t->tdb_flags, TDBF_PFSYNC);
		t->tdb_updates = 0;
	} else {
		if (++t->tdb_updates >= sc->sc_maxupdates)
			schednetisr(NETISR_PFSYNC);
	}

	if (output)
		SET(t->tdb_flags, TDBF_PFSYNC_RPL);
	else
		CLR(t->tdb_flags, TDBF_PFSYNC_RPL);
}

void
pfsync_delete_tdb(struct tdb *t)
{
	struct pfsync_softc *sc = pfsyncif;

	if (sc == NULL || !ISSET(t->tdb_flags, TDBF_PFSYNC))
		return;

	sc->sc_len -= sizeof(struct pfsync_tdb);
	TAILQ_REMOVE(&sc->sc_tdb_q, t, tdb_sync_entry);
	CLR(t->tdb_flags, TDBF_PFSYNC);

	if (TAILQ_EMPTY(&sc->sc_tdb_q))
		sc->sc_len -= sizeof(struct pfsync_subheader);
}

void
pfsync_out_tdb(struct tdb *t, void *buf)
{
	struct pfsync_tdb *ut = buf;

	bzero(ut, sizeof(*ut));
	ut->spi = t->tdb_spi;
	bcopy(&t->tdb_dst, &ut->dst, sizeof(ut->dst));
	/*
	 * When a failover happens, the master's rpl is probably above
	 * what we see here (we may be up to a second late), so
	 * increase it a bit for outbound tdbs to manage most such
	 * situations.
	 *
	 * For now, just add an offset that is likely to be larger
	 * than the number of packets we can see in one second. The RFC
	 * just says the next packet must have a higher seq value.
	 *
	 * XXX What is a good algorithm for this? We could use
	 * a rate-determined increase, but to know it, we would have
	 * to extend struct tdb.
	 * XXX pt->rpl can wrap over MAXINT, but if so the real tdb
	 * will soon be replaced anyway. For now, just don't handle
	 * this edge case.
	 */
#define RPL_INCR 16384
	ut->rpl = htonl(t->tdb_rpl + (ISSET(t->tdb_flags, TDBF_PFSYNC_RPL) ?
	    RPL_INCR : 0));
	ut->cur_bytes = htobe64(t->tdb_cur_bytes);
	ut->sproto = t->tdb_sproto;
}

void
pfsync_bulk_start(void)
{
	struct pfsync_softc *sc = pfsyncif;

	sc->sc_ureq_received = time_uptime;

	if (sc->sc_bulk_next == NULL)
		sc->sc_bulk_next = TAILQ_FIRST(&state_list);
	sc->sc_bulk_last = sc->sc_bulk_next;

	if (pf_status.debug >= PF_DEBUG_MISC)
		printf("pfsync: received bulk update request\n");

	pfsync_bulk_status(PFSYNC_BUS_START);
	pfsync_bulk_update(sc);
}

void
pfsync_bulk_update(void *arg)
{
	struct pfsync_softc *sc = arg;
	struct pf_state *st = sc->sc_bulk_next;
	int i = 0;
	int s;

	s = splsoftnet();
	do {
		if (st->sync_state == PFSYNC_S_NONE &&
		    st->timeout < PFTM_MAX &&
		    st->pfsync_time <= sc->sc_ureq_received) {
			pfsync_update_state_req(st);
			i++;
		}

		st = TAILQ_NEXT(st, entry_list);
		if (st == NULL)
			st = TAILQ_FIRST(&state_list);

		if (i > 0 && TAILQ_EMPTY(&sc->sc_qs[PFSYNC_S_UPD])) {
			sc->sc_bulk_next = st;
			timeout_add(&sc->sc_bulk_tmo, 1);
			goto out;
		}
	} while (st != sc->sc_bulk_last);

	/* we're done */
	sc->sc_bulk_next = NULL;
	sc->sc_bulk_last = NULL;
	pfsync_bulk_status(PFSYNC_BUS_END);

out:
	splx(s);
}

void
pfsync_bulk_status(u_int8_t status)
{
	struct {
		struct pfsync_subheader subh;
		struct pfsync_bus bus;
	} __packed r;

	struct pfsync_softc *sc = pfsyncif;

	bzero(&r, sizeof(r));

	r.subh.action = PFSYNC_ACT_BUS;
	r.subh.count = htons(1);

	r.bus.creatorid = pf_status.hostid;
	r.bus.endtime = htonl(time_uptime - sc->sc_ureq_received);
	r.bus.status = status;

	pfsync_send_plus(&r, sizeof(r));
}

void
pfsync_bulk_fail(void *arg)
{
	struct pfsync_softc *sc = arg;

	if (sc->sc_bulk_tries++ < PFSYNC_MAX_BULKTRIES) {
		/* Try again */
		timeout_add_sec(&sc->sc_bulkfail_tmo, 5);
		pfsync_request_update(0, 0);
	} else {
		/* Pretend like the transfer was ok */
		sc->sc_ureq_sent = 0;
		sc->sc_bulk_tries = 0;
#if NCARP > 0
		if (!pfsync_sync_ok)
			carp_group_demote_adj(&sc->sc_if, -1);
#endif
		pfsync_sync_ok = 1;
		if (pf_status.debug >= PF_DEBUG_MISC)
			printf("pfsync: failed to receive bulk update\n");
	}
}

void
pfsync_send_plus(void *plus, size_t pluslen)
{
	struct pfsync_softc *sc = pfsyncif;

	if (sc->sc_len + pluslen > sc->sc_if.if_mtu)
		pfsync_sendout();

	sc->sc_plus = plus;
	sc->sc_len += (sc->sc_pluslen = pluslen);

	pfsync_sendout();
}

int
pfsync_up(void)
{
	struct pfsync_softc *sc = pfsyncif;

	if (sc == NULL || !ISSET(sc->sc_if.if_flags, IFF_RUNNING))
		return (0);

	return (1);
}

int
pfsync_state_in_use(struct pf_state *st)
{
	struct pfsync_softc *sc = pfsyncif;

	if (sc == NULL)
		return (0);

	if (st->sync_state != PFSYNC_S_NONE)
		return (1);

	if (sc->sc_bulk_next == NULL && sc->sc_bulk_last == NULL)
		return (0);

	return (1);
}

void
pfsync_timeout(void *arg)
{
	int s;

	s = splsoftnet();
	pfsync_sendout();
	splx(s);
}

/* this is a softnet/netisr handler */
void
pfsyncintr(void)
{
	pfsync_sendout();
}

int
pfsync_sysctl(int *name, u_int namelen, void *oldp, size_t *oldlenp, void *newp,
    size_t newlen)
{
	/* All sysctl names at this level are terminal. */
	if (namelen != 1)
		return (ENOTDIR);

	switch (name[0]) {
	case PFSYNCCTL_STATS:
		if (newp != NULL)
			return (EPERM);
		return (sysctl_struct(oldp, oldlenp, newp, newlen,
		    &pfsyncstats, sizeof(pfsyncstats)));
	default:
		return (ENOPROTOOPT);
	}
}
