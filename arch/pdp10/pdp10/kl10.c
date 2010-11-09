/*	$NetBSD: kl10.c,v 1.2 2005/12/11 12:18:34 christos Exp $	*/
/*
 * Copyright (c) 2003 Anders Magnusson (ragge@ludd.luth.se).
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
 * 3. The name of the author may not be used to endorse or promote products
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

#include <sys/param.h>
#include <sys/systm.h>

#include <machine/io.h>
#include <machine/cpu.h>


#define	OPTSTR "\177\10b\43T20PAG\0b\42EXADDR\0b\41EXOTIC\0" \
	"b\21C50HZ\0b\20CACHE\0b\17CHANNEL\0b\16EXTCPU\0b\15MASTEROSC\0\0"

void
kl10_conf()
{
	char buf[100];
	unsigned int cpup;

	/*
	 * Identify CPU type.
	 */
	BLKI(0, cpup);
	strcpy(cpu_model, "KL10-");
	cpu_model[5] = (cpup & 040000 ? 'E' : 'A');
	printf("\ncpu: %s, serial number 0%o, microcode version 0%o\n",
	    cpu_model, cpup & 07777, (cpup >> 18) & 0777);
	bitmask_snprintf((unsigned long long)cpup, OPTSTR, buf, sizeof(buf));
	printf("cpu options: %s\n", buf);

#ifdef notyet
	/*
	 * Turn on cache system.
	 */
#endif
}
