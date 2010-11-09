/*	$NetBSD: intr.h,v 1.28 2006/12/21 15:55:24 yamt Exp $	*/

/*-
 * Copyright (c) 1998 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Charles M. Hannum.
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
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _PREP_INTR_H_
#define _PREP_INTR_H_

/* Interrupt priority `levels'. */
#define	IPL_NONE	9	/* nothing */
#define	IPL_SOFTCLOCK	8	/* software clock interrupt */
#define	IPL_SOFTNET	7	/* software network interrupt */
#define	IPL_BIO		6	/* block I/O */
#define	IPL_NET		5	/* network */
#define	IPL_SOFTSERIAL	4	/* software serial interrupt */
#define	IPL_TTY		3	/* terminal */
#define	IPL_LPT		IPL_TTY
#define	IPL_VM		3	/* memory allocation */
#define	IPL_AUDIO	2	/* audio */
#define	IPL_CLOCK	1	/* clock */
#define	IPL_STATCLOCK	IPL_CLOCK
#define	IPL_HIGH	1	/* everything */
#define	IPL_SCHED	IPL_HIGH
#define	IPL_LOCK	IPL_HIGH
#define	IPL_SERIAL	0	/* serial */
#define	NIPL		10

/* Interrupt sharing types. */
#define	IST_NONE	0	/* none */
#define	IST_PULSE	1	/* pulsed */
#define	IST_EDGE	2	/* edge-triggered */
#define	IST_LEVEL	3	/* level-triggered */

#ifndef _LOCORE
#include <powerpc/softintr.h>
#include <machine/cpu.h>

/*
 * Interrupt handler chains.  intr_establish() inserts a handler into
 * the list.  The handler is called with its (single) argument.
 */
struct intrhand {
	int	(*ih_fun)(void *);
	void	*ih_arg;
	u_long	ih_count;
	struct	intrhand *ih_next;
	int	ih_level;
	int	ih_irq;
};

#include <sys/device.h>
struct intrsource {
	int is_type;
	int is_level;
	int is_hwirq;
	int is_mask;
	struct intrhand *is_hand;
	struct evcnt is_ev;
	char is_source[16];
};

int splraise(int);
int spllower(int);
void softintr(int);

void do_pending_int(void);

void init_intr(void);
void init_intr_ivr(void);
void init_intr_openpic(void);
void openpic_init(unsigned char *);

void enable_intr(void);
void disable_intr(void);

void *intr_establish(int, int, int, int (*)(void *), void *);
void intr_disestablish(void *);

void softnet(int);
void softserial(void);
int isa_intr(void);
void isa_intr_mask(int);
void isa_intr_clr(int);
void isa_setirqstat(int, int, int);

extern int imen;
extern int imask[];
extern struct intrhand *intrhand[];
extern vaddr_t prep_intr_reg;
extern uint32_t prep_intr_reg_off;

#define	ICU_LEN			32
extern struct intrsource intrsources[ICU_LEN];

#define	IRQ_SLAVE		2
#define	LEGAL_IRQ(x)		((x) >= 0 && (x) < ICU_LEN && (x) != IRQ_SLAVE)
#define	I8259_INTR_NUM		16
#define	OPENPIC_INTR_NUM	((ICU_LEN)-(I8259_INTR_NUM))
#define	CLKF_BASEPRI(frame)	((frame)->pri == 0)

#define	PREP_INTR_REG	0xbffff000
#define	INTR_VECTOR_REG	0xff0

#define	SINT_CLOCK	0x20000000
#define	SINT_NET	0x40000000
#define	SINT_SERIAL	0x80000000
#define	SPL_CLOCK	0x00000001
#define	SINT_MASK	(SINT_CLOCK|SINT_NET|SINT_SERIAL)

#define	CNT_SINT_NET	29
#define	CNT_SINT_CLOCK	30
#define	CNT_SINT_SERIAL	31
#define	CNT_CLOCK	0

#define	spllowersoftclock() spllower(imask[IPL_SOFTCLOCK])

#define setsoftclock()  softintr(SINT_CLOCK);
#define setsoftnet()    softintr(SINT_NET);
#define setsoftserial() softintr(SINT_SERIAL);

#define	splx(x)		spllower(x)
#define	spl0()		spllower(0)

typedef int ipl_t;
typedef struct {
	ipl_t _ipl;
} ipl_cookie_t;

static inline ipl_cookie_t
makeiplcookie(ipl_t ipl)
{

	return (ipl_cookie_t){._ipl = ipl};
}

static inline int
splraiseipl(ipl_cookie_t icookie)
{

	return splraise(imask[icookie._ipl]);
}

#include <sys/spl.h>

#endif /* !_LOCORE */

#endif /* !_PREP_INTR_H_ */
