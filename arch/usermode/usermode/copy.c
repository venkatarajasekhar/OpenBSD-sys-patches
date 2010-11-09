/* $NetBSD: copy.c,v 1.2 2009/10/21 16:07:00 snj Exp $ */

/*-
 * Copyright (c) 2007 Jared D. McNeill <jmcneill@invisible.ca>
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

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: copy.c,v 1.2 2009/10/21 16:07:00 snj Exp $");

#include <sys/types.h>
#include <sys/systm.h>

int
copyin(const void *uaddr, void *kaddr, size_t len)
{
	memcpy(kaddr, uaddr, len);
	return 0;
}

int
copyout(const void *kaddr, void *uaddr, size_t len)
{
	memcpy(uaddr, kaddr, len);
	return 0;
}

int
copyinstr(const void *uaddr, void *kaddr, size_t len, size_t *done)
{
	strncpy(kaddr, uaddr, len);
	if (done)
		*done = min(strlen(uaddr), len);
	return 0;
}

int
copyoutstr(const void *kaddr, void *uaddr, size_t len, size_t *done)
{
	strncpy(uaddr, kaddr, len);
	if (done)
		*done = min(strlen(kaddr), len);
	return 0;
}

int
copystr(const void *kfaddr, void *kdaddr, size_t len, size_t *done)
{
	strncpy(kdaddr, kfaddr, len);
	if (done)
		*done = min(strlen(kfaddr), len);
	return 0;
}

int
kcopy(const void *src, void *dst, size_t len)
{
	memcpy(dst, src, len);
	return 0;
}

int
fuswintr(const void *base)
{
	return *(const short *)base;
}

int
suswintr(void *base, short c)
{
	*(short *)base = c;
	return 0;
}

int
subyte(void *base, int c)
{
	*(char *)base = c;
	return 0;
}

int
suword(void *base, long c)
{
	*(long *)base = c;
	return 0;
}
