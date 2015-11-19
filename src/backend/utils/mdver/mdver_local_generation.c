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
    local_mdver->local_generation = get_global_generation();
    
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
