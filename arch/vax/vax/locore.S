/*	$OpenBSD: locore.S,v 1.8 2013/11/24 22:08:25 miod Exp $   */
/*	$NetBSD: intvec.s,v 1.39 1999/06/28 08:20:48 itojun Exp $   */

/*
 * Copyright (c) 1994, 1997 Ludd, University of Lule}, Sweden.
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
 *     This product includes software developed at Ludd, University of Lule}.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
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


#include "assym.h"

#include <machine/asm.h>

#define	JSBENTRY(x) \
	.text; \
	_ALIGN_TEXT; \
	.globl x; \
	.type x,@function; \
x:

#define SCBENTRY(x)	JSBENTRY(__CONCAT(X,x))

#define TRAPCALL(namn, typ) \
SCBENTRY(namn)			; \
	pushl $0		; \
	pushl $typ		; \
	jbr trap

#define TRAPARGC(namn, typ) \
SCBENTRY(namn)			; \
	pushl $typ		; \
	jbr trap

#define FASTINTR(namn, rutin) \
SCBENTRY(namn)			; \
	pushr $0x3f		; \
	calls $0,_C_LABEL(rutin); \
	popr $0x3f		; \
	rei

#define	PUSHR	pushr	$0x3f
#define	POPR	popr	$0x3f

#define KSTACK 0
#define ISTACK 1
#define	NOVEC	.long 0
#define INTVEC(label,stack)	\
	.long	__CONCAT(X,label) + stack;

	.text

	.globl	_C_LABEL(kernbase), _C_LABEL(rpb), _C_LABEL(kernel_text)
	.set	_C_LABEL(kernel_text),KERNBASE
_C_LABEL(kernbase):
_C_LABEL(rpb):
/*
 * First page in memory we have rpb; so that we know where
 * (must be on a 64k page boundary, easiest here). We use it
 * to store SCB vectors generated when compiling the kernel,
 * and move the SCB later to somewhere else.
 */

	NOVEC;				# Unused, 0
	INTVEC(mcheck, ISTACK)		# Machine Check., 4
	INTVEC(invkstk, ISTACK) 	# Kernel Stack Invalid., 8
	NOVEC;			 	# Power Failed., C
	INTVEC(privinflt, KSTACK)	# Privileged/Reserved Instruction.
	INTVEC(xfcflt, KSTACK)		# Customer Reserved Instruction, 14
	INTVEC(resopflt, KSTACK)	# Reserved Operand/Boot Vector(?), 18
	INTVEC(resadflt, KSTACK)	# Reserved Address Mode., 1C
	INTVEC(access_v, KSTACK)	# Access Control Violation, 20
	INTVEC(transl_v, KSTACK)	# Translation Invalid, 24
	INTVEC(tracep, KSTACK)		# Trace Pending, 28
	INTVEC(breakp, KSTACK)		# Breakpoint Instruction, 2C
	NOVEC;			 	# Compatibility Exception, 30
	INTVEC(arithflt, KSTACK)	# Arithmetic Fault, 34
	NOVEC;			 	# Unused, 38
	NOVEC;			 	# Unused, 3C
	INTVEC(syscall, KSTACK)		# main syscall trap, chmk, 40
	INTVEC(resopflt, KSTACK)	# chme, 44
	INTVEC(resopflt, KSTACK)	# chms, 48
	INTVEC(resopflt, KSTACK)	# chmu, 4C
	NOVEC;				# System Backplane Exception/BIerror, 50
	INTVEC(cmrerr, ISTACK)		# Corrected Memory Read, 54
	NOVEC;				# System Backplane Alert/RXCD, 58
	INTVEC(sbiflt, ISTACK)		# System Backplane Fault, 5C
	NOVEC;				# Memory Write Timeout, 60
	NOVEC;				# Unused, 64
	NOVEC;				# Unused, 68
	NOVEC;				# Unused, 6C
	NOVEC;				# Unused, 70
	NOVEC;				# Unused, 74
	NOVEC;				# Unused, 78
	NOVEC;				# Unused, 7C
	NOVEC;				# Unused, 80
	NOVEC;				# Unused, 84
	INTVEC(astintr,	KSTACK)		# Asynchronous System Trap, AST
	NOVEC;				# Unused, 8C
	NOVEC;				# Unused, 90
	NOVEC;				# Unused, 94
	NOVEC;				# Unused, 98
	NOVEC;				# Unused, 9C
	INTVEC(softintr,ISTACK)		# Software interrupts (IPL_SOFT)
	INTVEC(softintr,ISTACK)		# Software interrupts (IPL_SOFTCLOCK)
	INTVEC(softintr,ISTACK)		# Software interrupts (IPL_SOFTNET)
	INTVEC(softintr,ISTACK)		# Software interrupts (IPL_SOFTTTY)
	NOVEC;				# Unused, B0
	NOVEC;				# Unused, B4
	NOVEC;				# Unused, B8
	INTVEC(ddbtrap, ISTACK) 	# Kernel debugger trap, BC
	INTVEC(hardclock,ISTACK)	# Interval Timer
	NOVEC;				# Unused, C4
	INTVEC(emulate, KSTACK)		# Subset instruction emulation, C8
	NOVEC;				# Unused, CC
	NOVEC;				# Unused, D0
	NOVEC;				# Unused, D4
	NOVEC;				# Unused, D8
	NOVEC;				# Unused, DC
	NOVEC;				# Unused, E0
	NOVEC;				# Unused, E4
	NOVEC;				# Unused, E8
	NOVEC;				# Unused, EC
	NOVEC;
	NOVEC;
	NOVEC;
	NOVEC;

	/* space for adapter vectors */
	.space 0x100

	.align 2
#
# mcheck is the badaddress trap, also called when referencing
# a invalid address (busserror)
# memtest holds the address to continue execution at when returning
# from a intentional test.
#
SCBENTRY(mcheck)
	tstl	_ASM_LABEL(memtest)	# Are we expecting a machine check?
	bneq	L4		# Yes.

	pushr	$0x7f
	pushab	24(%sp)
	movl	_C_LABEL(dep_call),%r6	# CPU dependent mchk handling
	calls	$1,*MCHK(%r6)
	tstl	%r0		# If not machine check, try memory error
	beql	1f
	calls	$0,*MEMERR(%r6)
	pushab	2f
	calls	$1,_C_LABEL(panic)
2:	.asciz	"mchk"
1:	popr	$0x7f
	addl2	(%sp)+,%sp

	rei

L4:	addl2	(%sp)+,%sp	# remove info pushed on stack
	# Clear the machine check condition by writing to the
	# MCESR register if available.
	cmpl	$VAX_TYP_UV2, _C_LABEL(vax_cputype)
	beql	2f
	cmpl	$VAX_TYP_SOC, _C_LABEL(vax_cputype)
	beql	2f
  	mtpr	$0,$PR_MCESR	# clear the bus error bit
2:	movl	_ASM_LABEL(memtest),(%sp)	# REI to new address
	rei

TRAPCALL(invkstk, T_KSPNOTVAL)

SCBENTRY(privinflt)	# Privileged/unimplemented instruction
#ifdef INSN_EMULATE
	jsb	unimemu	# do not return if insn emulated
#endif
	pushl $0
	pushl $T_PRIVINFLT
	jbr trap

TRAPCALL(xfcflt, T_XFCFLT);
TRAPCALL(resopflt, T_RESOPFLT)
TRAPCALL(resadflt, T_RESADFLT)

/*
 * Translation fault, used only when simulating page reference bit.
 * Therefore it is done a fast revalidation of the page if it is
 * referenced. Trouble here is the hardware bug on KA650 CPUs that
 * put in a need for an extra check when the fault is gotten during
 * PTE reference. Handled in pmap.c.
 */
SCBENTRY(transl_v)		# 20: Translation violation
	PUSHR
	pushl	28(%sp)
	pushl	28(%sp)
	calls	$2,_C_LABEL(pmap_simulref)
	tstl	%r0
	bneq	1f
	POPR
	addl2	$8,%sp
	rei
1:	POPR
	brb	access_v

SCBENTRY(access_v)		# 24: Access cntrl viol fault
access_v:
	blbs	(%sp), ptelen
	pushl	$T_ACCFLT
	bbc	$1,4(%sp),1f
	bisl2	$T_PTEFETCH,(%sp)
1:	bbc	$2,4(%sp),2f
	bisl2	$T_WRITE,(%sp)
2:	movl	(%sp), 4(%sp)
	addl2	$4, %sp
	jbr	trap

ptelen: movl	$T_PTELEN, (%sp)	# PTE must expand (or send segv)
	jbr	trap

TRAPCALL(tracep, T_TRCTRAP)
TRAPCALL(breakp, T_BPTFLT)

TRAPARGC(arithflt, T_ARITHFLT)

SCBENTRY(syscall)			# Main system call
	pushl	$T_SYSCALL
	pushr	$0xfff
	mfpr	$PR_USP, -(%sp)
	pushl	%ap
	pushl	%fp
	pushl	%sp		# pointer to syscall frame; defined in trap.h
	calls	$1, _C_LABEL(syscall)
	movl	(%sp)+, %fp
	movl	(%sp)+, %ap
	mtpr	(%sp)+, $PR_USP
	popr	$0xfff
	addl2	$8, %sp
	mtpr	$0x1f, $PR_IPL	# Be sure we can REI
	rei

SCBENTRY(cmrerr)
	PUSHR
	movl	_C_LABEL(dep_call),%r0
	calls	$0,*MEMERR(%r0)
	POPR
	rei

SCBENTRY(sbiflt)
	pushab	sbifltmsg
	calls	$1,_C_LABEL(panic)

TRAPCALL(astintr, T_ASTFLT)

FASTINTR(softintr,softintr_dispatch)

TRAPCALL(ddbtrap, T_KDBTRAP)

SCBENTRY(hardclock)
	mtpr	$0xc1,$PR_ICCS		# Reset interrupt flag
	PUSHR
	pushl	%sp
	addl2	$24,(%sp)
	movl	_C_LABEL(dep_call),%r0
	calls	$1,*HARDCLOCK(%r0)
	incl	_C_LABEL(clock_intrcnt)+EC_COUNT	# increment low longword
	adwc	$0,_C_LABEL(clock_intrcnt)+EC_COUNT+4	# add any carry to hi
							# longword
	POPR
	rei

/*
 * Main routine for traps; all go through this.
 * Note that we put USP on the frame here, which sometimes should
 * be KSP to be correct, but because we only alters it when we are
 * called from user space it doesn't care.
 * _sret is used in cpu_set_kpc to jump out to user space first time.
 */
trap:	pushr	$0xfff
	mfpr	$PR_USP, -(%sp)
	pushl	%ap
	pushl	%fp
	pushl	%sp
	calls	$1, _C_LABEL(arithflt)
	.globl	_C_LABEL(sret)
_C_LABEL(sret):
	movl	(%sp)+, %fp
	movl	(%sp)+, %ap
	mtpr	(%sp)+, $PR_USP
	popr	$0xfff
	addl2	$8, %sp
	mtpr	$0x1f, $PR_IPL	# Be sure we can REI
	rei

sbifltmsg:
	.asciz	"SBI fault"

#if INSN_EMULATE
/*
 * Table of emulated Microvax instructions supported by emulate.s.
 * Use noemulate to convert unimplemented ones to reserved instruction faults.
 */
	.globl	_C_LABEL(emtable)
_C_LABEL(emtable):
/* f8 */ .long _C_LABEL(EMashp);	.long _C_LABEL(EMcvtlp)
	 .long noemulate;		.long noemulate
/* fc */ .long noemulate;		.long noemulate
	 .long noemulate;		.long noemulate
/* 00 */ .long noemulate;		.long noemulate
	 .long noemulate;		.long noemulate
/* 04 */ .long noemulate;		.long noemulate
	 .long noemulate;		.long noemulate
/* 08 */ .long _C_LABEL(EMcvtps);	.long _C_LABEL(EMcvtsp)
	 .long noemulate;		.long _C_LABEL(EMcrc)
/* 0c */ .long noemulate;		.long noemulate
	 .long noemulate;		.long noemulate
/* 10 */ .long noemulate;		.long noemulate
	 .long noemulate;		.long noemulate
/* 14 */ .long noemulate;		.long noemulate
	 .long noemulate;		.long noemulate
/* 18 */ .long noemulate;		.long noemulate
	 .long noemulate;		.long noemulate
/* 1c */ .long noemulate;		.long noemulate
	 .long noemulate;		.long noemulate
/* 20 */ .long _C_LABEL(EMaddp4);	.long _C_LABEL(EMaddp6)
	 .long _C_LABEL(EMsubp4);	.long _C_LABEL(EMsubp6)
/* 24 */ .long _C_LABEL(EMcvtpt);	.long _C_LABEL(EMmulp)
	 .long _C_LABEL(EMcvttp);	.long _C_LABEL(EMdivp)
/* 28 */ .long noemulate;		.long _C_LABEL(EMcmpc3)
	 .long _C_LABEL(EMscanc);	.long _C_LABEL(EMspanc)
/* 2c */ .long noemulate;		.long _C_LABEL(EMcmpc5)
	 .long _C_LABEL(EMmovtc);	.long _C_LABEL(EMmovtuc)
/* 30 */ .long noemulate;		.long noemulate;
	 .long noemulate;		.long noemulate
/* 34 */ .long _C_LABEL(EMmovp);	.long _C_LABEL(EMcmpp3)
	 .long _C_LABEL(EMcvtpl);	.long _C_LABEL(EMcmpp4)
/* 38 */ .long _C_LABEL(EMeditpc);	.long _C_LABEL(EMmatchc);
	 .long _C_LABEL(EMlocc);	.long _C_LABEL(EMskpc)
#endif
/*
 * The following is called with the stack set up as follows:
 *
 *	  (sp): Opcode
 *	 4(sp): Instruction PC
 *	 8(sp): Operand 1
 *	12(sp): Operand 2
 *	16(sp): Operand 3
 *	20(sp): Operand 4
 *	24(sp): Operand 5
 *	28(sp): Operand 6
 *	32(sp): Operand 7 (unused)
 *	36(sp): Operand 8 (unused)
 *	40(sp): Return PC
 *	44(sp): Return PSL
 *	48(sp): TOS before instruction
 *
 * Each individual routine is called with the stack set up as follows:
 *
 *	  (sp): Return address of trap handler
 *	 4(sp): Opcode (will get return PSL)
 *	 8(sp): Instruction PC
 *	12(sp): Operand 1
 *	16(sp): Operand 2
 *	20(sp): Operand 3
 *	24(sp): Operand 4
 *	28(sp): Operand 5
 *	32(sp): Operand 6
 *	36(sp): saved register 11
 *	40(sp): saved register 10
 *	44(sp): Return PC
 *	48(sp): Return PSL
 *	52(sp): TOS before instruction
 *	See the VAX Architecture Reference Manual, Section B-5 for more
 *	information.
 */

SCBENTRY(emulate)
#if INSN_EMULATE
	movl	%r11,32(%sp)		# save register r11 in unused operand
	movl	%r10,36(%sp)		# save register r10 in unused operand
	cvtbl	(%sp),%r10		# get opcode
	addl2	$8,%r10			# shift negative opcodes
	subl3	%r10,$0x43,%r11		# forget it if opcode is out of range
	bcs	noemulate
	movl	_C_LABEL(emtable)[%r10],%r10 # call appropriate emulation routine
	jsb	(%r10)		# routines put return values into regs 0-5
	movl	32(%sp),%r11		# restore register r11
	movl	36(%sp),%r10		# restore register r10
	insv	(%sp),$0,$4,44(%sp)	# and condition codes in Opcode spot
	addl2	$40,%sp			# adjust stack for return
	rei
noemulate:
	addl2	$48,%sp			# adjust stack for
#endif
	.word	0xffff			# "reserved instruction fault"

	.text

/*
 * First entry routine from boot. This should be in a file called locore.
 */
ASENTRY_NOPROFILE(__start, 0)
	bisl3	$0x80000000,%r9,_C_LABEL(esym)	# End of loaded code
	pushl	$0x1f0000			# Push a nice PSL
	pushl	$to				# Address to jump to
	rei					# change to kernel stack
to:	movw	$0xfff,_C_LABEL(panic)		# Save all regs in panic
	moval	_C_LABEL(end), %r0		# Get kernel end address
	addl2	$0x3ff, %r0			# Round it up
	cmpl	_C_LABEL(esym), %r0		# Compare with symbol table end
	bleq	eskip				# Symbol table not present
	addl3	_C_LABEL(esym), $0x3ff, %r0	# Use symbol end and round
eskip:
	bicl3	$0x3ff,%r0,%r1
	movl	%r1,_C_LABEL(proc0paddr)	# save proc0 uarea pointer
	bicl3	$0x80000000,%r1,%r0		# get phys proc0 uarea addr
#if 0
	movl	%r0,PCB_PADDR(%r1)		# save PCB physical address
#endif
	mtpr	%r0,$PR_PCBB			# Save in IPR PCBB
	addl3	$USPACE,%r1,%r0			# Get kernel stack top
	mtpr	%r0,$PR_KSP			# put in IPR KSP
	movl	%r0,_C_LABEL(Sysmap)		# SPT start addr after KSP

# Set some registers in known state
	movl	%r1,%r0
	clrl	P0LR(%r0)
	clrl	P1LR(%r0)
	mtpr	$0,$PR_P0LR
	mtpr	$0,$PR_P1LR
	movl	$0x80000000,%r1
	movl	%r1,P0BR(%r0)
	movl	%r1,P1BR(%r0)
	mtpr	%r1,$PR_P0BR
	mtpr	%r1,$PR_P1BR
	clrl	IFTRAP(%r0)
	mtpr	$0,$PR_SCBB

# Copy the RPB to its new position
#if 1 /* compat with old bootblocks */
	tstl	(%ap)				# Any arguments?
	bneq	1f				# Yes, called from new boot
	movl	%r11,_C_LABEL(boothowto)	# Howto boot (single etc...)
#	movl	%r10,_C_LABEL(bootdev)		# uninteresting, will complain
	movl	%r8,_C_LABEL(avail_end)		# Usable memory (from VMB)
	clrl	-(%sp)				# Have no RPB
	brb	2f
#endif

1:	pushl	4(%ap)				# Address of old rpb
2:	calls	$1,_C_LABEL(_start)		# Jump away.
	/* NOTREACHED */


/*
 * Signal handler code.
 */

	.globl	_C_LABEL(sigcode),_C_LABEL(esigcode)
_C_LABEL(sigcode):
	movl	0x0c(%sp),%r0	/* get signal handler */
	calls	$3,(%r0)	/* and call it */
	chmk	$SYS_sigreturn	/* sigreturn frame set up by sendsig */
	chmk	$SYS_exit
	halt
	_ALIGN_TEXT
_C_LABEL(esigcode):

	.globl	_C_LABEL(idsptch), _C_LABEL(eidsptch)
_C_LABEL(idsptch):
	pushr	$0x3f
	.word	0x9f16		# jsb to absolute address
	.long	_cmn_idsptch	# the absolute address
	.long	0		# the callback interrupt routine
	.long	0		# its argument
	.long	0		# ptr to correspond evcount struct
_C_LABEL(eidsptch):

_cmn_idsptch:
	movl	(%sp)+,%r0	# get pointer to idspvec
	movl	8(%r0),%r1	# get evcount pointer
	beql	1f		# no ptr, skip increment
	incl	EC_COUNT(%r1)	# increment low longword
	adwc	$0,EC_COUNT+4(%r1) # add any carry to hi longword
1:	pushl	4(%r0)		# push argument
	calls	$1,*(%r0)	# call interrupt routine
	popr	$0x3f		# pop registers
	rei			# return from interrut

ENTRY_NOPROFILE(badaddr,R2|R3)		# Called with addr,b/w/l
	mfpr	$0x12,%r0	# splhigh()
	mtpr	$0x1f,$0x12
	movl	4(%ap),%r2	# First argument, the address
	movl	8(%ap),%r1	# Sec arg, b,w,l
	pushl	%r0		# Save old IPL
	clrl	%r3
	movab	4f,_ASM_LABEL(memtest)	# Set the return address

	caseb	%r1,$1,$4	# What is the size
1:	.word	1f-1b
	.word	2f-1b
	.word	3f-1b		# This is unused
	.word	3f-1b

1:	movb	(%r2),%r1	# Test a byte
	brb	5f

2:	movw	(%r2),%r1	# Test a word
	brb	5f

3:	movl	(%r2),%r1	# Test a long
	brb	5f

4:	incl	%r3		# Got machine chk => addr bad
5:	clrl	_ASM_LABEL(memtest)	# do not ignore further mchk
	mtpr	(%sp)+,$0x12
	movl	%r3,%r0
	ret

#ifdef DDB
/*
 * DDB is the only routine that uses setjmp/longjmp.
 */
ENTRY_NOPROFILE(setjmp, 0)
	movl	4(%ap), %r0
	movl	8(%fp), (%r0)
	movl	12(%fp), 4(%r0)
	movl	16(%fp), 8(%r0)
	addl3	%fp,$28,12(%r0)
	clrl	%r0
	ret

ENTRY_NOPROFILE(longjmp, 0)
	movl	4(%ap), %r1
	movl	$1, %r0
	movl	(%r1), %ap
	movl	4(%r1), %fp
	movl	12(%r1), %sp
	jmp	*8(%r1)
#endif

#
# void
# cpu_switchto(struct proc *oldproc = r0, struct proc *newproc = r1);
#

#define CURPROC _C_LABEL(cpu_info_store) + CI_CURPROC

JSBENTRY(__cpu_switchto)
	svpctx

	movb	$SONPROC,P_STAT(%r1)	# p->p_stat = SONPROC
	movl	%r1, CURPROC		# set new process running

	movl	P_ADDR(%r1),%r0		# Get pointer to new pcb.
	addl3	%r0,$IFTRAP,pcbtrap	# Save for copy* functions.

#
# Do the actual process switch. pc + psl are already on stack, from
# the beginning of this routine.
#
	mtpr	PCB_PADDR(%r0),$PR_PCBB

	pushl	CURPROC
	calls	$1, _C_LABEL(pmap_activate)

	ldpctx
	rei

#
# copy/fetch/store routines.
#
	.align 2
ENTRY_NOPROFILE(copyin, R2|R3|R4|R5|R6)
	movl	4(%ap), %r0
	blss	3f		# kernel space
	movl	8(%ap), %r1
	brb	2f

ENTRY_NOPROFILE(copyout, R2|R3|R4|R5|R6)
	movl	8(%ap), %r1
	blss	3f		# kernel space
	movl	4(%ap), %r0
2:	movab	1f,*pcbtrap
	movzwl	12(%ap), %r2
	movzwl	14(%ap), %r6

	movc3	%r2, (%r0), (%r1)

	tstl	%r6
	bleq	1f
0:	movb	(%r1)+, (%r3)+
	movc3	$0xffff, (%r1), (%r3)
	sobgtr	%r6,0b

1:	clrl	*pcbtrap
	ret

3:	movl	$EFAULT, %r0
	ret

/* kcopy:  just like bcopy, except return EFAULT upon failure */
ENTRY_NOPROFILE(kcopy,R2|R3|R4|R5|R6)
	movl	*pcbtrap,-(%sp)
	movab	1f,*pcbtrap
	movl	4(%ap), %r0
	movl	8(%ap), %r1
	movzwl	12(%ap), %r2
	movzwl	14(%ap), %r6

	movc3	%r2, (%r0), (%r1)

	tstl	%r6
	bleq	1f
0:	movb	(%r1)+, (%r3)+
	movc3	$0xffff, (%r1), (%r3)
	sobgtr	%r6, 0b

	/*
	 * If there is a failure, trap.c will set r0 to EFAULT, and jump
	 * to the following 1.  If not, we return 0 (movc3 sets r0 to 0).
	 */
1:
	movl	(%sp)+,*pcbtrap
	ret

ENTRY_NOPROFILE(copyinstr,0)
	tstl	4(%ap)		# is from a kernel address?
	bgeq	8f		# no, continue

6:	movl	$EFAULT,%r0
	ret

ENTRY_NOPROFILE(copyoutstr,0)
	tstl	8(%ap)		# is to a kernel address?
	bgeq	8f		# no, continue
	brb	6b

ENTRY_NOPROFILE(copystr,0)
8:	movl	4(%ap),%r4	# from
	movl	8(%ap),%r5	# to
	movl	16(%ap),%r3	# copied
	movl	12(%ap),%r2	# len

	bneq	1f		# nothing to copy?
	movl	$ENAMETOOLONG,%r0
	tstl	%r3
	beql	0f
	movl	$0,(%r3)
0:	ret

1:	movab	2f,*pcbtrap

/*
 * This routine consists of two parts: One is for MV2 that doesn't have
 * locc in hardware, the other is a fast version with locc. But because
 * locc only handles <64k strings, we default to the slow version if the
 * string is longer.
 */
	cmpl	_C_LABEL(vax_cputype),$VAX_TYP_UV2
	bneq	4f		# Check if locc emulated

9:	movl	%r2,%r0
7:	movb	(%r4)+,(%r5)+
	beql	6f		# end of string
	sobgtr	%r0,7b		# no null byte in the len first bytes?
	brb 1f

6:	tstl	%r3
	beql	5f
	incl	%r2
	subl3	%r0,%r2,(%r3)
5:	clrl	%r0
	clrl	*pcbtrap
	ret

4:	cmpl	%r2,$65535	# maxlen < 64k?
	blss	8f		# then use fast code.

	locc	$0,$65535,(%r4)	# is strlen < 64k?
	beql	9b		# No, use slow code
	subl3	%r0,$65535,%r1	# Get string len
	brb	0f		# do the copy

8:	locc	$0,%r2,(%r4)	# check for null byte
	beql	1f

	subl3	%r0,%r2,%r1	# Calculate len to copy
0:	incl	%r1		# Copy null byte also
	tstl	%r3
	beql	3f
	movl	%r1,(%r3)	# save len copied
3:	movc3	%r1,(%r4),(%r5)
	brb	4f

1:	movl	$ENAMETOOLONG,%r0
2:	movab	4f,*pcbtrap	# if we fault again, don't resume there
	subl3	8(%ap),%r5,%r1	# did we write to the string?
	beql	3f
	decl	%r5
3:	movb	$0,(%r5)	# null terminate the output string
	tstl	%r3
	beql	4f
	incl	%r1		# null byte accounts for outlen...
	movl	%r1,(%r3)	# save len copied
4:	clrl	*pcbtrap
	ret

#
# data department
#
	.data

_ASM_LABEL(memtest):			# badaddr() in progress
	.long 0
pcbtrap:
	.long 0x800001fc		# Safe place

	.globl _C_LABEL(bootdev)
_C_LABEL(bootdev):
	.long 0
