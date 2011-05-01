/*
 * Copyright (c) 2011 Mike Larkin <mlarkin@openbsd.org>
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

/* i386 hibernate support definitions */

#define PAGE_SHIFT_4M 22
#define PAGE_MASK_4M (NBPD - 1)
#define PMAP_PA_MASK_4M ~((paddr_t)PAGE_MASK_4M)

#define HIBERNATE_STACK_PAGE	(PAGE_SIZE * 5)
#define HIBERNATE_IO_PAGE	(PAGE_SIZE * 6)
#define HIBERNATE_TEMP_PAGE	(PAGE_SIZE * 10)
#define HIBERNATE_PT_PAGE	(PAGE_SIZE * 11)
#define HIBERNATE_ALLOC_PAGE	(PAGE_SIZE * 12)
#define HIBERNATE_STACK_OFFSET	0x0F00

#define atop_4m(x) ((x) >> PAGE_SHIFT_4M)
#define s4pte_4m(va) ((pt_entry_t *)HIBERNATE_PT_PAGE + atop_4m(va))
