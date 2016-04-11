//---------------------------------------------------------------------------
//  Greenplum Database
//  Copyright (C) 2016 Pivotal Software, Inc.
//
//  @filename:
//    init_codegen.cpp
//
//  @doc:
//    C wrappers for initialization of code generator.
//
//---------------------------------------------------------------------------

#include "codegen/codegen_wrapper.h"
#include "codegen/codegen_manager.h"
#include "codegen/slot_deform_tuple_codegen.h"

#include "codegen/utils/codegen_utils.h"

using gpcodegen::CodeGenManager;
using gpcodegen::BaseCodeGen;
using gpcodegen::SlotDeformTupleCodeGen;

// Current code generator manager that oversees all code generators
static void* ActiveCodeGeneratorManager = nullptr;
static bool is_codegen_initalized = false;

// Perform global set-up tasks for code generation. Returns 0 on
// success, nonzero on error.
int InitCodeGen() {
  is_codegen_initalized = gpcodegen::CodeGenUtils::InitializeGlobal();
  return !is_codegen_initalized;
}

void* CodeGeneratorManager_Create(const char* module_name) {
  return new CodeGenManager(module_name);
}

bool CodeGeneratorManager_GenerateCode(void* manager) {
  return static_cast<CodeGenManager*>(manager)->GenerateCode();
}

bool CodeGeneratorManager_PrepareGeneratedFunctions(void* manager) {
  return static_cast<CodeGenManager*>(manager)->PrepareGeneratedFunctions();
}

bool CodeGeneratorManager_NotifyParameterChange(void* manager) {
  // parameter change notification is not supported yet
  assert(false);
  return false;
}

void CodeGeneratorManager_Destroy(void* manager) {
  delete (static_cast<CodeGenManager*>(manager));
}

void* GetActiveCodeGeneratorManager() {
  return ActiveCodeGeneratorManager;
}

void SetActiveCodeGeneratorManager(void* manager) {
  ActiveCodeGeneratorManager = manager;
}

void* SlotDeformTupleCodeGen_Enroll(
    TupleTableSlot* slot,
    SlotDeformTupleFn regular_func_ptr,
    SlotDeformTupleFn* ptr_to_chosen_func_ptr) {
  CodeGenManager* manager = static_cast<CodeGenManager*>(
      GetActiveCodeGeneratorManager());

  if (nullptr == manager) {
    BaseCodeGen<SlotDeformTupleFn>::SetToRegular(
        regular_func_ptr, ptr_to_chosen_func_ptr);
    return nullptr;
  }

  SlotDeformTupleCodeGen* generator = new SlotDeformTupleCodeGen(
      slot, regular_func_ptr, ptr_to_chosen_func_ptr);
  assert(nullptr != manager);
  bool is_enrolled = manager->EnrollCodeGenerator(
      CodeGenFuncLifespan_Parameter_Invariant, generator);
  assert(is_enrolled);
  return generator;
}

