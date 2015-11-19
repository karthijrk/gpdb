/*-------------------------------------------------------------------------
 *
 * mdver_global_generation.c
 *		Implementation of Global Cache Generation of Metadata Versioning
 *
 * Copyright (c) 2015, Pivotal, Inc.
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"
#include "utils/mdver.h"
#include "utils/guc.h"

/* Name to identify the MD Version Global Cache Generation (GG) shared memory area*/
#define MDVER_GLOBAL_GEN_SHMEM_NAME "MDVer Global Cache Generation"
const int64 INC = 1;

/*
 * Pointer to the shared memory global cache generation (GG)
 * 		This records the current
 * 		generation number, and can be read and written atomically by all backends.
 */
uint64 *mdver_global_generation = NULL;

/*
 * mdver_shmem_init
 * 		Initalize the shared memory data structure needed for MD Version
 */
void
mdver_shmem_init(void) {

    bool attach = false;

    /*Allocate or attach to shared memory area */
    void *shmem_base = ShmemInitStruct(MDVER_GLOBAL_GEN_SHMEM_NAME,
            sizeof (*mdver_global_generation),
            &attach);
    mdver_global_generation = (uint64 *) shmem_base;
    
#ifdef MD_VERSIONING_INSTRUMENTATION
    elog(gp_mdver_loglevel,
            "MDVer: Creating global cache generation");
#endif

    Assert(0 == *mdver_global_generation);
}

/*
 * mdver_shmem_size
 * 		Compute the size of shared memory required for the MD Version component
 */
Size
mdver_shmem_size(void)
{
    return sizeof(*mdver_global_generation);
}

/* 
 * get_global_generation
 * 		Get Current global generation number
 */
uint64
get_global_generation(void)
{
    Assert(NULL != mdver_global_generation);
    return *mdver_global_generation;
}

/*
 * bump_global_generation
 * 		Atomically increment global cache generation by 1
 */
void
bump_global_generation(void)
{
    Assert(NULL != mdver_global_generation);
#ifdef MD_VERSIONING_INSTRUMENTATION
    uint64 old_global_id = *mdver_global_generation;
#endif

    gp_atomic_add_uint64(mdver_global_generation, INC);

#ifdef MD_VERSIONING_INSTRUMENTATION
    elog(gp_mdver_loglevel,
            "MDVer: Bumping global cache generation from %llu to %llu",
			old_global_id, *mdver_global_generation);
#endif
}
