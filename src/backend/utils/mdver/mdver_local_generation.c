/*-------------------------------------------------------------------------
 *
 * mdver_local_generation.c
 *	 Implementation of Local Cache Generation for Metadata Versioning
 *
 * Copyright (c) 2015, Pivotal, Inc.
 *
 *-------------------------------------------------------------------------
 */

#include "utils/mdver.h"
#include "utils/guc.h"
#include "cdb/cdbvars.h"

/*
 * mdver_create_local
 * 		Creates the new Local MDVer and initialize for the current transaction
 * 		Same mdver_local will be used for subtransaction as well.
 */
mdver_local*
mdver_create_local(void)
{
	mdver_local* local_mdver = palloc0(sizeof(mdver_local));
    
    /* sync to global cache generation */
    local_mdver->local_generation = mdver_get_global_generation();
    
    /* set to default bump command id */
    local_mdver->bump_cmd_id = DEFAULT_BUMP_CMD_ID;
    
#ifdef MD_VERSIONING_INSTRUMENTATION
    elog(gp_mdver_loglevel, "MDVer : Creating local cache generation. Gp_role=%d, Gp_identity=%d",
            Gp_role,
            GpIdentity.segindex);
#endif
    
    return local_mdver;
}

/*
 * mdver_local_bump_cmd_id
 * 		Set  bump command id to current command id.
 * 		local_mdver : current local mdver pointer
 */
void
mdver_local_bump_cmd_id(mdver_local* local_mdver)
{
    Assert(NULL != local_mdver);

#ifdef MD_VERSIONING_INSTRUMENTATION
    int old_cmd_id = local_mdver->bump_cmd_id;
#endif

    local_mdver->bump_cmd_id = gp_command_count;

#ifdef MD_VERSIONING_INSTRUMENTATION
    elog(gp_mdver_loglevel, "MDVer: Bump Command id changed from %d to %d",
            old_cmd_id, local_mdver->bump_cmd_id);
#endif

}

/*
 * mdver_command_begin
 *   Called at the beginning of a new command
 *   Checks local generation vs global generation. If different, returns true and
 *   updates the local generation.
 *   The caller should purge the MD Cache when a new generation is detected.
 */
bool
mdver_command_begin(void) {

	/* Always purge cache if MD Versioning is disabled */
	if (!mdver_enabled()) {
		return true;
	}

	uint64 global_generation = mdver_get_global_generation();
	mdver_local *local_mdver = GetCurrentLocalMDVer();
	Assert(NULL != local_mdver);

#ifdef 	MD_VERSIONING_INSTRUMENTATION
	elog(gp_mdver_loglevel, "MDVer: New command to optimizer, LG = " UINT64_FORMAT" , GG = " UINT64_FORMAT " mdver_dirty_mdcache = %d",
			local_mdver->local_generation, global_generation, mdver_dirty_mdcache);
#endif

	bool new_generation_detected = false;

	/*
	 * We updated the local generation and ask a MD Cache purge in two
	 * scenarios:
	 *   1. A previous command in this session has updated the
	 *     catalog (mdver dirty flag true)
	 *   2. A transaction in another session committed a catalog
	 *     change and bumped the global generation (LG != GG)
	 */
	if (mdver_dirty_mdcache ||
			local_mdver->local_generation != global_generation)
	{
		new_generation_detected = true;
		local_mdver->local_generation = global_generation;
		/* We are requesting MD Cache purge, we can reset the "dirty" flag */
		mdver_dirty_mdcache = false;

#ifdef 	MD_VERSIONING_INSTRUMENTATION
	elog(gp_mdver_loglevel, "MDVer: MDCache purge requested at command start");
#endif
	}

	return new_generation_detected;
}
