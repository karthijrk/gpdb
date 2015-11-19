/*-------------------------------------------------------------------------
 *
 * mdver_inv_translator.c
 *	 Implementation of Invalidation Translator (IT) for metadata version
 *
 * Copyright (c) 2015, Pivotal, Inc.
 *
 *-------------------------------------------------------------------------
 */

#include "utils/mdver.h"
#include "utils/guc.h"

/*
 * mdver_inv_translator
 * 		This component intercepts all changes to metadata done by a query.
 * 		When a relevant catalog update is detected, this IT component
 * 		updates the bump command id so at command end mdcache is purged and at tx commit
 * 		new global cache generation can be generated.
 * 		relation : The catalog table being touched
 */
void
mdver_inv_translator(Relation relation)
{
    if (!mdver_enabled())
    {
        return;
    }
    mdver_local* local_mdver = GetCurrentLocalMDVer();
    
    /*
     * We set local_mdver to null when transaction commit at AtEOXact_Inval.
     * There are some catalog updates after this for e.g. by storage manager.
     * We don't want to track those changes similar to how it is done for other
     * cache invalidation
     */
    if (NULL == local_mdver)
    {
        return;
    }
    
    if (IsAoSegmentRelation(relation))
    {
		return;
    }
    
#ifdef MD_VERSIONING_INSTRUMENTATION
    elog(gp_mdver_loglevel, "MDVer : INV Translator setting current command id %u to bump",
            GetCurrentCommandId());
#endif
    
    mdver_local_bump_cmd_id(local_mdver);
}
