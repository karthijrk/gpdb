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

extern bool codegen;  //defined from guc

// Perform global set-up tasks for code generation. Returns 0 on
// success, nonzero on error.
unsigned int InitCodegen() {
  return gpcodegen::CodegenUtils::InitializeGlobal();
}

void* CodeGeneratorManagerCreate(const char* module_name) {
  if (!codegen) {
    return nullptr;
  }
  return new CodeGenManager(module_name);
}

unsigned int CodeGeneratorManagerGenerateCode(void* manager) {
  if (!codegen) {
    return 0;
  }
  return static_cast<CodeGenManager*>(manager)->GenerateCode();
}

unsigned int CodeGeneratorManagerPrepareGeneratedFunctions(void* manager) {
  if (!codegen) {
    return 0;
  }
  return static_cast<CodeGenManager*>(manager)->PrepareGeneratedFunctions();
}

unsigned int CodeGeneratorManagerNotifyParameterChange(void* manager) {
  // parameter change notification is not supported yet
  assert(false);
  return 0;
}

void CodeGeneratorManagerDestroy(void* manager) {
  delete (static_cast<CodeGenManager*>(manager));
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
 * @param regular_func_ptr Regular version of the generated function.
 * @param ptr_to_chosen_func_ptr Reference to the function pointer that the caller will call.
 * @param args Variable length argument for ClassType
 *
 * @return Pointer to ClassType
 **/
template <typename ClassType, typename FuncType, typename ...Args>
ClassType* CodeGenEnroll(FuncType regular_func_ptr,
                          FuncType* ptr_to_chosen_func_ptr,
                          Args&&... args) {  // NOLINT(build/c++11)
  CodeGenManager* manager = static_cast<CodeGenManager*>(
        GetActiveCodeGeneratorManager());
  if (nullptr == manager ||
      !codegen) {  // if codegen guc is false
      BaseCodeGen<FuncType>::SetToRegular(
          regular_func_ptr, ptr_to_chosen_func_ptr);
      return nullptr;
    }

  ClassType* generator = new ClassType(
      regular_func_ptr, ptr_to_chosen_func_ptr, std::forward<Args>(args)...);
    bool is_enrolled = manager->EnrollCodeGenerator(
        CodeGenFuncLifespan_Parameter_Invariant, generator);
    assert(is_enrolled);
    return generator;
}

void* SlotDeformTupleCodeGenEnroll(
    SlotDeformTupleFn regular_func_ptr,
    SlotDeformTupleFn* ptr_to_chosen_func_ptr,
    TupleTableSlot* slot) {
  SlotDeformTupleCodeGen* generator = CodeGenEnroll<SlotDeformTupleCodeGen>(
      regular_func_ptr, ptr_to_chosen_func_ptr, slot);
  return generator;
}

