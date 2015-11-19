/* -------------------------------------------------------------------------
 * 
 * mdver.h
 *	  Interface for metadata versioning
 *
 * Copyright (c) 2014, Pivotal, Inc.
 * 
 * 
 * -------------------------------------------------------------------------
 */

#ifndef MDVER_H
#define	MDVER_H

#include "postgres.h"
#include "utils/relcache.h"

#define DEFAULT_BUMP_CMD_ID 0


typedef struct mdver_local
{
	/*
	 * An integer stored inside the Metadata Cache component of a Backend
	 * process. This holds the last global generation observed by the backend. This
	 * is updated only when a command starts.
	 */
    uint64 local_generation;

    /*
     * An integer counter stored in the local memory of the Backend process. This
     * holds the command id of the last local command that changed metadata.
     */
    int bump_cmd_id;
} mdver_local;

/* Pointer to the shared memory global cache generation (GG)*/
extern uint64 *mdver_global_generation;

/* MD Lite global cache generation init */
void mdver_shmem_init(void);
Size mdver_shmem_size(void);
uint64 get_global_generation(void);
void bump_global_generation(void);

/* MD Lite Local cache generation functions*/
mdver_local* mdver_create_local(void);
void mdver_local_bump_cmd_id(mdver_local* local_mdver);

/*MD Lite Invalidation Translator operations*/
void mdver_inv_translator(Relation relation);

/* MD Lite utility functions */
bool mdver_enabled();

/* inval.c */
extern mdver_local *GetCurrentLocalMDVer(void);


#endif	/* MDVER_H */

