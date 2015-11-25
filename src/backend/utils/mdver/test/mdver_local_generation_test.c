#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include "cmockery.h"

#include "../mdver_local_generation.c"
extern int gp_command_count;

/*
 * test__mdver_create_local__default_value
 * 		Testing default value for newly created local cache generations objects
 */
void 
test__mdver_create_local__default_value(void** state)
{
    will_return(mdver_get_global_generation, 0);
    mdver_local* local_mdver = mdver_create_local();
    assert_true(local_mdver->bump_cmd_id == DEFAULT_BUMP_CMD_ID &&
                local_mdver->local_generation == 0);
    pfree(local_mdver);
}

/*
 * test__mdver_local_bump_cmd_id
 * 		Testing setting bump id functionality
 */
void 
test__mdver_local_bump_cmd_id(void** state)
{
	mdver_local local;
	gp_command_count = 10;
    mdver_local_bump_cmd_id(&local);
    assert_true(local.bump_cmd_id == 10);
}

/*
 * test__mdver_command_begin
 *  Testing that at command begin, we correctly check the local and global
 *  generation and take action:
 *    - if LG == GG, nothing to do, return false
 *    - if LG != GG, then set LG = GG and return true
 */
void
test__mdver_command_begin(void **state)
{
	mdver_local local_mdver = {0, 0};

	/* First case, local_generation == global_generation == 10 */
	local_mdver.local_generation = 10;
	local_mdver.bump_cmd_id = 20;
	will_return(mdver_enabled, true);
	will_return(mdver_get_global_generation, 10);
	will_return(GetCurrentLocalMDVer, &local_mdver);

	bool result = mdver_command_begin();
	assert_false(result);
	assert_true(local_mdver.local_generation == 10);
	assert_true(local_mdver.bump_cmd_id == 20);

	/* Second case, local_generation = 10, global_generation = 15 */
	local_mdver.local_generation = 10;
	local_mdver.bump_cmd_id = 20;
	will_return(mdver_enabled, true);
	will_return(mdver_get_global_generation, 15);
	will_return(GetCurrentLocalMDVer, &local_mdver);

	result = mdver_command_begin();

	assert_true(result);
	assert_true(local_mdver.local_generation == 15);
	assert_true(local_mdver.bump_cmd_id == 20);


}

int
main(int argc, char* argv[])
{
    cmockery_parse_arguments(argc, argv);
    
    const UnitTest tests[] = {
    		unit_test(test__mdver_create_local__default_value),
			unit_test(test__mdver_local_bump_cmd_id),
			unit_test(test__mdver_command_begin)
	};

    return run_tests(tests);
}
