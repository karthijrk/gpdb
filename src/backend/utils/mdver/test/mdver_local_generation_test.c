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

/* test__mdver_local_bump_cmd_id
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


int
main(int argc, char* argv[])
{
    cmockery_parse_arguments(argc, argv);
    
    const UnitTest tests[] = {
		unit_test(test__mdver_create_local__default_value),
                unit_test(test__mdver_local_bump_cmd_id)
	};

    return run_tests(tests);
}
