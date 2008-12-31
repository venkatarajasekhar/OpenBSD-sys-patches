/*        $NetBSD: dm_target.c,v 1.4 2008/12/21 00:59:39 haad Exp $      */

/*
 * Copyright (c) 2008 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Adam Hamsik.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code Must retain the above copyright
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

#include <sys/types.h>
#include <sys/param.h>

#include <sys/kmem.h>

#include "netbsd-dm.h"
#include "dm.h"

static dm_target_t* dm_target_lookup_name(const char *);

TAILQ_HEAD(dm_target_head, dm_target);

static struct dm_target_head dm_target_list =
TAILQ_HEAD_INITIALIZER(dm_target_list);

kmutex_t dm_target_mutex;

/*
 * Called indirectly from dm_table_load_ioct to mark target as used.
 */
void
dm_target_busy(dm_target_t *target)
{
	target->ref_cnt++;	
}

void
dm_target_unbusy(dm_target_t *target)
{
	target->ref_cnt--;
}

dm_target_t *
dm_target_lookup(const char *dm_target_name)
{
	dm_target_t *dmt;

	dmt = NULL;

	mutex_enter(&dm_target_mutex);

	if (dm_target_name != NULL)
		dmt = dm_target_lookup_name(dm_target_name);

	if (dmt != NULL)
		dm_target_busy(dmt);
	
	mutex_exit(&dm_target_mutex);
	
	return dmt;	
}
	
/*
 * Search for name in TAIL and return apropriate pointer.
 */
static dm_target_t*
dm_target_lookup_name(const char *dm_target_name)
{
	dm_target_t *dm_target;
        int dlen; int slen;

	slen = strlen(dm_target_name) + 1;

	TAILQ_FOREACH(dm_target, &dm_target_list, dm_target_next) {
		dlen = strlen(dm_target->name) + 1;

		if (dlen != slen)
			continue;
		
		if (strncmp(dm_target_name, dm_target->name, slen) == 0){
			return dm_target;
		}
	}

	return NULL;
}

/*
 * Insert new target struct into the TAIL.
 * dm_target
 *   contains name, version, function pointer to specifif target functions.
 */
int
dm_target_insert(dm_target_t *dm_target)
{
	dm_target_t *dmt;
	
	mutex_enter(&dm_target_mutex);

	dmt = dm_target_lookup_name(dm_target->name);
	if (dmt != NULL) {
		mutex_exit(&dm_target_mutex);
		return EEXIST;
	}
		
	TAILQ_INSERT_TAIL(&dm_target_list, dm_target, dm_target_next);

	mutex_exit(&dm_target_mutex);
	
	return 0;
}


/*
 * Remove target from TAIL, target is selected with it's name.
 */
int
dm_target_rem(char *dm_target_name)
{
	dm_target_t *dmt;
	
	KASSERT(dm_target_name != NULL);

	mutex_enter(&dm_target_mutex);
	
	dmt = dm_target_lookup_name(dm_target_name);
	if (dmt == NULL) {
		mutex_exit(&dm_target_mutex);
		return ENOENT;
	}
		
	if (dmt->ref_cnt > 0) {
		mutex_exit(&dm_target_mutex);
		return EBUSY;
	}
	
	TAILQ_REMOVE(&dm_target_list,
	    dmt, dm_target_next);

	mutex_exit(&dm_target_mutex);
	
	(void)kmem_free(dmt, sizeof(dm_target_t));

	return 0;
}

/*
 * Destroy all targets and remove them from queue.
 * This routine is called from dm_detach, before module
 * is unloaded.
 */

int
dm_target_destroy(void)
{
	dm_target_t *dm_target;

	mutex_enter(&dm_target_mutex);
	while (TAILQ_FIRST(&dm_target_list) != NULL){

		dm_target = TAILQ_FIRST(&dm_target_list);
		
		TAILQ_REMOVE(&dm_target_list, TAILQ_FIRST(&dm_target_list),
		dm_target_next);
		
		(void)kmem_free(dm_target, sizeof(dm_target_t));
	}
	mutex_exit(&dm_target_mutex);
	
	mutex_destroy(&dm_target_mutex);
	
	return 0;
}

/*
 * Allocate new target entry.
 */
dm_target_t*
dm_target_alloc(const char *name)
{
	return kmem_zalloc(sizeof(dm_target_t), KM_NOSLEEP);
}

/*
 * Return prop_array of dm_target dictionaries.
 */
prop_array_t
dm_target_prop_list(void)
{
	prop_array_t target_array,ver;
	prop_dictionary_t target_dict;
	dm_target_t *dm_target;

	size_t i;

	target_array = prop_array_create();

	mutex_enter(&dm_target_mutex);
	
	TAILQ_FOREACH (dm_target, &dm_target_list, dm_target_next){

		target_dict  = prop_dictionary_create();
		ver = prop_array_create();
		prop_dictionary_set_cstring(target_dict, DM_TARGETS_NAME,
		    dm_target->name);

		for (i = 0; i < 3; i++)
			prop_array_add_uint32(ver, dm_target->version[i]);

		prop_dictionary_set(target_dict, DM_TARGETS_VERSION, ver);
		prop_array_add(target_array, target_dict);

		prop_object_release(ver);
		prop_object_release(target_dict);
	}

	mutex_exit(&dm_target_mutex);
	
	return target_array;
}

/* Initialize dm_target subsystem. */
int
dm_target_init(void)
{
	dm_target_t *dmt,*dmt3,*dmt4;
	int r;

	r = 0;

	mutex_init(&dm_target_mutex, MUTEX_DEFAULT, IPL_NONE);
	
	dmt = dm_target_alloc("linear");
	dmt3 = dm_target_alloc("striped");
	dmt4 = dm_target_alloc("mirror");
	
	dmt->version[0] = 1;
	dmt->version[1] = 0;
	dmt->version[2] = 2;
	strlcpy(dmt->name, "linear", DM_MAX_TYPE_NAME);
	dmt->init = &dm_target_linear_init;
	dmt->status = &dm_target_linear_status;
	dmt->strategy = &dm_target_linear_strategy;
	dmt->deps = &dm_target_linear_deps;
	dmt->destroy = &dm_target_linear_destroy;
	dmt->upcall = &dm_target_linear_upcall;
	
	r = dm_target_insert(dmt);
		
/*	dmt1->version[0] = 1;
	dmt1->version[1] = 0;
	dmt1->version[2] = 0;
	strlcpy(dmt1->name, "zero", DM_MAX_TYPE_NAME);
	dmt1->init = &dm_target_zero_init;
	dmt1->status = &dm_target_zero_status;
	dmt1->strategy = &dm_target_zero_strategy;
	dmt1->deps = &dm_target_zero_deps; 
	dmt1->destroy = &dm_target_zero_destroy; 
	dmt1->upcall = &dm_target_zero_upcall;
	
	r = dm_target_insert(dmt1);

	dmt2->version[0] = 1;
	dmt2->version[1] = 0;
	dmt2->version[2] = 0;
	strlcpy(dmt2->name, "error", DM_MAX_TYPE_NAME);
	dmt2->init = &dm_target_error_init;
	dmt2->status = &dm_target_error_status;
	dmt2->strategy = &dm_target_error_strategy;
	dmt2->deps = &dm_target_error_deps; 
	dmt2->destroy = &dm_target_error_destroy; 
	dmt2->upcall = &dm_target_error_upcall;
	
	r = dm_target_insert(dmt2);*/
	
	dmt3->version[0] = 1;
	dmt3->version[1] = 0;
	dmt3->version[2] = 3;
	strlcpy(dmt3->name, "striped", DM_MAX_TYPE_NAME);
	dmt3->init = &dm_target_linear_init;
	dmt3->status = &dm_target_linear_status;
	dmt3->strategy = &dm_target_linear_strategy;
	dmt3->deps = &dm_target_linear_deps;
	dmt3->destroy = &dm_target_linear_destroy;
	dmt3->upcall = NULL;
	
	r = dm_target_insert(dmt3);

	dmt4->version[0] = 1;
	dmt4->version[1] = 0;
	dmt4->version[2] = 3;
	strlcpy(dmt4->name, "mirror", DM_MAX_TYPE_NAME);
	dmt4->init = NULL;
	dmt4->status = NULL;
	dmt4->strategy = NULL;
	dmt4->deps = NULL;
	dmt4->destroy = NULL;
	dmt4->upcall = NULL;
	
	r = dm_target_insert(dmt4);

/*	dmt5->version[0] = 1;
	dmt5->version[1] = 0;
	dmt5->version[2] = 5;
	strlcpy(dmt5->name, "snapshot", DM_MAX_TYPE_NAME);
	dmt5->init = &dm_target_snapshot_init;
	dmt5->status = &dm_target_snapshot_status;
	dmt5->strategy = &dm_target_snapshot_strategy;
	dmt5->deps = &dm_target_snapshot_deps;
	dmt5->destroy = &dm_target_snapshot_destroy;
	dmt5->upcall = &dm_target_snapshot_upcall;
	
	r = dm_target_insert(dmt5);
	
	dmt6->version[0] = 1;
	dmt6->version[1] = 0;
	dmt6->version[2] = 5;
	strlcpy(dmt6->name, "snapshot-origin", DM_MAX_TYPE_NAME);
	dmt6->init = &dm_target_snapshot_orig_init;
	dmt6->status = &dm_target_snapshot_orig_status;
	dmt6->strategy = &dm_target_snapshot_orig_strategy;
	dmt6->deps = &dm_target_snapshot_orig_deps;
	dmt6->destroy = &dm_target_snapshot_orig_destroy;
	dmt6->upcall = &dm_target_snapshot_orig_upcall;

	r = dm_target_insert(dmt6);*/
	
	return r;
}
