/*	$OpenBSD: kern_kthread.c,v 1.32 2013/03/28 16:55:25 deraadt Exp $	*/
/*	$NetBSD: kern_kthread.c,v 1.3 1998/12/22 21:21:36 kleink Exp $	*/

/*-
 * Copyright (c) 1998, 1999 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Jason R. Thorpe of the Numerical Aerospace Simulation Facility,
 * NASA Ames Research Center.
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/kthread.h>
#include <sys/proc.h>
#include <sys/wait.h>
#include <sys/malloc.h>
#include <sys/queue.h>


/*
 * note that stdarg.h and the ansi style va_start macro is used for both
 * ansi and traditional c compilers.
 * XXX: this requires that stdarg.h define: va_alist and va_dcl
 */
#include <sys/stdarg.h>

int	kthread_create_now;

/*
 * Fork a kernel thread.  Any process can request this to be done.
 * The VM space and limits, etc. will be shared with proc0.
 */
int
kthread_create(void (*func)(void *), void *arg,
    struct proc **newpp, const char *fmt, ...)
{
	struct proc *p;
	int error;
	va_list ap;

	/*
	 * First, create the new process.  Share the memory, file
	 * descriptors and don't leave the exit status around for the
	 * parent to wait for.
	 */
	error = fork1(&proc0, 0, FORK_SHAREVM|FORK_SHAREFILES|FORK_NOZOMBIE|
	    FORK_SIGHAND, NULL, 0, func, arg, NULL, &p);
	if (error)
		return (error);

	/*
	 * Mark it as a system process.
	 */
	atomic_setbits_int(&p->p_flag, P_SYSTEM);

	/* Name it as specified. */
	va_start(ap, fmt);
	vsnprintf(p->p_comm, sizeof p->p_comm, fmt, ap);
	va_end(ap);

	/* All done! */
	if (newpp != NULL)
		*newpp = p;
	return (0);
}

/*
 * Cause a kernel thread to exit.  Assumes the exiting thread is the
 * current context.
 */
void
kthread_exit(int ecode)
{

	/*
	 * XXX What do we do with the exit code?  Should we even bother
	 * XXX with it?  The parent (proc0) isn't going to do much with
	 * XXX it.
	 */
	if (ecode != 0)
		printf("WARNING: thread `%s' (%d) exits with status %d\n",
		    curproc->p_comm, curproc->p_pid, ecode);

	exit1(curproc, W_EXITCODE(ecode, 0), EXIT_NORMAL);

	/*
	 * XXX Fool the compiler.  Making exit1() __dead is a can
	 * XXX of worms right now.
	 */
	for (;;);
}

struct kthread_q {
	SIMPLEQ_ENTRY(kthread_q) kq_q;
	void (*kq_func)(void *);
	void *kq_arg;
};

SIMPLEQ_HEAD(, kthread_q) kthread_q = SIMPLEQ_HEAD_INITIALIZER(kthread_q);

/*
 * Defer the creation of a kernel thread.  Once the standard kernel threads
 * and processes have been created, this queue will be run to callback to
 * the caller to create threads for e.g. file systems and device drivers.
 */
void
kthread_create_deferred(void (*func)(void *), void *arg)
{
	struct kthread_q *kq;

	if (kthread_create_now) {
		(*func)(arg);
		return;
	}

	kq = malloc(sizeof *kq, M_TEMP, M_NOWAIT|M_ZERO);
	if (kq == NULL)
		panic("unable to allocate kthread_q");

	kq->kq_func = func;
	kq->kq_arg = arg;

	SIMPLEQ_INSERT_TAIL(&kthread_q, kq, kq_q);
}

void
kthread_run_deferred_queue(void)
{
	struct kthread_q *kq;

	/* No longer need to defer kthread creation. */
	kthread_create_now = 1;

	while ((kq = SIMPLEQ_FIRST(&kthread_q)) != NULL) {
		SIMPLEQ_REMOVE_HEAD(&kthread_q, kq_q);
		(*kq->kq_func)(kq->kq_arg);
		free(kq, M_TEMP);
	}
}
