//---------------------------------------------------------------------------
//  Greenplum Database
//  Copyright (C) 2016 Pivotal Software, Inc.
//
//  @filename:
//    codegen_wrapper.cc
//
//  @doc:
//    C wrappers for initialization of code generator.
//
//---------------------------------------------------------------------------

#include "codegen/codegen_wrapper.h"

#include <assert.h>
#include <string>
#include <type_traits>

#include "codegen/base_codegen.h"
#include "codegen/codegen_manager.h"
#include "codegen/exec_eval_expr_codegen.h"
#include "codegen/exec_variable_list_codegen.h"
#include "codegen/expr_tree_generator.h"
#include "codegen/utils/gp_codegen_utils.h"

extern "C" {
#include "lib/stringinfo.h"
#include "utils/inval.h"
#include "utils/syscache.h"
}

using gpcodegen::CodegenManager;
using gpcodegen::BaseCodegen;
using gpcodegen::ExecVariableListCodegen;
using gpcodegen::ExecEvalExprCodegen;

// Current code generator manager that oversees all code generators
static void* ActiveCodeGeneratorManager = nullptr;

extern bool codegen;  // defined from guc
extern bool init_codegen;  // defined from guc

static bool mdcache_invalidation_counter_registered = false;
static int64 mdcache_invalidation_counter = 0;
static int64 last_mdcache_invalidation_counter = 0;

static void
mdsyscache_invalidation_counter_callback(Datum arg, int cacheid,  ItemPointer tuplePtr)
{
  mdcache_invalidation_counter++;
}

static void
mdrelcache_invalidation_counter_callback(Datum arg, Oid relid)
{
  mdcache_invalidation_counter++;
}

static void
register_mdcache_invalidation_callbacks(void)
{
  /* These are all the catalog tables that we care about. */
  int     metadata_caches[] = {
    AGGFNOID,     /* pg_aggregate */
    AMOPOPID,     /* pg_amop */
    CASTSOURCETARGET, /* pg_cast */
    CONSTROID,      /* pg_constraint */
    OPEROID,      /* pg_operator */
    OPFAMILYOID,    /* pg_opfamily */
    PARTOID,      /* pg_partition */
    PARTRULEOID,    /* pg_partition_rule */
    STATRELATT,     /* pg_statistics */
    TYPEOID,      /* pg_type */
    PROCOID,      /* pg_proc */

    /*
     * lookup_type_cache() will also access pg_opclass, via GetDefaultOpClass(),
     * but there is no syscache for it. Postgres doesn't seem to worry about
     * invalidating the type cache on updates to pg_opclass, so we don't
     * worry about that either.
     */
    /* pg_opclass */

    /*
     * Information from the following catalogs are included in the
     * relcache, and any updates will generate relcache invalidation
     * event. We'll catch the relcache invalidation event and don't need
     * to register a catcache callback for them.
     */
    /* pg_class */
    /* pg_index */
    /* pg_trigger */

    /*
     * pg_exttable is only updated when a new external table is dropped/created,
     * which will trigger a relcache invalidation event.
     */
    /* pg_exttable */

    /*
     * XXX: no syscache on pg_inherits. Is that OK? For any partitioning
     * changes, I think there will also be updates on pg_partition and/or
     * pg_partition_rules.
     */
    /* pg_inherits */

    /*
     * We assume that gp_segment_config will not change on the fly in a way that
     * would affect ORCA
     */
    /* gp_segment_config */
  };
  int     i;

  for (i = 0; i < lengthof(metadata_caches); i++)
  {
    CacheRegisterSyscacheCallback(metadata_caches[i],
                    &mdsyscache_invalidation_counter_callback,
                    (Datum) 0);
  }

  /* also register the relcache callback */
  CacheRegisterRelcacheCallback(&mdrelcache_invalidation_counter_callback,
                  (Datum) 0);
}

// Perform global set-up tasks for code generation. Returns 0 on
// success, nonzero on error.
unsigned int InitCodegen() {
  return gpcodegen::GpCodegenUtils::InitializeGlobal();
}

void* CodeGeneratorManagerCreate(const char* module_name) {
  if (!codegen) {
    return nullptr;
  }
  return new CodegenManager(module_name);
}

unsigned int CodeGeneratorManagerGenerateCode(void* manager) {
  if (!codegen) {
    return 0;
  }
  return static_cast<CodegenManager*>(manager)->GenerateCode();
}

unsigned int CodeGeneratorManagerPrepareGeneratedFunctions(void* manager) {
  if (!codegen) {
    return 0;
  }
  return static_cast<CodegenManager*>(manager)->PrepareGeneratedFunctions();
}

unsigned int CodeGeneratorManagerNotifyParameterChange(void* manager) {
  // parameter change notification is not supported yet
  assert(false);
  return 0;
}

void CodeGeneratorManagerAccumulateExplainString(void* manager) {
  if (!codegen) {
    return;
  }
  assert(nullptr != manager);
  static_cast<CodegenManager*>(manager)->AccumulateExplainString();
}

char* CodeGeneratorManagerGetExplainString(void* manager) {
  if (!codegen) {
    return nullptr;
  }
  StringInfo return_string = makeStringInfo();
  appendStringInfoString(
      return_string,
      static_cast<CodegenManager*>(manager)->GetExplainString().c_str());
  return return_string->data;
}

void CodeGeneratorManagerDestroy(void* manager) {
  delete (static_cast<CodegenManager*>(manager));
}

void* GetActiveCodeGeneratorManager() {
  return ActiveCodeGeneratorManager;
}

void SetActiveCodeGeneratorManager(void* manager) {
  ActiveCodeGeneratorManager = manager;
}

/**
 * @brief Template function to facilitate enroll for any type of
 *        codegen
 *
 * @tparam ClassType Type of Code Generator class
 * @tparam FuncType Type of the regular function
 * @tparam Args Variable argument that ClassType will take in its constructor
 *
 * @param regular_func_ptr Regular version of the target function.
 * @param ptr_to_chosen_func_ptr Reference to the function pointer that the caller will call.
 * @param args Variable length argument for ClassType
 *
 * @return Pointer to ClassType
 **/
template <typename ClassType, typename FuncType, typename ...Args>
ClassType* CodegenEnroll(FuncType regular_func_ptr,
                          FuncType* ptr_to_chosen_func_ptr,
                          Args&&... args) {  // NOLINT(build/c++11)
  CodegenManager* manager = static_cast<CodegenManager*>(
        GetActiveCodeGeneratorManager());
  if (nullptr == manager ||
      !codegen) {  // if codegen guc is false
      BaseCodegen<FuncType>::SetToRegular(
          regular_func_ptr, ptr_to_chosen_func_ptr);
      return nullptr;
    }

  ClassType* generator = new ClassType(
      manager,
      regular_func_ptr,
      ptr_to_chosen_func_ptr,
      std::forward<Args>(args)...);
    bool is_enrolled = manager->EnrollCodeGenerator(
        CodegenFuncLifespan_Parameter_Invariant, generator);
    assert(is_enrolled);
    return generator;
}

void* ExecVariableListCodegenEnroll(
    ExecVariableListFn regular_func_ptr,
    ExecVariableListFn* ptr_to_chosen_func_ptr,
    ProjectionInfo* proj_info,
    TupleTableSlot* slot) {
  static bool is_registered = false;
  if (!is_registered) {
    register_mdcache_invalidation_callbacks();
    is_registered = true;
  }
  ExecVariableListCodegen* generator = CodegenEnroll<ExecVariableListCodegen>(
      regular_func_ptr, ptr_to_chosen_func_ptr, proj_info, slot);
  return generator;
}

void* ExecEvalExprCodegenEnroll(
    ExecEvalExprFn regular_func_ptr,
    ExecEvalExprFn* ptr_to_chosen_func_ptr,
    ExprState *exprstate,
    ExprContext *econtext,
    PlanState* plan_state) {
  ExecEvalExprCodegen* generator = CodegenEnroll<ExecEvalExprCodegen>(
      regular_func_ptr,
      ptr_to_chosen_func_ptr,
      exprstate,
      econtext,
      plan_state);
  return generator;
}





