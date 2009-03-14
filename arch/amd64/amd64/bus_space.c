/*	$OpenBSD: bus_space.c,v 1.11 2008/12/03 15:46:06 oga Exp $	*/
/*	$NetBSD: bus_space.c,v 1.2 2003/03/14 18:47:53 christos Exp $	*/

/*-
 * Copyright (c) 1996, 1997, 1998 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Charles M. Hannum and by Jason R. Thorpe of the Numerical Aerospace
 * Simulation Facility, NASA Ames Research Center.
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
#include <sys/malloc.h>
#include <sys/extent.h>

#include <uvm/uvm_extern.h>

#define _BUS_DMA_PRIVATE
#include <machine/bus.h>

#include <dev/isa/isareg.h>
#include <machine/isa_machdep.h>

/*
 * Extent maps to manage I/O and memory space.  Allocate
 * storage for 8 regions in each, initially.  Later, ioport_malloc_safe
 * will indicate that it's safe to use malloc() to dynamically allocate
 * region descriptors.
 *
 * N.B. At least two regions are _always_ allocated from the iomem
 * extent map; (0 -> ISA hole) and (end of ISA hole -> end of RAM).
 *
 * The extent maps are not static!  Machine-dependent ISA and EISA
 * routines need access to them for bus address space allocation.
 */
static	long ioport_ex_storage[EXTENT_FIXED_STORAGE_SIZE(8) / sizeof(long)];
static	long iomem_ex_storage[EXTENT_FIXED_STORAGE_SIZE(8) / sizeof(long)];
struct	extent *ioport_ex;
struct	extent *iomem_ex;
static	int ioport_malloc_safe;

int	x86_mem_add_mapping(bus_addr_t, bus_size_t,
	    int, bus_space_handle_t *);

void
x86_bus_space_init(void)
{
	/*
	 * Initialize the I/O port and I/O mem extent maps.
	 * Note: we don't have to check the return value since
	 * creation of a fixed extent map will never fail (since
	 * descriptor storage has already been allocated).
	 *
	 * N.B. The iomem extent manages _all_ physical addresses
	 * on the machine.  When the amount of RAM is found, the two
	 * extents of RAM are allocated from the map (0 -> ISA hole
	 * and end of ISA hole -> end of RAM).
	 */
	ioport_ex = extent_create("ioport", 0x0, 0xffff, M_DEVBUF,
	    (caddr_t)ioport_ex_storage, sizeof(ioport_ex_storage),
	    EX_NOCOALESCE|EX_NOWAIT);
	iomem_ex = extent_create("iomem", 0x0, 0xffffffff, M_DEVBUF,
	    (caddr_t)iomem_ex_storage, sizeof(iomem_ex_storage),
	    EX_NOCOALESCE|EX_NOWAIT);
}

void
x86_bus_space_mallocok(void)
{

	ioport_malloc_safe = 1;
}

int
bus_space_map(bus_space_tag_t t, bus_addr_t bpa, bus_size_t size, int flags,
    bus_space_handle_t *bshp)
{
	int error;
	struct extent *ex;

	/*
	 * Pick the appropriate extent map.
	 */
	if (t == X86_BUS_SPACE_IO) {
		ex = ioport_ex;
		if (flags & BUS_SPACE_MAP_LINEAR)
			return (EINVAL);
	} else if (t == X86_BUS_SPACE_MEM)
		ex = iomem_ex;
	else
		panic("bus_space_map: bad bus space tag");

	/*
	 * Before we go any further, let's make sure that this
	 * region is available.
	 */
	error = extent_alloc_region(ex, bpa, size,
	    EX_NOWAIT | (ioport_malloc_safe ? EX_MALLOCOK : 0));
	if (error)
		return (error);

	/*
	 * For I/O space, that's all she wrote.
	 */
	if (t == X86_BUS_SPACE_IO) {
		*bshp = bpa;
		return (0);
	}

	if (bpa >= IOM_BEGIN && (bpa + size) <= IOM_END) {
		*bshp = (bus_space_handle_t)ISA_HOLE_VADDR(bpa);
		return(0);
	}

	/*
	 * For memory space, map the bus physical address to
	 * a kernel virtual address.
	 */
	error = x86_mem_add_mapping(bpa, size, 0, bshp);
	if (error) {
		if (extent_free(ex, bpa, size, EX_NOWAIT |
		    (ioport_malloc_safe ? EX_MALLOCOK : 0))) {
			printf("bus_space_map: pa 0x%lx, size 0x%lx\n",
			    bpa, size);
			printf("bus_space_map: can't free region\n");
		}
	}

	return (error);
}

int
_bus_space_map(bus_space_tag_t t, bus_addr_t bpa, bus_size_t size, int flags,
    bus_space_handle_t *bshp)
{

	/*
	 * For I/O space, just fill in the handle.
	 */
	if (t == X86_BUS_SPACE_IO) {
		*bshp = bpa;
		return (0);
	}

	/*
	 * For memory space, map the bus physical address to
	 * a kernel virtual address.
	 */
	return (x86_mem_add_mapping(bpa, size, 0, bshp));
}

int
bus_space_alloc(bus_space_tag_t t, bus_addr_t rstart, bus_addr_t rend,
    bus_size_t size, bus_size_t alignment, bus_size_t boundary, int flags,
    bus_addr_t *bpap, bus_space_handle_t *bshp)
{
	struct extent *ex;
	u_long bpa;
	int error;

	/*
	 * Pick the appropriate extent map.
	 */
	if (t == X86_BUS_SPACE_IO) {
		ex = ioport_ex;
	} else if (t == X86_BUS_SPACE_MEM)
		ex = iomem_ex;
	else
		panic("bus_space_alloc: bad bus space tag");

	/*
	 * Sanity check the allocation against the extent's boundaries.
	 */
	if (rstart < ex->ex_start || rend > ex->ex_end)
		panic("bus_space_alloc: bad region start/end");

	/*
	 * Do the requested allocation.
	 */
	error = extent_alloc_subregion(ex, rstart, rend, size, alignment,
	    0, boundary,
	    EX_FAST | EX_NOWAIT | (ioport_malloc_safe ?  EX_MALLOCOK : 0),
	    &bpa);

	if (error)
		return (error);

	/*
	 * For I/O space, that's all she wrote.
	 */
	if (t == X86_BUS_SPACE_IO) {
		*bshp = *bpap = bpa;
		return (0);
	}

	/*
	 * For memory space, map the bus physical address to
	 * a kernel virtual address.
	 */
	error = x86_mem_add_mapping(bpa, size, 0, bshp);
	if (error) {
		if (extent_free(iomem_ex, bpa, size, EX_NOWAIT |
		    (ioport_malloc_safe ? EX_MALLOCOK : 0))) {
			printf("bus_space_alloc: pa 0x%lx, size 0x%lx\n",
			    bpa, size);
			printf("bus_space_alloc: can't free region\n");
		}
	}

	*bpap = bpa;

	return (error);
}

int
x86_mem_add_mapping(bus_addr_t bpa, bus_size_t size, int cacheable,
    bus_space_handle_t *bshp)
{
	if (cacheable)
		*bshp = PMAP_DIRECT_MAP(bpa);
	else
		*bshp = PMAP_DIRECT_NC_MAP(bpa);
	return 0;
}

/*
 * void _bus_space_unmap(bus_space_tag bst, bus_space_handle bsh,
 *                        bus_size_t size, bus_addr_t *adrp)
 *
 *   This function unmaps memory- or io-space mapped by the function
 *   _bus_space_map().  This function works nearly as same as
 *   bus_space_unmap(), but this function does not ask kernel
 *   built-in extents and returns physical address of the bus space,
 *   for the convenience of the extra extent manager.
 */
void
_bus_space_unmap(bus_space_tag_t t, bus_space_handle_t bsh, bus_size_t size,
    bus_addr_t *adrp)
{
	bus_addr_t bpa;

	/*
	 * Find the correct extent and bus physical address.
	 */
	if (t == X86_BUS_SPACE_IO) {
		bpa = bsh;
	} else if (t == X86_BUS_SPACE_MEM) {
		if (bsh >= atdevbase && (bsh + size) <= (atdevbase + IOM_SIZE)) {
			bpa = (bus_addr_t)ISA_PHYSADDR(bsh);
		} else {
			if (bsh >= PMAP_DIRECT_BASE_NC &&
			    bsh < PMAP_DIRECT_END_NC)
				bpa = PMAP_DIRECT_NC_UNMAP(bsh);
			else
				bpa = PMAP_DIRECT_UNMAP(bsh);
		}
	} else {
		panic("_bus_space_unmap: bad bus space tag");
	}

	if (adrp != NULL) {
		*adrp = bpa;
	}
}

void
bus_space_unmap(bus_space_tag_t t, bus_space_handle_t bsh, bus_size_t size)
{
	struct extent *ex;
	bus_addr_t bpa;

	/*
	 * Find the correct extent and bus physical address.
	 */
	if (t == X86_BUS_SPACE_IO) {
		ex = ioport_ex;
		bpa = bsh;
	} else if (t == X86_BUS_SPACE_MEM) {
		ex = iomem_ex;

		if (bsh >= atdevbase &&
		    (bsh + size) <= (atdevbase + IOM_SIZE)) {
			bpa = (bus_addr_t)ISA_PHYSADDR(bsh);
			goto ok;
		}

		if (bsh >= PMAP_DIRECT_BASE_NC &&
		    bsh < PMAP_DIRECT_END_NC)
			bpa = PMAP_DIRECT_NC_UNMAP(bsh);
		else
			bpa = PMAP_DIRECT_UNMAP(bsh);
	} else
		panic("bus_space_unmap: bad bus space tag");

ok:
	if (extent_free(ex, bpa, size,
	    EX_NOWAIT | (ioport_malloc_safe ? EX_MALLOCOK : 0))) {
		printf("bus_space_unmap: %s 0x%lx, size 0x%lx\n",
		    (t == X86_BUS_SPACE_IO) ? "port" : "pa", bpa, size);
		printf("bus_space_unmap: can't free region\n");
	}
}

void
bus_space_free(bus_space_tag_t t, bus_space_handle_t bsh, bus_size_t size)
{

	/* bus_space_unmap() does all that we need to do. */
	bus_space_unmap(t, bsh, size);
}

int
bus_space_subregion(bus_space_tag_t t, bus_space_handle_t bsh,
    bus_size_t offset, bus_size_t size, bus_space_handle_t *nbshp)
{

	*nbshp = bsh + offset;
	return (0);
}

paddr_t
bus_space_mmap(bus_space_tag_t t, bus_addr_t addr, off_t off, int prot, int flags)
{

	/* Can't mmap I/O space. */
	if (t == X86_BUS_SPACE_IO)
		return (-1);

	/*
	 * "addr" is the base address of the device we're mapping.
	 * "off" is the offset into that device.
	 *
	 * Note we are called for each "page" in the device that
	 * the upper layers want to map.
	 */
	return (atop(addr + off));
}

u_int8_t
bus_space_read_1(bus_space_tag_t t, bus_space_handle_t h, bus_size_t o)
{
	return (t == X86_BUS_SPACE_IO ? (inb(h + o)) :
	    (*(volatile u_int8_t *)(h + o)));
}

u_int16_t
bus_space_read_2(bus_space_tag_t t, bus_space_handle_t h, bus_size_t o)
{
	return (t == X86_BUS_SPACE_IO ? (inw(h + o)) :
	    (*(volatile u_int16_t *)(h + o)));
}

u_int32_t
bus_space_read_4(bus_space_tag_t t, bus_space_handle_t h, bus_size_t o)
{
	return (t == X86_BUS_SPACE_IO ? (inl(h + o)) :
	    (*(volatile u_int32_t *)(h + o)));
}

void
bus_space_read_multi_1(bus_space_tag_t t, bus_space_handle_t h, bus_size_t o,
	    u_int8_t *ptr, bus_size_t cnt)
{
	if ((t) == X86_BUS_SPACE_IO) {
		insb(h + o, ptr, cnt);
	} else {
		void *dummy1;
		int dummy2;
		void *dummy3;
		int __x;
		__asm __volatile(" cld					;"
		"1:	movb (%2),%%al				;"
		"	stosb					;"
		"	loop 1b"				:
		    "=D" (dummy1), "=c" (dummy2), "=r" (dummy3), "=&a" (__x) :
		    "0" ((ptr)), "1" ((cnt)), "2" (h + o)       :
		    "memory");
	}
}

void
bus_space_read_multi_2(bus_space_tag_t t, bus_space_handle_t h, bus_size_t o,
    u_int16_t *ptr, bus_size_t cnt)
{
	if ((t) == X86_BUS_SPACE_IO) {
		insw(h + o, ptr, cnt);
	} else {
		void *dummy1;
		int dummy2;
		void *dummy3;
		int __x;
		__asm __volatile(" cld				;"
		"1:	movw (%2),%%ax				;"
		"	stosw					;"
		"	loop 1b"				:
		    "=D" (dummy1), "=c" (dummy2), "=r" (dummy3), "=&a" (__x) :
		    "0" ((ptr)), "1" ((cnt)), "2" (h + o)       :
		    "memory");
	}
}

void
bus_space_read_multi_4(bus_space_tag_t t, bus_space_handle_t h, bus_size_t o,
    u_int32_t *ptr, bus_size_t cnt)
{
	if ((t) == X86_BUS_SPACE_IO) {
		insl(h + o, ptr, cnt);
	} else {
		void *dummy1;
		int dummy2;
		void *dummy3;
		int __x;
		__asm __volatile(" cld				;"
		"1:	movl (%2),%%eax				;"
		"	stosl					;"
		"	loop 1b"				:
		    "=D" (dummy1), "=c" (dummy2), "=r" (dummy3), "=&a" (__x) :
		    "0" ((ptr)), "1" ((cnt)), "2" (h + o)       :
		    "memory");
	}
}

void
bus_space_read_region_1(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, u_int8_t *ptr, bus_size_t cnt)
{
	if ((t) == X86_BUS_SPACE_IO) {
		int dummy1;
		void *dummy2;
		int dummy3;
		int __x;
		__asm __volatile(" cld					;"
		"1:	inb %w1,%%al				;"
		"	stosb					;"
		"	incl %1					;"
		"	loop 1b"				:
		    "=&a" (__x), "=d" (dummy1), "=D" (dummy2),
		    "=c" (dummy3)				:
		    "1" (h + o), "2" (ptr), "3" (cnt)	:
		    "memory");
	} else {
		int dummy1;
		void *dummy2;
		int dummy3;
		__asm __volatile(" cld					;"
		"	repne					;"
		"	movsb"					:
		    "=S" (dummy1), "=D" (dummy2), "=c" (dummy3)	:
		    "0" (h + o), "1" (ptr), "2" (cnt)	:
		    "memory");
	}
}

void
bus_space_read_region_2(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, u_int16_t *ptr, bus_size_t cnt)
{
	if ((t) == X86_BUS_SPACE_IO) {
		int dummy1;
		void *dummy2;
		int dummy3;
		int __x;
		__asm __volatile(" cld				;"
		"1:	inw %w1,%%ax				;"
		"	stosw					;"
		"	addl $2,%1				;"
		"	loop 1b"				:
		    "=&a" (__x), "=d" (dummy1), "=D" (dummy2),
		    "=c" (dummy3)				:
		    "1" ((h) + (o)), "2" ((ptr)), "3" ((cnt))	:
		    "memory");
	} else {
		int dummy1;
		void *dummy2;
		int dummy3;
		__asm __volatile(" cld				;"
		"	repne					;"
		"	movsw"					:
		    "=S" (dummy1), "=D" (dummy2), "=c" (dummy3)	:
		    "0" (h + o), "1" (ptr), "2" (cnt)	:
		    "memory");
	}
}

void
bus_space_read_region_4(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, u_int32_t *ptr, bus_size_t cnt)
{
	if ((t) == X86_BUS_SPACE_IO) {
		int dummy1;
		void *dummy2;
		int dummy3;
		int __x;
		__asm __volatile("cld				;"
		"1:	inl %w1,%%eax				;"
		"	stosl					;"
		"	addl $4,%1				;"
		"	loop 1b"				:
		    "=&a" (__x), "=d" (dummy1), "=D" (dummy2),
		    "=c" (dummy3)				:
		    "1" (h + o), "2" (ptr), "3" (cnt)	:
		    "memory");
	} else {
		int dummy1;
		void *dummy2;
		int dummy3;
		__asm __volatile("cld				;"
		"	repne					;"
		"	movsl"					:
		    "=S" (dummy1), "=D" (dummy2), "=c" (dummy3)	:
		    "0" (h + o), "1" (ptr), "2" (cnt)	:
		    "memory");
	}
}
/*
 *	void bus_space_write_N(bus_space_tag_t tag,
 *	    bus_space_handle_t bsh, bus_size_t offset,
 *	    u_intN_t value);
 *
 * Write the 1, 2, 4, or 8 byte value `value' to bus space
 * described by tag/handle/offset.
 */
void
bus_space_write_1(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, u_int8_t v)
{
	if (t == X86_BUS_SPACE_IO)
		outb(h + o, v);
	else
		((void)(*(volatile u_int8_t *)(h + o) = v));
}

void
bus_space_write_2(bus_space_tag_t t, bus_space_handle_t h, bus_size_t o,
    u_int16_t v)
{
	if ((t) == X86_BUS_SPACE_IO)
		outw(h + o, v);
	else
		((void)(*(volatile u_int16_t *)(h + o) = v));
}

void
bus_space_write_4(bus_space_tag_t t, bus_space_handle_t h, bus_size_t o,
    u_int32_t v)
{
	if ((t) == X86_BUS_SPACE_IO)
		outl(h + o, v);
	else
		((void)(*(volatile u_int32_t *)(h + o) = v));
}

void
bus_space_write_multi_1(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, const u_int8_t *ptr, bus_size_t cnt)
{
	if ((t) == X86_BUS_SPACE_IO) {
		outsb(h + o, ptr, cnt);
	} else {
		void *dummy1;
		int dummy2;
		void *dummy3;
		int __x;
		__asm __volatile("cld				;"
		"1:	lodsb					;"
		"	movb %%al,(%2)				;"
		"	loop 1b"				:
		    "=S" (dummy1), "=c" (dummy2), "=r" (dummy3), "=&a" (__x) :
		    "0" (ptr), "1" (cnt), "2" (h + o));
	}
}

void
bus_space_write_multi_2(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, const u_int16_t *ptr, bus_size_t cnt)
{
	if ((t) == X86_BUS_SPACE_IO) {
		outsw(h + o, ptr, cnt);
	} else {
		void *dummy1;
		int dummy2;
		void *dummy3;
		int __x;
		__asm __volatile("cld					;"
		"1:	lodsw					;"
		"	movw %%ax,(%2)				;"
		"	loop 1b"				:
		    "=S" (dummy1), "=c" (dummy2), "=r" (dummy3), "=&a" (__x) :
		    "0" (ptr), "1" (cnt), "2" (h + o));
	}
}

void
bus_space_write_multi_4(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, const u_int32_t *ptr, bus_size_t cnt)
{
	if ((t) == X86_BUS_SPACE_IO) {
		outsl(h + o, ptr, cnt);
	} else {
		void *dummy1;
		int dummy2;
		void *dummy3;
		int __x;
		__asm __volatile("cld				;"
		"1:	lodsl					;"
		"	movl %%eax,(%2)				;"
		"	loop 1b"				:
		    "=S" (dummy1), "=c" (dummy2), "=r" (dummy3), "=&a" (__x) :
		    "0" (ptr), "1" (cnt), "2" (h + o));
	}
}

void
bus_space_write_region_1(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, const u_int8_t *ptr, bus_size_t cnt)
{
	if ((t) == X86_BUS_SPACE_IO) {
		int dummy1;
		void *dummy2;
		int dummy3;
		int __x;
		__asm __volatile("cld				;"
		"1:	lodsb					;"
		"	outb %%al,%w1				;"
		"	incl %1					;"
		"	loop 1b"				:
		    "=&a" (__x), "=d" (dummy1), "=S" (dummy2),
		    "=c" (dummy3)				:
		    "1" (h + o), "2" (ptr), "3" (cnt)	:
		    "memory");
	} else {
		int dummy1;
		void *dummy2;
		int dummy3;
		__asm __volatile("cld				;"
		"	repne					;"
		"	movsb"					:
		    "=D" (dummy1), "=S" (dummy2), "=c" (dummy3)	:
		    "0" (h + o), "1" (ptr), "2" (cnt)	:
		    "memory");
	}
}

void
bus_space_write_region_2(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, const u_int16_t *ptr, bus_size_t cnt)
{
	if ((t) == X86_BUS_SPACE_IO) {
		int dummy1;
		void *dummy2;
		int dummy3;
		int __x;
		__asm __volatile("cld				;"
		"1:	lodsw					;"
		"	outw %%ax,%w1				;"
		"	addl $2,%1				;"
		"	loop 1b"				: 
		    "=&a" (__x), "=d" (dummy1), "=S" (dummy2),
		    "=c" (dummy3)				:
		    "1" (h + o), "2" (ptr), "3" (cnt)	:
		    "memory");
	} else {
		int dummy1;
		void *dummy2;
		int dummy3;
		__asm __volatile("cld				;"
		"	repne					;"
		"	movsw"					:
		    "=D" (dummy1), "=S" (dummy2), "=c" (dummy3)	:
		    "0" (h + o), "1" (ptr), "2" (cnt)	:
		    "memory");
	}
}

void
bus_space_write_region_4(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, const u_int32_t *ptr, bus_size_t cnt)
{
	if ((t) == X86_BUS_SPACE_IO) {
		int dummy1;
		void *dummy2;
		int dummy3;
		int __x;
		__asm __volatile(" cld				;"
		"1:	lodsl					;"
		"	outl %%eax,%w1				;"
		"	addl $4,%1				;"
		"	loop 1b"				:
		    "=&a" (__x), "=d" (dummy1), "=S" (dummy2),
		    "=c" (dummy3)				:
		    "1" (h + o), "2" (ptr), "3" (cnt)	:
		    "memory");
	} else {
		int dummy1;
		void *dummy2;
		int dummy3;
		__asm __volatile("cld				;"
		"	repne					;"
		"	movsl"					:
		    "=D" (dummy1), "=S" (dummy2), "=c" (dummy3)	:
		    "0" (h + o), "1" (ptr), "2" (cnt)	:
		    "memory");
	}
}

void
bus_space_set_multi_1(bus_space_tag_t t, bus_space_handle_t h, bus_size_t o,
    u_int8_t v, size_t c)
{
	bus_addr_t addr = h + o;

	if (t == X86_BUS_SPACE_IO)
		while (c--)
			outb(addr, v);
	else
		while (c--)
			*(volatile u_int8_t *)(addr) = v;
}

void
bus_space_set_multi_2(bus_space_tag_t t, bus_space_handle_t h, bus_size_t o,
    u_int16_t v, size_t c)
{
	bus_addr_t addr = h + o;

	if (t == X86_BUS_SPACE_IO)
		while (c--)
			outw(addr, v);
	else
		while (c--)
			*(volatile u_int16_t *)(addr) = v;
}

void
bus_space_set_multi_4(bus_space_tag_t t, bus_space_handle_t h, bus_size_t o,
    u_int32_t v, size_t c)
{
	bus_addr_t addr = h + o;

	if (t == X86_BUS_SPACE_IO)
		while (c--)
			outl(addr, v);
	else
		while (c--)
			*(volatile u_int32_t *)(addr) = v;
}

void
bus_space_set_region_1(bus_space_tag_t t, bus_space_handle_t h, bus_size_t o,
    u_int8_t v, size_t c)
{
	bus_addr_t addr = h + o;

	if (t == X86_BUS_SPACE_IO)
		for (; c != 0; c--, addr++)
			outb(addr, v);
	else
		for (; c != 0; c--, addr++)
			*(volatile u_int8_t *)(addr) = v;
}

void
bus_space_set_region_2(bus_space_tag_t t, bus_space_handle_t h, bus_size_t o,
    u_int16_t v, size_t c)
{
	bus_addr_t addr = h + o;

	if (t == X86_BUS_SPACE_IO)
		for (; c != 0; c--, addr += 2)
			outw(addr, v);
	else
		for (; c != 0; c--, addr += 2)
			*(volatile u_int16_t *)(addr) = v;
}

void
bus_space_set_region_4(bus_space_tag_t t, bus_space_handle_t h, bus_size_t o,
    u_int32_t v, size_t c)
{
	bus_addr_t addr = h + o;

	if (t == X86_BUS_SPACE_IO)
		for (; c != 0; c--, addr += 4)
			outl(addr, v);
	else
		for (; c != 0; c--, addr += 4)
			*(volatile u_int32_t *)(addr) = v;
}

void
bus_space_copy_region_1(bus_space_tag_t t,
    bus_space_handle_t h1, bus_size_t o1,
    bus_space_handle_t h2, bus_size_t o2, size_t c)
{
	bus_addr_t addr1 = h1 + o1;
	bus_addr_t addr2 = h2 + o2;

	if (t == X86_BUS_SPACE_IO) {
		if (addr1 >= addr2) {
			/* src after dest: copy forward */
			for (; c != 0; c--, addr1++, addr2++)
				outb(addr2, inb(addr1));
		} else {
			/* dest after src: copy backwards */
			for (addr1 += (c - 1), addr2 += (c - 1);
			    c != 0; c--, addr1--, addr2--)
				outb(addr2, inb(addr1));
		}
	} else {
		if (addr1 >= addr2) {
			/* src after dest: copy forward */
			for (; c != 0; c--, addr1++, addr2++)
				*(volatile u_int8_t *)(addr2) =
				    *(volatile u_int8_t *)(addr1);
		} else {
			/* dest after src: copy backwards */
			for (addr1 += (c - 1), addr2 += (c - 1);
			    c != 0; c--, addr1--, addr2--)
				*(volatile u_int8_t *)(addr2) =
				    *(volatile u_int8_t *)(addr1);
		}
	}
}

void
bus_space_copy_region_2(bus_space_tag_t t,
    bus_space_handle_t h1, bus_size_t o1,
    bus_space_handle_t h2, bus_size_t o2, size_t c)
{
	bus_addr_t addr1 = h1 + o1;
	bus_addr_t addr2 = h2 + o2;

	if (t == X86_BUS_SPACE_IO) {
		if (addr1 >= addr2) {
			/* src after dest: copy forward */
			for (; c != 0; c--, addr1 += 2, addr2 += 2)
				outw(addr2, inw(addr1));
		} else {
			/* dest after src: copy backwards */
			for (addr1 += 2 * (c - 1), addr2 += 2 * (c - 1);
			    c != 0; c--, addr1 -= 2, addr2 -= 2)
				outw(addr2, inw(addr1));
		}
	} else {
		if (addr1 >= addr2) {
			/* src after dest: copy forward */
			for (; c != 0; c--, addr1 += 2, addr2 += 2)
				*(volatile u_int16_t *)(addr2) =
				    *(volatile u_int16_t *)(addr1);
		} else {
			/* dest after src: copy backwards */
			for (addr1 += 2 * (c - 1), addr2 += 2 * (c - 1);
			    c != 0; c--, addr1 -= 2, addr2 -= 2)
				*(volatile u_int16_t *)(addr2) =
				    *(volatile u_int16_t *)(addr1);
		}
	}
}

void
bus_space_copy_region_4(bus_space_tag_t t,
    bus_space_handle_t h1, bus_size_t o1,
    bus_space_handle_t h2, bus_size_t o2, size_t c)
{
	bus_addr_t addr1 = h1 + o1;
	bus_addr_t addr2 = h2 + o2;

	if (t == X86_BUS_SPACE_IO) {
		if (addr1 >= addr2) {
			/* src after dest: copy forward */
			for (; c != 0; c--, addr1 += 4, addr2 += 4)
				outl(addr2, inl(addr1));
		} else {
			/* dest after src: copy backwards */
			for (addr1 += 4 * (c - 1), addr2 += 4 * (c - 1);
			    c != 0; c--, addr1 -= 4, addr2 -= 4)
				outl(addr2, inl(addr1));
		}
	} else {
		if (addr1 >= addr2) {
			/* src after dest: copy forward */
			for (; c != 0; c--, addr1 += 4, addr2 += 4)
				*(volatile u_int32_t *)(addr2) =
				    *(volatile u_int32_t *)(addr1);
		} else {
			/* dest after src: copy backwards */
			for (addr1 += 4 * (c - 1), addr2 += 4 * (c - 1);
			    c != 0; c--, addr1 -= 4, addr2 -= 4)
				*(volatile u_int32_t *)(addr2) =
				    *(volatile u_int32_t *)(addr1);
		}
	}
}

void
bus_space_barrier(bus_space_tag_t tag, bus_space_handle_t bsh,
    bus_size_t offset, bus_size_t len, int flags)
{
	if (flags == (BUS_SPACE_BARRIER_READ|BUS_SPACE_BARRIER_WRITE))
		__asm __volatile("mfence");
	else if (flags == BUS_SPACE_BARRIER_WRITE)
		__asm __volatile("sfence");
	else
		__asm __volatile("lfence");
}

