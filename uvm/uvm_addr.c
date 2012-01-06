/*
 * Copyright (c) 2011 Ariane van der Steldt <ariane@stack.nl>
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

#define DEBUG

#include <sys/param.h>
#include <uvm/uvm.h>
#include <uvm/uvm_addr.h>
#include <sys/pool.h>
#include <dev/rndvar.h>

/* Max gap between hint allocations. */
#define UADDR_HINT_MAXGAP	(4 * PAGE_SIZE)
/* Number of pivots in pivot allocator. */
#define NUM_PIVOTS		16
/*
 * Max number (inclusive) of pages the pivot allocator
 * will place between allocations.
 *
 * The uaddr_pivot_random() function attempts to bias towards
 * small space between allocations, so putting a large number here is fine.
 */
#define PIVOT_RND		8
/*
 * Number of allocations that a pivot can supply before expiring.
 * When a pivot expires, a new pivot has to be found.
 *
 * Must be at least 1.
 */
#define PIVOT_EXPIRE		1024


/* Pool with uvm_addr_state structures. */
struct pool uaddr_pool;
struct pool uaddr_hint_pool;
struct pool uaddr_bestfit_pool;
struct pool uaddr_pivot_pool;
struct pool uaddr_rnd_pool;

/* uvm_addr state for hint based selector. */
struct uaddr_hint_state {
	struct uvm_addr_state		 uaddr;
	vsize_t				 max_dist;
};

/* uvm_addr state for bestfit selector. */
struct uaddr_bestfit_state {
	struct uvm_addr_state		 ubf_uaddr;
	struct uaddr_free_rbtree	 ubf_free;
};

/* uvm_addr state for rnd selector. */
struct uaddr_rnd_state {
	struct uvm_addr_state		 ur_uaddr;
	TAILQ_HEAD(, vm_map_entry)	 ur_free;
};

/*
 * Definition of a pivot in pivot selector.
 */
struct uaddr_pivot {
	vaddr_t				 addr;	/* End of prev. allocation. */
	int				 expire;/* Best before date. */
	int				 dir;	/* Direction. */
	struct vm_map_entry		*entry; /* Will contain next alloc. */
};
/* uvm_addr state for pivot selector. */
struct uaddr_pivot_state {
	struct uvm_addr_state		 up_uaddr;

	/* Free space tree, for fast pivot selection. */
	struct uaddr_free_rbtree	 up_free;

	/* List of pivots. The pointers point to after the last allocation. */
	struct uaddr_pivot		 up_pivots[NUM_PIVOTS];
};

/*
 * Free space comparison.
 * Compares smaller free-space before larger free-space.
 */
static __inline int
uvm_mapent_fspace_cmp(struct vm_map_entry *e1, struct vm_map_entry *e2)
{
	if (e1->fspace != e2->fspace)
		return (e1->fspace < e2->fspace ? -1 : 1);
	return (e1->start < e2->start ? -1 : e1->start > e2->start);
}

/* Forward declaration (see below). */
extern const struct uvm_addr_functions uaddr_kernel_functions;
struct uvm_addr_state uaddr_kbootstrap;


/*
 * Support functions.
 */

struct vm_map_entry	*uvm_addr_entrybyspace(struct uaddr_free_rbtree*,
			    vsize_t);
void			 uaddr_kinsert(struct vm_map*, struct uvm_addr_state*,
			    struct vm_map_entry*);
void			 uaddr_kremove(struct vm_map*, struct uvm_addr_state*,
			    struct vm_map_entry*);
void			 uaddr_kbootstrapdestroy(struct uvm_addr_state*);

void			 uaddr_destroy(struct uvm_addr_state*);
void			 uaddr_hint_destroy(struct uvm_addr_state*);
void			 uaddr_kbootstrap_destroy(struct uvm_addr_state*);
void			 uaddr_rnd_destroy(struct uvm_addr_state*);
void			 uaddr_bestfit_destroy(struct uvm_addr_state*);
void			 uaddr_pivot_destroy(struct uvm_addr_state*);

int			 uaddr_lin_select(struct vm_map*,
			    struct uvm_addr_state*, struct vm_map_entry**,
			    vaddr_t*, vsize_t, vaddr_t, vaddr_t, vm_prot_t,
			    vaddr_t);
int			 uaddr_kbootstrap_select(struct vm_map*,
			    struct uvm_addr_state*, struct vm_map_entry**,
			    vaddr_t*, vsize_t, vaddr_t, vaddr_t, vm_prot_t,
			    vaddr_t);
int			 uaddr_rnd_select(struct vm_map*,
			    struct uvm_addr_state*, struct vm_map_entry**,
			    vaddr_t*, vsize_t, vaddr_t, vaddr_t, vm_prot_t,
			    vaddr_t);
int			 uaddr_hint_select(struct vm_map*,
			    struct uvm_addr_state*, struct vm_map_entry**,
			    vaddr_t*, vsize_t, vaddr_t, vaddr_t, vm_prot_t,
			    vaddr_t);
int			 uaddr_bestfit_select(struct vm_map*,
			    struct uvm_addr_state*, struct vm_map_entry**,
			    vaddr_t*, vsize_t, vaddr_t, vaddr_t, vm_prot_t,
			    vaddr_t);
int			 uaddr_pivot_select(struct vm_map*,
			    struct uvm_addr_state*, struct vm_map_entry**,
			    vaddr_t*, vsize_t, vaddr_t, vaddr_t, vm_prot_t,
			    vaddr_t);
int			 uaddr_stack_brk_select(struct vm_map*,
			    struct uvm_addr_state*, struct vm_map_entry**,
			    vaddr_t*, vsize_t, vaddr_t, vaddr_t, vm_prot_t,
			    vaddr_t);

void			 uaddr_rnd_insert(struct vm_map*,
			    struct uvm_addr_state*, struct vm_map_entry*);
void			 uaddr_rnd_remove(struct vm_map*,
			    struct uvm_addr_state*, struct vm_map_entry*);
void			 uaddr_bestfit_insert(struct vm_map*,
			    struct uvm_addr_state*, struct vm_map_entry*);
void			 uaddr_bestfit_remove(struct vm_map*,
			    struct uvm_addr_state*, struct vm_map_entry*);
void			 uaddr_pivot_insert(struct vm_map*,
			    struct uvm_addr_state*, struct vm_map_entry*);
void			 uaddr_pivot_remove(struct vm_map*,
			    struct uvm_addr_state*, struct vm_map_entry*);

vsize_t			 uaddr_pivot_random(void);
int			 uaddr_pivot_newpivot(struct vm_map*,
			    struct uaddr_pivot_state*, struct uaddr_pivot*,
			    struct vm_map_entry**, vaddr_t*,
			    vsize_t, vaddr_t, vaddr_t, vsize_t, vsize_t);

#if defined(DEBUG) || defined(DDB)
void			 uaddr_pivot_print(struct uvm_addr_state*, boolean_t,
			    int (*)(const char*, ...));
void			 uaddr_rnd_print(struct uvm_addr_state*, boolean_t,
			    int (*)(const char*, ...));
#endif /* DEBUG || DDB */


/*
 * Find smallest entry in tree that will fit sz bytes.
 */
struct vm_map_entry*
uvm_addr_entrybyspace(struct uaddr_free_rbtree *free, vsize_t sz)
{
	struct vm_map_entry	*tmp, *res;

	tmp = RB_ROOT(free);
	res = NULL;
	while (tmp) {
		if (tmp->fspace >= sz) {
			res = tmp;
			tmp = RB_LEFT(tmp, dfree.rbtree);
		} else if (tmp->fspace < sz)
			tmp = RB_RIGHT(tmp, dfree.rbtree);
	}
	return res;
}

static __inline vaddr_t
uvm_addr_align_forward(vaddr_t addr, vaddr_t align, vaddr_t offset)
{
	vaddr_t adjusted;

	KASSERT(offset < align || (align == 0 && offset == 0));
	KASSERT((align & (align - 1)) == 0);
	KASSERT((offset & PAGE_MASK) == 0);

	align = MAX(align, PAGE_SIZE);
	adjusted = addr & ~(align - 1);
	adjusted += offset;
	return (adjusted < addr ? adjusted + align : adjusted);
}

static __inline vaddr_t
uvm_addr_align_backward(vaddr_t addr, vaddr_t align, vaddr_t offset)
{
	vaddr_t adjusted;

	KASSERT(offset < align || (align == 0 && offset == 0));
	KASSERT((align & (align - 1)) == 0);
	KASSERT((offset & PAGE_MASK) == 0);

	align = MAX(align, PAGE_SIZE);
	adjusted = addr & ~(align - 1);
	adjusted += offset;
	return (adjusted > addr ? adjusted - align : adjusted);
}

/*
 * Try to fit the requested space into the entry.
 */
int
uvm_addr_fitspace(vaddr_t *min_result, vaddr_t *max_result,
    vaddr_t low_addr, vaddr_t high_addr, vsize_t sz,
    vaddr_t align, vaddr_t offset,
    vsize_t before_gap, vsize_t after_gap)
{
	vaddr_t	tmp;
	vsize_t	fspace;

	if (low_addr > high_addr)
		return ENOMEM;
	fspace = high_addr - low_addr;
	if (fspace < sz + before_gap + after_gap)
		return ENOMEM;

	/*
	 * Calculate lowest address.
	 */
	low_addr += before_gap;
	low_addr = uvm_addr_align_forward(tmp = low_addr, align, offset);
	if (low_addr < tmp)	/* Overflow during alignment. */
		return ENOMEM;
	if (high_addr - after_gap - sz < low_addr)
		return ENOMEM;

	/*
	 * Calculate highest address.
	 */
	high_addr -= after_gap + sz;
	high_addr = uvm_addr_align_backward(tmp = high_addr, align, offset);
	if (high_addr > tmp)	/* Overflow during alignment. */
		return ENOMEM;
	if (low_addr > high_addr)
		return ENOMEM;

	*min_result = low_addr;
	*max_result = high_addr;
	return 0;
}


/*
 * Initialize uvm_addr.
 */
void
uvm_addr_init()
{
	pool_init(&uaddr_pool, sizeof(struct uvm_addr_state),
	    0, 0, 0, "uaddr", &pool_allocator_nointr);
	pool_init(&uaddr_hint_pool, sizeof(struct uaddr_hint_state),
	    0, 0, 0, "uaddrhint", &pool_allocator_nointr);
	pool_init(&uaddr_bestfit_pool, sizeof(struct uaddr_bestfit_state),
	    0, 0, 0, "uaddrbestfit", &pool_allocator_nointr);
	pool_init(&uaddr_pivot_pool, sizeof(struct uaddr_pivot_state),
	    0, 0, 0, "uaddrpivot", &pool_allocator_nointr);
	pool_init(&uaddr_rnd_pool, sizeof(struct uaddr_rnd_state),
	    0, 0, 0, "uaddrrnd", &pool_allocator_nointr);

	uaddr_kbootstrap.uaddr_minaddr = PAGE_SIZE;
	uaddr_kbootstrap.uaddr_maxaddr = -(vaddr_t)PAGE_SIZE;
	uaddr_kbootstrap.uaddr_functions = &uaddr_kernel_functions;
}

/*
 * Invoke destructor function of uaddr.
 */
void
uvm_addr_destroy(struct uvm_addr_state *uaddr)
{
	if (uaddr)
		(*uaddr->uaddr_functions->uaddr_destroy)(uaddr);
}

/*
 * Move address forward to satisfy align, offset.
 */
vaddr_t
uvm_addr_align(vaddr_t addr, vaddr_t align, vaddr_t offset)
{
	vaddr_t result = (addr & ~(align - 1)) + offset;
	if (result < addr)
		result += align;
	return result;
}

/*
 * Move address backwards to satisfy align, offset.
 */
vaddr_t
uvm_addr_align_back(vaddr_t addr, vaddr_t align, vaddr_t offset)
{
	vaddr_t result = (addr & ~(align - 1)) + offset;
	if (result > addr)
		result -= align;
	return result;
}

/*
 * Directional first fit.
 *
 * Do a lineair search for free space, starting at addr in entry.
 * direction ==  1: search forward
 * direction == -1: search backward
 *
 * Output: low <= addr <= high and entry will contain addr.
 * 0 will be returned if no space is available.
 *
 * gap describes the space that must appear between the preceding entry.
 */
int
uvm_addr_linsearch(struct vm_map *map, struct uvm_addr_state *uaddr,
    struct vm_map_entry**entry_out, vaddr_t *addr_out,
    vaddr_t hint, vsize_t sz, vaddr_t align, vaddr_t offset,
    int direction, vaddr_t low, vaddr_t high,
    vsize_t before_gap, vsize_t after_gap)
{
	struct vm_map_entry	*entry;
	vaddr_t			 low_addr, high_addr;

	KASSERT(entry_out != NULL && addr_out != NULL);
	KASSERT(direction == -1 || direction == 1);
	KASSERT((hint & PAGE_MASK) == 0 && (high & PAGE_MASK) == 0 &&
	    (low & PAGE_MASK) == 0 &&
	    (before_gap & PAGE_MASK) == 0 && (after_gap & PAGE_MASK) == 0);
	KASSERT(high + sz > high); /* Check for overflow. */

	/*
	 * Hint magic.
	 */
	if (hint == 0)
		hint = (direction == 1 ? low : high);
	else if (hint > high) {
		if (direction != -1)
			return ENOMEM;
		hint = high;
	} else if (hint < low) {
		if (direction != 1)
			return ENOMEM;
		hint = low;
	}

	for (entry = uvm_map_entrybyaddr(&map->addr,
	    hint - (direction == -1 ? 1 : 0)); entry != NULL;
	    entry = (direction == 1 ?
	    RB_NEXT(uvm_map_addr, &map->addr, entry) :
	    RB_PREV(uvm_map_addr, &map->addr, entry))) {
		if (VMMAP_FREE_START(entry) > high ||
		    VMMAP_FREE_END(entry) < low) {
			break;
		}

		if (uvm_addr_fitspace(&low_addr, &high_addr,
		    MAX(low, VMMAP_FREE_START(entry)),
		    MIN(high, VMMAP_FREE_END(entry)),
		    sz, align, offset, before_gap, after_gap) == 0) {
			*entry_out = entry;
			if (hint >= low_addr && hint <= high_addr) {
				*addr_out = hint;
			} else {
				*addr_out = (direction == 1 ?
				    low_addr : high_addr);
			}
			return 0;
		}
	}

	return ENOMEM;
}

/*
 * Invoke address selector of uaddr.
 * uaddr may be NULL, in which case the algorithm will fail with ENOMEM.
 *
 * Will invoke uvm_addr_isavail to fill in last_out.
 */
int
uvm_addr_invoke(struct vm_map *map, struct uvm_addr_state *uaddr,
    struct vm_map_entry**entry_out, struct vm_map_entry**last_out,
    vaddr_t *addr_out,
    vsize_t sz, vaddr_t align, vaddr_t offset, vm_prot_t prot, vaddr_t hint)
{
	int error;

	if (uaddr == NULL)
		return ENOMEM;

	hint &= ~((vaddr_t)PAGE_MASK);
	if (hint != 0 &&
	    !(hint >= uaddr->uaddr_minaddr && hint < uaddr->uaddr_maxaddr))
		return ENOMEM;

	error = (*uaddr->uaddr_functions->uaddr_select)(map, uaddr,
	    entry_out, addr_out, sz, align, offset, prot, hint);

	if (error == 0) {
		KASSERT(*entry_out != NULL);
		*last_out = NULL;
		if (!uvm_map_isavail(map, uaddr, entry_out, last_out,
		    *addr_out, sz)) {
			panic("uvm_addr_invoke: address selector %p "
			    "(%s 0x%lx-0x%lx) "
			    "returned unavailable address 0x%lx",
			    uaddr, uaddr->uaddr_functions->uaddr_name,
			    uaddr->uaddr_minaddr, uaddr->uaddr_maxaddr,
			    *addr_out);
		}
	}

	return error;
}

#if defined(DEBUG) || defined(DDB)
void
uvm_addr_print(struct uvm_addr_state *uaddr, const char *slot, boolean_t full,
    int (*pr)(const char*, ...))
{
	if (uaddr == NULL) {
		(*pr)("- uvm_addr %s: NULL\n", slot);
		return;
	}

	(*pr)("- uvm_addr %s: %p (%s 0x%lx-0x%lx)\n", slot, uaddr,
	    uaddr->uaddr_functions->uaddr_name,
	    uaddr->uaddr_minaddr, uaddr->uaddr_maxaddr);
	if (uaddr->uaddr_functions->uaddr_print == NULL)
		return;

	(*uaddr->uaddr_functions->uaddr_print)(uaddr, full, pr);
}
#endif /* DEBUG || DDB */

/*
 * Destroy a uvm_addr_state structure.
 * The uaddr must have been previously allocated from uaddr_state_pool.
 */
void
uaddr_destroy(struct uvm_addr_state *uaddr)
{
	pool_put(&uaddr_pool, uaddr);
}


/*
 * Lineair allocator.
 * This allocator uses a first-fit algorithm.
 *
 * If hint is set, search will start at the hint position.
 * Only searches forward.
 */

const struct uvm_addr_functions uaddr_lin_functions = {
	.uaddr_select = &uaddr_lin_select,
	.uaddr_destroy = &uaddr_destroy,
	.uaddr_name = "uaddr_lin"
};

struct uvm_addr_state*
uaddr_lin_create(vaddr_t minaddr, vaddr_t maxaddr)
{
	struct uvm_addr_state* uaddr;

	uaddr = pool_get(&uaddr_pool, PR_WAITOK);
	uaddr->uaddr_minaddr = minaddr;
	uaddr->uaddr_maxaddr = maxaddr;
	uaddr->uaddr_functions = &uaddr_lin_functions;
	return uaddr;
}

int
uaddr_lin_select(struct vm_map *map, struct uvm_addr_state *uaddr,
    struct vm_map_entry**entry_out, vaddr_t *addr_out,
    vsize_t sz, vaddr_t align, vaddr_t offset,
    vm_prot_t prot, vaddr_t hint)
{
	vaddr_t guard_sz;

	/*
	 * Deal with guardpages: search for space with one extra page.
	 */
	guard_sz = ((map->flags & VM_MAP_GUARDPAGES) == 0 ? 0 : PAGE_SIZE);

	if (uaddr->uaddr_maxaddr - uaddr->uaddr_minaddr < sz + guard_sz)
		return ENOMEM;
	return uvm_addr_linsearch(map, uaddr, entry_out, addr_out, 0, sz,
	    align, offset, 1, uaddr->uaddr_minaddr, uaddr->uaddr_maxaddr - sz,
	    0, guard_sz);
}


/*
 * Randomized allocator.
 * This allocator use uvm_map_hint to acquire a random address and searches
 * from there.
 */

const struct uvm_addr_functions uaddr_rnd_functions = {
	.uaddr_select = &uaddr_rnd_select,
	.uaddr_free_insert = &uaddr_rnd_insert,
	.uaddr_free_remove = &uaddr_rnd_remove,
	.uaddr_destroy = &uaddr_rnd_destroy,
#if defined(DEBUG) || defined(DDB)
	.uaddr_print = &uaddr_rnd_print,
#endif /* DEBUG || DDB */
	.uaddr_name = "uaddr_rnd"
};

struct uvm_addr_state*
uaddr_rnd_create(vaddr_t minaddr, vaddr_t maxaddr)
{
	struct uaddr_rnd_state* uaddr;

	uaddr = pool_get(&uaddr_rnd_pool, PR_WAITOK);
	uaddr->ur_uaddr.uaddr_minaddr = minaddr;
	uaddr->ur_uaddr.uaddr_maxaddr = maxaddr;
	uaddr->ur_uaddr.uaddr_functions = &uaddr_rnd_functions;
	TAILQ_INIT(&uaddr->ur_free);
	return &uaddr->ur_uaddr;
}

int
uaddr_rnd_select(struct vm_map *map, struct uvm_addr_state *uaddr,
    struct vm_map_entry**entry_out, vaddr_t *addr_out,
    vsize_t sz, vaddr_t align, vaddr_t offset,
    vm_prot_t prot, vaddr_t hint)
{
	struct vmspace		*vm;
	vaddr_t			 guard_sz;
	vaddr_t			 low_addr, high_addr;
	struct vm_map_entry	*entry;
	vsize_t			 before_gap, after_gap;
	vaddr_t			 tmp;

	KASSERT((map->flags & VM_MAP_ISVMSPACE) != 0);
	vm = (struct vmspace*)map;

	/* Deal with guardpages: search for space with one extra page. */
	guard_sz = ((map->flags & VM_MAP_GUARDPAGES) == 0 ? 0 : PAGE_SIZE);

	/* Quick fail if the allocation won't fit. */
	if (uaddr->uaddr_maxaddr - uaddr->uaddr_minaddr < sz + guard_sz)
		return ENOMEM;

	/* Select a hint. */
	if (hint == 0)
		hint = uvm_map_hint(vm, prot);
	/* Clamp hint to uaddr range. */
	hint = MIN(MAX(hint, uaddr->uaddr_minaddr),
	    uaddr->uaddr_maxaddr - sz - guard_sz);

	/* Align hint to align,offset parameters. */
	tmp = hint;
	hint = uvm_addr_align_forward(tmp, align, offset);
	/* Check for overflow during alignment. */
	if (hint < tmp || hint > uaddr->uaddr_maxaddr - sz - guard_sz)
		return ENOMEM; /* Compatibility mode: never look backwards. */

	before_gap = 0;
	after_gap = guard_sz;

	/*
	 * Find the first entry at or after hint with free space.
	 *
	 * Since we need an entry that is on the free-list, search until
	 * we hit an entry that is owned by our uaddr.
	 */
	for (entry = uvm_map_entrybyaddr(&map->addr, hint);
	    entry != NULL &&
	    uvm_map_uaddr_e(map, entry) != uaddr;
	    entry = RB_NEXT(uvm_map_addr, &map->addr, entry)) {
		/* Fail if we search past uaddr_maxaddr. */
		if (VMMAP_FREE_START(entry) >= uaddr->uaddr_maxaddr) {
			entry = NULL;
			break;
		}
	}

	for ( /* initial entry filled in above */ ;
	    entry != NULL && VMMAP_FREE_START(entry) < uaddr->uaddr_maxaddr;
	    entry = TAILQ_NEXT(entry, dfree.tailq)) {
		if (uvm_addr_fitspace(&low_addr, &high_addr,
		    MAX(uaddr->uaddr_minaddr, VMMAP_FREE_START(entry)),
		    MIN(uaddr->uaddr_maxaddr, VMMAP_FREE_END(entry)),
		    sz, align, offset, before_gap, after_gap) == 0) {
			*entry_out = entry;
			if (hint >= low_addr && hint <= high_addr)
				*addr_out = hint;
			else
				*addr_out = low_addr;
			return 0;
		}
	}

	return ENOMEM;
}

/*
 * Destroy a uaddr_rnd_state structure.
 */
void
uaddr_rnd_destroy(struct uvm_addr_state *uaddr)
{
	pool_put(&uaddr_rnd_pool, uaddr);
}

/*
 * Add entry to tailq.
 */
void
uaddr_rnd_insert(struct vm_map *map, struct uvm_addr_state *uaddr_p,
    struct vm_map_entry *entry)
{
	struct uaddr_rnd_state	*uaddr;
	struct vm_map_entry	*prev;

	uaddr = (struct uaddr_rnd_state*)uaddr_p;
	KASSERT(entry == RB_FIND(uvm_map_addr, &map->addr, entry));

	/*
	 * Make prev the first vm_map_entry before entry.
	 */
	for (prev = RB_PREV(uvm_map_addr, &map->addr, entry);
	    prev != NULL;
	    prev = RB_PREV(uvm_map_addr, &map->addr, prev)) {
		/* Stop and fail when reaching uaddr minaddr. */
		if (VMMAP_FREE_START(prev) < uaddr_p->uaddr_minaddr) {
			prev = NULL;
			break;
		}

		KASSERT(prev->etype & UVM_ET_FREEMAPPED);
		if (uvm_map_uaddr_e(map, prev) == uaddr_p)
			break;
	}

	/* Perform insertion. */
	if (prev == NULL)
		TAILQ_INSERT_HEAD(&uaddr->ur_free, entry, dfree.tailq);
	else
		TAILQ_INSERT_AFTER(&uaddr->ur_free, prev, entry, dfree.tailq);
}

/*
 * Remove entry from tailq.
 */
void
uaddr_rnd_remove(struct vm_map *map, struct uvm_addr_state *uaddr_p,
    struct vm_map_entry *entry)
{
	struct uaddr_rnd_state	*uaddr;

	uaddr = (struct uaddr_rnd_state*)uaddr_p;
	TAILQ_REMOVE(&uaddr->ur_free, entry, dfree.tailq);
}

#if defined(DEBUG) || defined(DDB)
void
uaddr_rnd_print(struct uvm_addr_state *uaddr_p, boolean_t full,
    int (*pr)(const char*, ...))
{
	struct vm_map_entry	*entry;
	struct uaddr_rnd_state	*uaddr;
	vaddr_t			 addr;
	size_t			 count;
	vsize_t			 space;

	uaddr = (struct uaddr_rnd_state*)uaddr_p;
	addr = 0;
	count = 0;
	space = 0;
	TAILQ_FOREACH(entry, &uaddr->ur_free, dfree.tailq) {
		count++;
		space += entry->fspace;

		if (full) {
			(*pr)("\tentry %p: 0x%lx-0x%lx G=0x%lx F=0x%lx\n",
			    entry, entry->start, entry->end,
			    entry->guard, entry->fspace);
			(*pr)("\t\tfree: 0x%lx-0x%lx\n",
			    VMMAP_FREE_START(entry), VMMAP_FREE_END(entry));
		}
		if (entry->start < addr) {
			if (!full)
				(*pr)("\tentry %p: 0x%lx-0x%lx "
				    "G=0x%lx F=0x%lx\n",
				    entry, entry->start, entry->end,
				    entry->guard, entry->fspace);
			(*pr)("\t\tstart=0x%lx, expected at least 0x%lx\n",
			    entry->start, addr);
		}

		addr = VMMAP_FREE_END(entry);
	}
	(*pr)("\t0x%lu entries, 0x%lx free bytes\n", count, space);
}
#endif /* DEBUG || DDB */


/*
 * An allocator that selects an address within distance of the hint.
 *
 * If no hint is given, the allocator refuses to allocate.
 */

const struct uvm_addr_functions uaddr_hint_functions = {
	.uaddr_select = &uaddr_hint_select,
	.uaddr_destroy = &uaddr_hint_destroy,
	.uaddr_name = "uaddr_hint"
};

/*
 * Create uaddr_hint state.
 */
struct uvm_addr_state*
uaddr_hint_create(vaddr_t minaddr, vaddr_t maxaddr, vsize_t max_dist)
{
	struct uaddr_hint_state* ua_hint;

	KASSERT(uaddr_hint_pool.pr_size == sizeof(*ua_hint));

	ua_hint = pool_get(&uaddr_hint_pool, PR_WAITOK);
	ua_hint->uaddr.uaddr_minaddr = minaddr;
	ua_hint->uaddr.uaddr_maxaddr = maxaddr;
	ua_hint->uaddr.uaddr_functions = &uaddr_hint_functions;
	ua_hint->max_dist = max_dist;
	return &ua_hint->uaddr;
}

/*
 * Destroy uaddr_hint state.
 */
void
uaddr_hint_destroy(struct uvm_addr_state *uaddr)
{
	pool_put(&uaddr_hint_pool, uaddr);
}

/*
 * Hint selector.
 *
 * Attempts to find an address that is within max_dist of the hint.
 */
int
uaddr_hint_select(struct vm_map *map, struct uvm_addr_state *uaddr_param,
    struct vm_map_entry**entry_out, vaddr_t *addr_out,
    vsize_t sz, vaddr_t align, vaddr_t offset,
    vm_prot_t prot, vaddr_t hint)
{
	struct uaddr_hint_state	*uaddr = (struct uaddr_hint_state*)uaddr_param;
	vsize_t			 before_gap, after_gap;
	vaddr_t			 low, high;
	int			 dir;

	if (hint == 0)
		return ENOMEM;

	/*
	 * Calculate upper and lower bound for selected address.
	 */
	high = hint + uaddr->max_dist;
	if (high < hint)	/* overflow */
		high = map->max_offset;
	high = MIN(high, uaddr->uaddr.uaddr_maxaddr);
	high -= sz;

	/* Calculate lower bound for selected address. */
	low = hint - uaddr->max_dist;
	if (low > hint)		/* underflow */
		low = map->min_offset;
	low = MAX(low, uaddr->uaddr.uaddr_minaddr);

	/* Search strategy setup. */
	before_gap = PAGE_SIZE +
	    (arc4random_uniform(UADDR_HINT_MAXGAP) & ~(vaddr_t)PAGE_MASK);
	after_gap = PAGE_SIZE +
	    (arc4random_uniform(UADDR_HINT_MAXGAP) & ~(vaddr_t)PAGE_MASK);
	dir = (arc4random() & 0x01) ? 1 : -1;

	/*
	 * Try to search:
	 * - forward,  with gap
	 * - backward, with gap
	 * - forward,  without gap
	 * - backward, without gap
	 * (Where forward is in the direction specified by dir and
	 * backward is in the direction specified by -dir).
	 */
	if (uvm_addr_linsearch(map, uaddr_param,
	    entry_out, addr_out, hint, sz, align, offset,
	    dir, low, high, before_gap, after_gap) == 0)
		return 0;
	if (uvm_addr_linsearch(map, uaddr_param,
	    entry_out, addr_out, hint, sz, align, offset,
	    -dir, low, high, before_gap, after_gap) == 0)
		return 0;

	if (uvm_addr_linsearch(map, uaddr_param,
	    entry_out, addr_out, hint, sz, align, offset,
	    dir, low, high, 0, 0) == 0)
		return 0;
	if (uvm_addr_linsearch(map, uaddr_param,
	    entry_out, addr_out, hint, sz, align, offset,
	    -dir, low, high, 0, 0) == 0)
		return 0;

	return ENOMEM;
}

/*
 * Kernel allocation bootstrap logic.
 */

const struct uvm_addr_functions uaddr_kernel_functions = {
	.uaddr_select = &uaddr_kbootstrap_select,
	.uaddr_destroy = &uaddr_kbootstrap_destroy,
	.uaddr_name = "uaddr_kbootstrap"
};

/*
 * Select an address from the map.
 *
 * This function ignores the uaddr spec and instead uses the map directly.
 * Because of that property, the uaddr algorithm can be shared across all
 * kernel maps.
 */
int
uaddr_kbootstrap_select(struct vm_map *map, struct uvm_addr_state *uaddr,
    struct vm_map_entry **entry_out, vaddr_t *addr_out,
    vsize_t sz, vaddr_t align, vaddr_t offset, vm_prot_t prot, vaddr_t hint)
{
	vaddr_t tmp;

	RB_FOREACH(*entry_out, uvm_map_addr, &map->addr) {
		if (VMMAP_FREE_END(*entry_out) <= uvm_maxkaddr &&
		    uvm_addr_fitspace(addr_out, &tmp,
		    VMMAP_FREE_START(*entry_out), VMMAP_FREE_END(*entry_out),
		    sz, align, offset, 0, 0) == 0)
			return 0;
	}

	return ENOMEM;
}

/*
 * Don't destroy the kernel bootstrap allocator.
 */
void
uaddr_kbootstrap_destroy(struct uvm_addr_state *uaddr)
{
	KASSERT(uaddr == (struct uvm_addr_state*)&uaddr_kbootstrap);
}

/*
 * Best fit algorithm.
 */

const struct uvm_addr_functions uaddr_bestfit_functions = {
	.uaddr_select = &uaddr_bestfit_select,
	.uaddr_free_insert = &uaddr_bestfit_insert,
	.uaddr_free_remove = &uaddr_bestfit_remove,
	.uaddr_destroy = &uaddr_bestfit_destroy,
	.uaddr_name = "uaddr_bestfit"
};

struct uvm_addr_state*
uaddr_bestfit_create(vaddr_t minaddr, vaddr_t maxaddr)
{
	struct uaddr_bestfit_state *uaddr;

	uaddr = pool_get(&uaddr_bestfit_pool, PR_WAITOK);
	uaddr->ubf_uaddr.uaddr_minaddr = minaddr;
	uaddr->ubf_uaddr.uaddr_maxaddr = maxaddr;
	uaddr->ubf_uaddr.uaddr_functions = &uaddr_bestfit_functions;
	RB_INIT(&uaddr->ubf_free);
	return &uaddr->ubf_uaddr;
}

void
uaddr_bestfit_destroy(struct uvm_addr_state *uaddr)
{
	pool_put(&uaddr_bestfit_pool, uaddr);
}

void
uaddr_bestfit_insert(struct vm_map *map, struct uvm_addr_state *uaddr_p,
    struct vm_map_entry *entry)
{
	struct uaddr_bestfit_state	*uaddr;
	struct vm_map_entry		*rb_rv;

	uaddr = (struct uaddr_bestfit_state*)uaddr_p;
	if ((rb_rv = RB_INSERT(uaddr_free_rbtree, &uaddr->ubf_free, entry)) !=
	    NULL) {
		panic("%s: duplicate insertion: state %p "
		    "interting %p, colliding with %p", __func__,
		    uaddr, entry, rb_rv);
	}
}

void
uaddr_bestfit_remove(struct vm_map *map, struct uvm_addr_state *uaddr_p,
    struct vm_map_entry *entry)
{
	struct uaddr_bestfit_state	*uaddr;

	uaddr = (struct uaddr_bestfit_state*)uaddr_p;
	if (RB_REMOVE(uaddr_free_rbtree, &uaddr->ubf_free, entry) != entry)
		panic("%s: entry was not in tree", __func__);
}

int
uaddr_bestfit_select(struct vm_map *map, struct uvm_addr_state *uaddr_p,
    struct vm_map_entry**entry_out, vaddr_t *addr_out,
    vsize_t sz, vaddr_t align, vaddr_t offset,
    vm_prot_t prot, vaddr_t hint)
{
	vaddr_t				 min, max;
	struct uaddr_bestfit_state	*uaddr;
	struct vm_map_entry		*entry;
	vsize_t				 guardsz;

	uaddr = (struct uaddr_bestfit_state*)uaddr_p;
	guardsz = ((map->flags & VM_MAP_GUARDPAGES) ? PAGE_SIZE : 0);

	/*
	 * Find smallest item on freelist capable of holding item.
	 * Deal with guardpages: search for space with one extra page.
	 */
	entry = uvm_addr_entrybyspace(&uaddr->ubf_free, sz + guardsz);
	if (entry == NULL)
		return ENOMEM;

	/*
	 * Walk the tree until we find an entry that fits.
	 */
	while (uvm_addr_fitspace(&min, &max,
	    VMMAP_FREE_START(entry), VMMAP_FREE_END(entry),
	    sz, align, offset, 0, guardsz) != 0) {
		entry = RB_NEXT(uaddr_free_rbtree, &uaddr->ubf_free, entry);
		if (entry == NULL)
			return ENOMEM;
	}

	/*
	 * Return the address that generates the least fragmentation.
	 */
	*entry_out = entry;
	*addr_out = (min - VMMAP_FREE_START(entry) <=
	    VMMAP_FREE_END(entry) - guardsz - sz - max ?
	    min : max);
	return 0;
}


/*
 * A userspace allocator based on pivots.
 */

const struct uvm_addr_functions uaddr_pivot_functions = {
	.uaddr_select = &uaddr_pivot_select,
	.uaddr_free_insert = &uaddr_pivot_insert,
	.uaddr_free_remove = &uaddr_pivot_remove,
	.uaddr_destroy = &uaddr_pivot_destroy,
#if defined(DEBUG) || defined(DDB)
	.uaddr_print = &uaddr_pivot_print,
#endif /* DEBUG || DDB */
	.uaddr_name = "uaddr_pivot"
};

/*
 * A special random function for pivots.
 *
 * This function will return:
 * - a random number
 * - a multiple of PAGE_SIZE
 * - at least PAGE_SIZE
 *
 * The random function has a slightly higher change to return a small number.
 */
vsize_t
uaddr_pivot_random()
{
	int			r;

	/*
	 * The sum of two six-sided dice will have a normal distribution.
	 * We map the highest probable number to 1, by folding the curve
	 * (think of a graph on a piece of paper, that you fold).
	 *
	 * Because the fold happens at PIVOT_RND - 1, the numbers 0 and 1
	 * have the same and highest probability of happening.
	 */
	r = arc4random_uniform(PIVOT_RND) + arc4random_uniform(PIVOT_RND) -
	    (PIVOT_RND - 1);
	if (r < 0)
		r = -r;

	/*
	 * Make the returned value at least PAGE_SIZE and a multiple of
	 * PAGE_SIZE.
	 */
	return (vaddr_t)(1 + r) << PAGE_SHIFT;
}

/*
 * Select a new pivot.
 *
 * A pivot must:
 * - be chosen random
 * - have a randomly chosen gap before it, where the uaddr_state starts
 * - have a randomly chosen gap after it, before the uaddr_state ends
 *
 * Furthermore, the pivot must provide sufficient space for the allocation.
 * The addr will be set to the selected address.
 *
 * Returns ENOMEM on failure.
 */
int
uaddr_pivot_newpivot(struct vm_map *map, struct uaddr_pivot_state *uaddr,
    struct uaddr_pivot *pivot,
    struct vm_map_entry**entry_out, vaddr_t *addr_out,
    vsize_t sz, vaddr_t align, vaddr_t offset,
    vsize_t before_gap, vsize_t after_gap)
{
	struct vm_map_entry		*entry, *found;
	vaddr_t				 minaddr, maxaddr;
	vsize_t				 dist;
	vaddr_t				 found_minaddr, found_maxaddr;
	vaddr_t				 min, max;
	vsize_t				 arc4_arg;
	int				 fit_error;
	u_int32_t			 path;

	minaddr = uaddr->up_uaddr.uaddr_minaddr;
	maxaddr = uaddr->up_uaddr.uaddr_maxaddr;
	KASSERT(minaddr < maxaddr);
#ifdef DIAGNOSTIC
	if (minaddr + 2 * PAGE_SIZE > maxaddr) {
		panic("uaddr_pivot_newpivot: cannot grant random pivot "
		    "in area less than 2 pages (size = 0x%lx)",
		    maxaddr - minaddr);
	}
#endif /* DIAGNOSTIC */

	/*
	 * Gap calculation: 1/32 of the size of the managed area.
	 *
	 * At most: sufficient to not get truncated at arc4random.
	 * At least: 2 PAGE_SIZE
	 *
	 * minaddr and maxaddr will be changed according to arc4random.
	 */
	dist = MAX((maxaddr - minaddr) / 32, 2 * (vaddr_t)PAGE_SIZE);
	if (dist >> PAGE_SHIFT > 0xffffffff) {
		minaddr += (vsize_t)arc4random() << PAGE_SHIFT;
		maxaddr -= (vsize_t)arc4random() << PAGE_SHIFT;
	} else {
		minaddr += (vsize_t)arc4random_uniform(dist >> PAGE_SHIFT) <<
		    PAGE_SHIFT;
		maxaddr -= (vsize_t)arc4random_uniform(dist >> PAGE_SHIFT) <<
		    PAGE_SHIFT;
	}

	/*
	 * A very fast way to find an entry that will be large enough
	 * to hold the allocation, but still is found more or less
	 * randomly: the tree path selector has a 50% chance to go for
	 * a bigger or smaller entry.
	 *
	 * Note that the memory may actually be available,
	 * but the fragmentation may be so bad and the gaps chosen
	 * so unfortunately, that the allocation will not succeed.
	 * Or the alignment can only be satisfied by an entry that
	 * is not visited in the randomly selected path.
	 *
	 * This code finds an entry with sufficient space in O(log n) time.
	 */
	path = arc4random();
	found = NULL;
	entry = RB_ROOT(&uaddr->up_free);
	while (entry != NULL) {
		fit_error = uvm_addr_fitspace(&min, &max,
		    MAX(VMMAP_FREE_START(entry), minaddr),
		    MIN(VMMAP_FREE_END(entry), maxaddr),
		    sz, align, offset, before_gap, after_gap);

		/* It fits, save this entry. */
		if (fit_error == 0) {
			found = entry;
			found_minaddr = min;
			found_maxaddr = max;
		}

		/* Next. */
		if (fit_error != 0)
			entry = RB_RIGHT(entry, dfree.rbtree);
		else if	((path & 0x1) == 0) {
			path >>= 1;
			entry = RB_RIGHT(entry, dfree.rbtree);
		} else {
			path >>= 1;
			entry = RB_LEFT(entry, dfree.rbtree);
		}
	}
	if (found == NULL)
		return ENOMEM;	/* Not found a large enough region. */

	/*
	 * Calculate a random address within found.
	 *
	 * found_minaddr and found_maxaddr are already aligned, so be sure
	 * to select a multiple of align as the offset in the entry.
	 * Preferably, arc4random_uniform is used to provide no bias within
	 * the entry.
	 * However if the size of the entry exceeds arc4random_uniforms
	 * argument limit, we simply use arc4random (thus limiting ourselves
	 * to 4G * PAGE_SIZE bytes offset).
	 */
	if (found_maxaddr == found_minaddr)
		*addr_out = found_minaddr;
	else {
		KASSERT(align >= PAGE_SIZE && (align & (align - 1)) == 0);
		arc4_arg = found_maxaddr - found_minaddr;
		if (arc4_arg > 0xffffffff) {
			*addr_out = found_minaddr +
			    (arc4random() & (align - 1));
		} else {
			*addr_out = found_minaddr +
			    (arc4random_uniform(arc4_arg) & (align - 1));
		}
	}
	/* Address was found in this entry. */
	*entry_out = found;

	/*
	 * Set up new pivot and return selected address.
	 *
	 * Depending on the direction of the pivot, the pivot must be placed
	 * at the bottom or the top of the allocation:
	 * - if the pivot moves upwards, place the pivot at the top of the
	 *   allocation,
	 * - if the pivot moves downwards, place the pivot at the bottom
	 *   of the allocation.
	 */
	pivot->entry = found;
	pivot->dir = (arc4random() & 0x1 ? 1 : -1);
	if (pivot->dir > 0)
		pivot->addr = *addr_out + sz;
	else
		pivot->addr = *addr_out;
	pivot->expire = PIVOT_EXPIRE - 1; /* First use is right now. */
	return 0;
}

/*
 * Pivot selector.
 *
 * Each time the selector is invoked, it will select a random pivot, which
 * it will use to select memory with. The memory will be placed at the pivot,
 * with a randomly sized gap between the allocation and the pivot.
 * The pivot will then move so it will never revisit this address.
 *
 * Each allocation, the pivot expiry timer ticks. Once the pivot becomes
 * expired, it will be replaced with a newly created pivot. Pivots also
 * automatically expire if they fail to provide memory for an allocation.
 *
 * Expired pivots are replaced using the uaddr_pivot_newpivot() function,
 * which will ensure the pivot points at memory in such a way that the
 * allocation will succeed.
 * As an added bonus, the uaddr_pivot_newpivot() function will perform the
 * allocation immediately and move the pivot as appropriate.
 *
 * If uaddr_pivot_newpivot() fails to find a new pivot that will allow the
 * allocation to succeed, it will not create a new pivot and the allocation
 * will fail.
 *
 * A pivot running into used memory will automatically expire (because it will
 * fail to allocate).
 *
 * Characteristics of the allocator:
 * - best case, an allocation is O(log N)
 *   (it would be O(1), if it werent for the need to check if the memory is
 *   free; although that can be avoided...)
 * - worst case, an allocation is O(log N)
 *   (the uaddr_pivot_newpivot() function has that complexity)
 * - failed allocations always take O(log N)
 *   (the uaddr_pivot_newpivot() function will walk that deep into the tree).
 */
int
uaddr_pivot_select(struct vm_map *map, struct uvm_addr_state *uaddr_p,
    struct vm_map_entry**entry_out, vaddr_t *addr_out,
    vsize_t sz, vaddr_t align, vaddr_t offset,
    vm_prot_t prot, vaddr_t hint)
{
	struct uaddr_pivot_state	*uaddr;
	struct vm_map_entry		*entry;
	struct uaddr_pivot		*pivot;
	vaddr_t				 min, max;
	vsize_t				 before_gap, after_gap;
	int				 err;

	/* Hint must be handled by dedicated hint allocator. */
	if (hint != 0)
		return EINVAL;

	/*
	 * Select a random pivot and a random gap sizes around the allocation.
	 */
	uaddr = (struct uaddr_pivot_state*)uaddr_p;
	pivot = &uaddr->up_pivots[
	    arc4random_uniform(nitems(uaddr->up_pivots))];
	before_gap = uaddr_pivot_random();
	after_gap = uaddr_pivot_random();
	if (pivot->addr == 0 || pivot->entry == NULL || pivot->expire == 0)
		goto expired;	/* Pivot is invalid (null or expired). */

	/*
	 * Attempt to use the pivot to map the entry.
	 */
	entry = pivot->entry;
	if (pivot->dir > 0) {
		if (uvm_addr_fitspace(&min, &max,
		    MAX(VMMAP_FREE_START(entry), pivot->addr),
		    VMMAP_FREE_END(entry), sz, align, offset,
		    before_gap, after_gap) == 0) {
			*addr_out = min;
			*entry_out = entry;
			pivot->addr = min + sz;
			pivot->expire--;
			return 0;
		}
	} else {
		if (uvm_addr_fitspace(&min, &max,
		    VMMAP_FREE_START(entry),
		    MIN(VMMAP_FREE_END(entry), pivot->addr),
		    sz, align, offset, before_gap, after_gap) == 0) {
			*addr_out = max;
			*entry_out = entry;
			pivot->addr = max;
			pivot->expire--;
			return 0;
		}
	}

expired:
	/*
	 * Pivot expired or allocation failed.
	 * Use pivot selector to do the allocation and find a new pivot.
	 */
	err = uaddr_pivot_newpivot(map, uaddr, pivot, entry_out, addr_out,
	    sz, align, offset, before_gap, after_gap);
	return err;
}

/*
 * Free the pivot.
 */
void
uaddr_pivot_destroy(struct uvm_addr_state *uaddr)
{
	pool_put(&uaddr_pivot_pool, uaddr);
}

/*
 * Insert an entry with free space in the space tree.
 */
void
uaddr_pivot_insert(struct vm_map *map, struct uvm_addr_state *uaddr_p,
    struct vm_map_entry *entry)
{
	struct uaddr_pivot_state	*uaddr;
	struct vm_map_entry		*rb_rv;
	struct uaddr_pivot		*p;
	vaddr_t				 check_addr;
	vaddr_t				 start, end;

	uaddr = (struct uaddr_pivot_state*)uaddr_p;
	if ((rb_rv = RB_INSERT(uaddr_free_rbtree, &uaddr->up_free, entry)) !=
	    NULL) {
		panic("%s: duplicate insertion: state %p "
		    "inserting entry %p which collides with %p", __func__,
		    uaddr, entry, rb_rv);
	}

	start = VMMAP_FREE_START(entry);
	end = VMMAP_FREE_END(entry);

	/*
	 * Update all pivots that are contained in this entry.
	 */
	for (p = &uaddr->up_pivots[0];
	    p != &uaddr->up_pivots[nitems(uaddr->up_pivots)]; p++) {
		check_addr = p->addr;
		if (check_addr == 0)
			continue;
		if (p->dir < 0)
			check_addr--;

		if (start <= check_addr &&
		    check_addr < end) {
			KASSERT(p->entry == NULL);
			p->entry = entry;
		}
	}
}

/*
 * Remove an entry with free space from the space tree.
 */
void
uaddr_pivot_remove(struct vm_map *map, struct uvm_addr_state *uaddr_p,
    struct vm_map_entry *entry)
{
	struct uaddr_pivot_state	*uaddr;
	struct uaddr_pivot		*p;

	uaddr = (struct uaddr_pivot_state*)uaddr_p;
	if (RB_REMOVE(uaddr_free_rbtree, &uaddr->up_free, entry) != entry)
		panic("%s: entry was not in tree", __func__);

	/*
	 * Inform any pivot with this entry that the entry is gone.
	 * Note that this does not automatically invalidate the pivot.
	 */
	for (p = &uaddr->up_pivots[0];
	    p != &uaddr->up_pivots[nitems(uaddr->up_pivots)]; p++) {
		if (p->entry == entry)
			p->entry = NULL;
	}
}

/*
 * Create a new pivot selector.
 *
 * Initially, all pivots are in the expired state.
 * Two reasons for this:
 * - it means this allocator will not take a huge amount of time
 * - pivots select better on demand, because the pivot selection will be
 *   affected by preceding allocations:
 *   the next pivots will likely end up in different segments of free memory,
 *   that was segmented by an earlier allocation; better spread.
 */
struct uvm_addr_state*
uaddr_pivot_create(vaddr_t minaddr, vaddr_t maxaddr)
{
	struct uaddr_pivot_state *uaddr;

	uaddr = pool_get(&uaddr_pivot_pool, PR_WAITOK);
	uaddr->up_uaddr.uaddr_minaddr = minaddr;
	uaddr->up_uaddr.uaddr_maxaddr = maxaddr;
	uaddr->up_uaddr.uaddr_functions = &uaddr_pivot_functions;
	RB_INIT(&uaddr->up_free);
	bzero(uaddr->up_pivots, sizeof(uaddr->up_pivots));

	return &uaddr->up_uaddr;
}

#if defined(DEBUG) || defined(DDB)
/*
 * Print the uaddr_pivot_state.
 *
 * If full, a listing of all entries in the state will be provided.
 */
void
uaddr_pivot_print(struct uvm_addr_state *uaddr_p, boolean_t full,
    int (*pr)(const char*, ...))
{
	struct uaddr_pivot_state	*uaddr;
	struct uaddr_pivot		*pivot;
	struct vm_map_entry		*entry;
	int				 i;
	vaddr_t				 check_addr;

	uaddr = (struct uaddr_pivot_state*)uaddr_p;

	for (i = 0; i < NUM_PIVOTS; i++) {
		pivot = &uaddr->up_pivots[i];

		(*pr)("\tpivot 0x%lx, epires in %d, direction %d\n",
		    pivot->addr, pivot->expire, pivot->dir);
	}
	if (!full)
		return;

	if (RB_EMPTY(&uaddr->up_free))
		(*pr)("\tempty\n");
	/* Print list of free space. */
	RB_FOREACH(entry, uaddr_free_rbtree, &uaddr->up_free) {
		(*pr)("\t0x%lx - 0x%lx free (0x%lx bytes)\n",
		    VMMAP_FREE_START(entry), VMMAP_FREE_END(entry),
		    VMMAP_FREE_END(entry) - VMMAP_FREE_START(entry));

		for (i = 0; i < NUM_PIVOTS; i++) {
			pivot = &uaddr->up_pivots[i];
			check_addr = pivot->addr;
			if (check_addr == 0)
				continue;
			if (pivot->dir < 0)
				check_addr--;

			if (VMMAP_FREE_START(entry) <= check_addr &&
			    check_addr < VMMAP_FREE_END(entry)) {
				(*pr)("\t\tcontains pivot %d (0x%lx)\n",
				    i, pivot->addr);
			}
		}
	}
}
#endif /* DEBUG || DDB */

/*
 * Strategy for uaddr_stack_brk_select.
 */
struct uaddr_bs_strat {
	vaddr_t			start;		/* Start of area. */
	vaddr_t			end;		/* End of area. */
	int			dir;		/* Search direction. */
};

/*
 * Stack/break allocator.
 *
 * Stack area is grown into in the opposite direction of the stack growth,
 * brk area is grown downward (because sbrk() grows upward).
 *
 * Both areas are grown into proportially: a weighted chance is used to
 * select which one (stack or brk area) to try. If the allocation fails,
 * the other one is tested.
 */

const struct uvm_addr_functions uaddr_stack_brk_functions = {
	.uaddr_select = &uaddr_stack_brk_select,
	.uaddr_destroy = &uaddr_destroy,
	.uaddr_name = "uaddr_stckbrk"
};

/*
 * Stack/brk address selector.
 */
int
uaddr_stack_brk_select(struct vm_map *map, struct uvm_addr_state *uaddr,
    struct vm_map_entry**entry_out, vaddr_t *addr_out,
    vsize_t sz, vaddr_t align, vaddr_t offset,
    vm_prot_t prot, vaddr_t hint)
{
	vsize_t			before_gap, after_gap;
	int			stack_idx, brk_idx;
	struct uaddr_bs_strat	strat[2], *s;
	vsize_t			sb_size;

	/*
	 * Choose gap size and if the stack is searched before or after the
	 * brk area.
	 */
	before_gap = ((arc4random() & 0x3) + 1) << PAGE_SHIFT;
	after_gap = ((arc4random() & 0x3) + 1) << PAGE_SHIFT;

	sb_size = (map->s_end - map->s_start) + (map->b_end - map->b_start);
	sb_size >>= PAGE_SHIFT;
	if (arc4random_uniform(MAX(sb_size, 0xffffffff)) >
	    map->b_end - map->b_start) {
		brk_idx = 1;
		stack_idx = 0;
	} else {
		brk_idx = 0;
		stack_idx = 1;
	}

	/*
	 * Set up stack search strategy.
	 */
	s = &strat[stack_idx];
	s->start = MAX(map->s_start, uaddr->uaddr_minaddr);
	s->end = MIN(map->s_end, uaddr->uaddr_maxaddr);
#ifdef MACHINE_STACK_GROWS_UP
	s->dir = -1;
#else
	s->dir =  1;
#endif

	/*
	 * Set up brk search strategy.
	 */
	s = &strat[brk_idx];
	s->start = MAX(map->b_start, uaddr->uaddr_minaddr);
	s->end = MIN(map->b_end, uaddr->uaddr_maxaddr);
	s->dir = -1;	/* Opposite of brk() growth. */

	/*
	 * Linear search for space.
	 */
	for (s = &strat[0]; s < &strat[nitems(strat)]; s++) {
		if (uvm_addr_linsearch(map, uaddr, entry_out, addr_out,
		    0, sz, align, offset, s->dir, s->start, s->end,
		    before_gap, after_gap) == 0)
			return 0;
	}

	return ENOMEM;
}

struct uvm_addr_state*
uaddr_stack_brk_create(vaddr_t minaddr, vaddr_t maxaddr)
{
	struct uvm_addr_state* uaddr;

	uaddr = pool_get(&uaddr_pool, PR_WAITOK);
	uaddr->uaddr_minaddr = minaddr;
	uaddr->uaddr_maxaddr = maxaddr;
	uaddr->uaddr_functions = &uaddr_stack_brk_functions;
	return uaddr;
}


RB_GENERATE(uaddr_free_rbtree, vm_map_entry, dfree.rbtree,
    uvm_mapent_fspace_cmp);
