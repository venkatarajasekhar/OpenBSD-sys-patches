/*	$OpenBSD: trap.c,v 1.41 2002/03/16 01:13:42 mickey Exp $	*/

/*
 * Copyright (c) 1998-2001 Michael Shalayeff
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Michael Shalayeff.
 * 4. The name of the author may not be used to endorse or promote products
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
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* #define TRAPDEBUG */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/syscall.h>
#include <sys/ktrace.h>
#include <sys/proc.h>
#include <sys/signalvar.h>
#include <sys/user.h>

#include <net/netisr.h>

#include <uvm/uvm.h>

#include <machine/autoconf.h>

#ifdef DDB
#include <machine/db_machdep.h>
#ifdef TRAPDEBUG
#include <ddb/db_output.h>
#endif
#endif

const char *trap_type[] = {
	"invalid",
	"HPMC",
	"power failure",
	"recovery counter",
	"external interrupt",
	"LPMC",
	"ITLB miss fault",
	"instruction protection",
	"Illegal instruction",
	"break instruction",
	"privileged operation",
	"privileged register",
	"overflow",
	"conditional",
	"assist exception",
	"DTLB miss",
	"ITLB non-access miss",
	"DTLB non-access miss",
	"data protection/rights/alignment",
	"data break",
	"TLB dirty",
	"page reference",
	"assist emulation",
	"higher-priv transfer",
	"lower-priv transfer",
	"taken branch",
	"data access rights",
	"data protection",
	"unaligned data ref",
};
int trap_types = sizeof(trap_type)/sizeof(trap_type[0]);

u_int32_t sir;
int want_resched, astpending;

void syscall(struct trapframe *frame, int *args);

static __inline void
userret (struct proc *p, register_t pc, u_quad_t oticks)
{
	int sig;

	/* take pending signals */
	while ((sig = CURSIG(p)) != 0)
		postsig(sig);

	p->p_priority = p->p_usrpri;
	if (want_resched) {
		/*
		 * We're being preempted.
		 */
		preempt(NULL);
		while ((sig = CURSIG(p)) != 0)
			postsig(sig);
	}

	/*
	 * If profiling, charge recent system time to the trapped pc.
	 */
	if (p->p_flag & P_PROFIL) {
		extern int psratio;

		addupc_task(p, pc, (int)(p->p_sticks - oticks) * psratio);
	}

	curpriority = p->p_priority;
}

void
trap(type, frame)
	int type;
	struct trapframe *frame;
{
	struct proc *p = curproc;
	struct pcb *pcbp;
	vaddr_t va;
	struct vm_map *map;
	struct vmspace *vm;
	register vm_prot_t vftype;
	register pa_space_t space;
	union sigval sv;
	u_int opcode;
	int ret, s, si, trapnum;
	const char *tts;

	trapnum = type & ~T_USER;
	opcode = frame->tf_iir;
	if (trapnum == T_ITLBMISS || trapnum == T_ITLBMISSNA) {
		va = frame->tf_iioq_head;
		space = frame->tf_iisq_head;
		vftype = VM_PROT_READ;	/* XXX VM_PROT_EXECUTE ??? */
	} else {
		va = frame->tf_ior;
		space = frame->tf_isr;
		vftype = inst_store(opcode) ? VM_PROT_WRITE : VM_PROT_READ;
	}

	if (frame->tf_flags & TFF_LAST)
		p->p_md.md_regs = frame;

#ifdef TRAPDEBUG
	if (trapnum > trap_types)
		tts = "reserved";
	else
		tts = trap_type[trapnum];

	if (trapnum != T_INTERRUPT && trapnum != T_IBREAK)
		db_printf("trap: %x, %s for %x:%x at %x:%x, fl=%x, fp=%p\n",
		    type, tts, space, va, frame->tf_iisq_head,
		    frame->tf_iioq_head, frame->tf_flags, frame);
	else if (trapnum  == T_IBREAK)
		db_printf("trap: break instruction %x:%x at %x:%x, fp=%p\n",
		    break5(opcode), break13(opcode),
		    frame->tf_iisq_head, frame->tf_iioq_head, frame);

	{
		extern int etext;
		if (frame < (struct trapframe *)&etext) {
			printf("trap: bogus frame ptr %p\n", frame);
			goto dead_end;
		}
	}
#endif
	switch (type) {
	case T_NONEXIST:
	case T_NONEXIST|T_USER:
#ifndef DDB
		/* we've got screwed up by the central scrutinizer */
		panic ("trap: elvis has just left the building!");
		break;
#else
		goto dead_end;
#endif
	case T_RECOVERY:
	case T_RECOVERY|T_USER:
#ifndef DDB
		/* XXX will implement later */
		printf ("trap: handicapped");
		break;
#else
		goto dead_end;
#endif

#ifdef DIAGNOSTIC
	case T_EXCEPTION:
		panic("FPU/SFU emulation botch");

		/* these just can't happen ever */
	case T_PRIV_OP:
	case T_PRIV_REG:
		/* these just can't make it to the trap() ever */
	case T_HPMC:      case T_HPMC | T_USER:
	case T_EMULATION: case T_EMULATION | T_USER:
#endif
	case T_IBREAK:
	case T_DATALIGN:
	case T_DBREAK:
	dead_end:
#ifdef DDB
		if (kdb_trap (type, va, frame)) {
			if (type == T_IBREAK) {
				/* skip break instruction */
				frame->tf_iioq_head = frame->tf_iioq_tail;
				frame->tf_iioq_tail += 4;
			}
			return;
		}
#else
		if (type == T_DATALIGN)
			panic ("trap: %s at 0x%x", tts, va);
		else
			panic ("trap: no debugger for \"%s\" (%d)", tts, type);
#endif
		break;

	case T_IBREAK | T_USER:
	case T_DBREAK | T_USER:
		/* pass to user debugger */
		break;

	case T_EXCEPTION | T_USER:	/* co-proc assist trap */
		sv.sival_int = va;
		trapsignal(p, SIGFPE, type &~ T_USER, FPE_FLTINV, sv);
		break;

	case T_OVERFLOW | T_USER:
		sv.sival_int = va;
		trapsignal(p, SIGFPE, type &~ T_USER, FPE_INTOVF, sv);
		break;
		
	case T_CONDITION | T_USER:
		break;

	case T_ILLEGAL | T_USER:
		sv.sival_int = va;
		trapsignal(p, SIGILL, type &~ T_USER, ILL_ILLOPC, sv);
		break;

	case T_PRIV_OP | T_USER:
		sv.sival_int = va;
		trapsignal(p, SIGILL, type &~ T_USER, ILL_PRVOPC, sv);
		break;

	case T_PRIV_REG | T_USER:
		sv.sival_int = va;
		trapsignal(p, SIGILL, type &~ T_USER, ILL_PRVREG, sv);
		break;

		/* these should never got here */
	case T_HIGHERPL | T_USER:
	case T_LOWERPL | T_USER:
		sv.sival_int = va;
		trapsignal(p, SIGSEGV, type &~ T_USER, SEGV_ACCERR, sv);
		break;

	case T_IPROT | T_USER:
	case T_DPROT | T_USER:
		sv.sival_int = va;
		trapsignal(p, SIGSEGV, vftype, SEGV_ACCERR, sv);
		break;

	case T_DATACC:   	case T_USER | T_DATACC:
	case T_ITLBMISS:	case T_USER | T_ITLBMISS:
	case T_DTLBMISS:	case T_USER | T_DTLBMISS:
	case T_ITLBMISSNA:	case T_USER | T_ITLBMISSNA:
	case T_DTLBMISSNA:	case T_USER | T_DTLBMISSNA:
	case T_TLB_DIRTY:	case T_USER | T_TLB_DIRTY:
		va = hppa_trunc_page(va);
		vm = p->p_vmspace;

		if (!vm) {
#ifdef TRAPDEBUG
			printf("trap: no vm, p=%p\n", p);
#endif
			goto dead_end;
		}

		/*
		 * it could be a kernel map for exec_map faults
		 */
		if (!(type & T_USER) && space == HPPA_SID_KERNEL)
			map = kernel_map;
		else
			map = &vm->vm_map;

		if (map->pmap->pm_space != space) {
#ifdef TRAPDEBUG
			printf("trap: space missmatch %d != %d\n",
			    space, map->pmap->pm_space);
#endif
			/* actually dump the user, crap the kernel */
			goto dead_end;
		}

#ifdef TRAPDEBUG
		if (space == -1) {
			extern int pmapdebug;
			pmapdebug = 0xffffff;
		}
#endif

		ret = uvm_fault(map, va, 0, vftype);

#ifdef TRAPDEBUG
		if (space == -1) {
			extern int pmapdebug;
			pmapdebug = 0;
		}

		printf("uvm_fault(%p, %x, %d, %d)=%d\n",
		    map, va, 0, vftype, ret);
#endif

		/*
		 * If this was a stack access we keep track of the maximum
		 * accessed stack size.  Also, if uvm_fault gets a protection
		 * failure it is due to accessing the stack region outside
		 * the current limit and we need to reflect that as an access
		 * error.
		 */
		if (va >= (vaddr_t)vm->vm_maxsaddr + vm->vm_ssize) {
			if (ret == 0) {
				vsize_t nss = btoc(va - USRSTACK + NBPG);
				if (nss > vm->vm_ssize)
					vm->vm_ssize = nss;
			} else if (ret == EACCES)
				ret = EFAULT;
		}

		if (ret != 0) {
			if (type & T_USER) {
				sv.sival_int = frame->tf_ior;
				trapsignal(p, SIGSEGV, vftype, SEGV_MAPERR, sv);
			} else {
				if (p && p->p_addr->u_pcb.pcb_onfault) {
					pcbp = &p->p_addr->u_pcb;
					frame->tf_iioq_tail = 4 +
					    (frame->tf_iioq_head =
						pcbp->pcb_onfault);
					break;
				}
#if 0
if (kdb_trap (type, va, frame))
	return;
#else
				panic("trap: uvm_fault(%p, %x, %d, %d): %d",
				    map, va, 0, vftype, ret);
#endif
			}
		}
		break;

	case T_DATALIGN | T_USER:
		sv.sival_int = va;
		trapsignal(p, SIGBUS, vftype, BUS_ADRALN, sv);
		break;

	case T_INTERRUPT:
	case T_INTERRUPT|T_USER:
		frame->tf_flags |= TFF_INTR;
		cpu_intr(frame);
#if 0
if (kdb_trap (type, va, frame))
return;
#endif
		/* FALLTHROUGH */
	case T_LOWERPL:
		__asm __volatile ("ldcws 0(%1), %0"
				  : "=r" (si) : "r" (&sir));
		s = spl0();
		if (si & SIR_CLOCK) {
			splclock();
			softclock();
			spl0();
		}

		if (si & SIR_NET) {
			register int ni;
			/* use atomic "load & clear" */
			__asm __volatile ("ldcws 0(%1), %0"
					  : "=r" (ni) : "r" (&netisr));
			splnet();
#define	DONETISR(m,c) if (ni & (1 << (m))) c()
#include <net/netisr_dispatch.h>
		}
		splx(s);
		break;

	case T_DPROT:
	case T_IPROT:
	case T_OVERFLOW:
	case T_CONDITION:
	case T_ILLEGAL:
	case T_HIGHERPL:
	case T_TAKENBR:
	case T_POWERFAIL:
	case T_LPMC:
	case T_PAGEREF:
	case T_DATAPID:  	case T_DATAPID  | T_USER:
		if (0 /* T-chip */) {
			break;
		}
		/* FALLTHROUGH to unimplemented */
	default:
#if 0
if (kdb_trap (type, va, frame))
	return;
#endif
		panic ("trap: unimplemented \'%s\' (%d)", tts, type);
	}

	if (type & T_USER)
		userret(p, p->p_md.md_regs->tf_iioq_head, 0);
}

void
child_return(arg)
	void *arg;
{
	struct proc *p = (struct proc *)arg;
	userret(p, p->p_md.md_regs->tf_iioq_head, 0);
#ifdef KTRACE
	if (KTRPOINT(p, KTR_SYSRET))
		ktrsysret(p, SYS_fork, 0, 0);
#endif
}

/*
 * call actual syscall routine
 * from the low-level syscall handler:
 * - all HPPA_FRAME_NARGS syscall's arguments supposed to be copied onto
 *   our stack, this wins compared to copyin just needed amount anyway
 * - register args are copied onto stack too
 */
void
syscall(frame, args)
	struct trapframe *frame;
	int *args;
{
	register struct proc *p;
	register const struct sysent *callp;
	int nsys, code, argsize, error;
	int rval[2];

	uvmexp.syscalls++;

	if (!USERMODE(frame->tf_iioq_head))
		panic("syscall");

	p = curproc;
	p->p_md.md_regs = frame;
	nsys = p->p_emul->e_nsysent;
	callp = p->p_emul->e_sysent;
	code = frame->tf_t1;
	switch (code) {
	case SYS_syscall:
		code = *args;
		args += 1;
		break;
	case SYS___syscall:
		if (callp != sysent)
			break;
		code = *args;
		args += 2;
	}

	if (code < 0 || code >= nsys)
		callp += p->p_emul->e_nosys;	/* bad syscall # */
	else
		callp += code;
	argsize = callp->sy_argsize;

#ifdef SYSCALL_DEBUG
	scdebug_call(p, code, args);
#endif
#ifdef KTRACE
	if (KTRPOINT(p, KTR_SYSCALL))
		ktrsyscall(p, code, argsize, args);
#endif

	rval[0] = 0;
	rval[1] = 0;
	switch (error = (*callp->sy_call)(p, args, rval)) {
	case 0:
		p = curproc;			/* changes on exec() */
		frame = p->p_md.md_regs;
		frame->tf_ret0 = rval[0];
		frame->tf_ret1 = rval[1];
		frame->tf_t1 = 0;
		break;
	case ERESTART:
		frame->tf_iioq_head -= 12;
		frame->tf_iioq_tail -= 12;
		break;
	case EJUSTRETURN:
		p = curproc;
		break;
	default:
		if (p->p_emul->e_errno)
			error = p->p_emul->e_errno[error];
		frame->tf_t1 = error;
		break;
	}
#ifdef SYSCALL_DEBUG
	scdebug_ret(p, code, error, rval);
#endif
	userret(p, frame->tf_iioq_head, 0);
#ifdef KTRACE
	if (KTRPOINT(p, KTR_SYSRET))
		ktrsysret(p, code, error, rval[0]);
#endif
}
