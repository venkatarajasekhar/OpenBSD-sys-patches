/*	$OpenBSD: cache.h,v 1.3 2012/04/21 12:20:30 miod Exp $	*/

/*
 * Copyright (c) 2012 Miodrag Vallat.
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

#ifndef	_MIPS64_CACHE_H_
#define	_MIPS64_CACHE_H_

/*
 * Declare canonical cache functions for a given processor.
 *
 * The following assumptions are made:
 * - only L1 has split instruction and data caches.
 * - L1 I$ is virtually indexed.
 *
 * Processor-specific routines will make extra assumptions.
 */

#define CACHE_PROTOS(chip) \
/* Figure out cache configuration */ \
void	chip##_ConfigCache(struct cpu_info *); \
/* Writeback and invalidate all caches */ \
void  	chip##_SyncCache(struct cpu_info *); \
/* Invalidate all I$ for the given range */ \
void	chip##_InvalidateICache(struct cpu_info *, vaddr_t, size_t); \
/* Writeback all D$ for the given page */ \
void	chip##_SyncDCachePage(struct cpu_info *, vaddr_t, paddr_t); \
/* Writeback all D$ for the given range */ \
void	chip##_HitSyncDCache(struct cpu_info *, vaddr_t, size_t); \
/* Invalidate all D$ for the given range */ \
void	chip##_HitInvalidateDCache(struct cpu_info *, vaddr_t, size_t); \
/* Enforce coherency of the given range */ \
void	chip##_IOSyncDCache(struct cpu_info *, vaddr_t, size_t, int);

/*
 * Cavium Octeon.
 */
CACHE_PROTOS(Octeon);

/*
 * STC Loongson 2E and 2F.
 */
CACHE_PROTOS(Loongson2);
 
/*
 * MIPS R4000 and R4400.
 */
CACHE_PROTOS(Mips4k);

/*
 * IDT/QED/PMC-Sierra R4600, R4700, R5000, RM52xx, RM7xxx, RM9xxx.
 */
CACHE_PROTOS(Mips5k);

/*
 * MIPS/NEC R10000/R120000/R140000/R16000.
 */
CACHE_PROTOS(Mips10k);

/*
 * Values used by the IOSyncDCache routine [which acts as the backend of
 * bus_dmamap_sync()].
 */
#define	CACHE_SYNC_R	0	/* WB invalidate, WT invalidate */
#define	CACHE_SYNC_W	1	/* WB writeback + invalidate, WT unaffected */
#define	CACHE_SYNC_X	2	/* WB writeback + invalidate, WT invalidate */

extern vaddr_t cache_valias_mask;

#endif	/* _MIPS64_CACHE_H_ */
