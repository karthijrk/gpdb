#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include "cmockery.h"

#include "../mdver_inv_translator.c"
extern int gp_command_count;

/*
 * test__mdver_inv_translator__set_bmp
 * 		Testing invalidate translator functionality
 */
void 
test__mdver_inv_translator__set_bmp(void** state)
{
	mdver_local local;
    Relation relation;
    expect_any(IsAoSegmentRelation, relation);
    will_return(IsAoSegmentRelation, false);
    will_return(mdver_enabled, true);
    will_return(GetCurrentLocalMDVer, &local);
    gp_command_count = 10;
    mdver_inv_translator(relation);
    printf("%d", local.bump_cmd_id);
    assert_true(local.bump_cmd_id == 10);
    
}

int
main(int argc, char* argv[])
{
    cmockery_parse_arguments(argc, argv);
    
    const UnitTest tests[] = {
		unit_test(test__mdver_inv_translator__set_bmp)
	};

    return run_tests(tests);
}
