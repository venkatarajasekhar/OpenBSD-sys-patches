/*	$OpenBSD: reloc.h,v 1.5 2011/03/23 16:54:37 pirofti Exp $	*/
/* 
 * Copyright (c) 2001 Artur Grabowski <art@openbsd.org>
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

#ifndef	_MACHINE_RELOC_H_
#define	_MACHINE_RELOC_H_

#define RELOC_NONE	0

#define RELOC_COPY	19
#define RELOC_GLOB_DAT	20
#define RELOC_JMP_SLOT	21
#define RELOC_RELATIVE	22

#endif	/* _MACHINE_RELOC_H_ */
