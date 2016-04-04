//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2016 Pivotal Software, Inc.
//
//	@filename:
//		codegen_manager.cpp
//
//	@doc:
//		Implementation of a code generator manager
//
//---------------------------------------------------------------------------

#include <cstdint>
#include <string>

#include "codegen/utils/clang_compiler.h"
#include "codegen/utils/utility.h"
#include "codegen/utils/instance_method_wrappers.h"
#include "codegen/utils/codegen_utils.h"
#include "codegen/codegen_interface.h"

#include "codegen/codegen_manager.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/Casting.h"
//#include "access/htup.h"
//#include "access/tupmacs.h"
//#include "c.h"
//#include "catalog/pg_attribute.h"

using namespace gpcodegen;

CodeGenManager::CodeGenManager() {
	codegen_utils_.reset(new gpcodegen::CodeGenUtils("test_module"));
}

bool CodeGenManager::EnrollCodeGenerator(CodeGenFuncLifespan funcLifespan, CodeGenInterface* generator) {
  	// Only CodeGenFuncLifespan_Parameter_Invariant is supported as of now
	assert(funcLifespan == CodeGenFuncLifespan_Parameter_Invariant);
	assert(nullptr != generator);
	enrolled_code_generators_.emplace_back(generator);
	return true;
}

size_t CodeGenManager::GenerateCode() {
	size_t success_count = 0;
	for(auto& generator : enrolled_code_generators_) {
		success_count += (generator->GenerateCode(codegen_utils_.get()) == true ? 1 : 0);
	}

	return success_count;
}

bool CodeGenManager::PrepareGeneratedFunctions() {
  // Call CodeGenUtils to compile entire module
	bool compilation_status = codegen_utils_->PrepareForExecution
	    (gpcodegen::CodeGenUtils::OptimizationLevel::kNone, true);

	if (!compilation_status)
	{
		return compilation_status;
	}

  	// On successful compilation, go through all generator and swap
  	// the pointer so compiled function get called
	gpcodegen::CodeGenUtils* codegen_utils = codegen_utils_.get();
	for(auto& generator : enrolled_code_generators_) {
		generator->SetToGenerated(codegen_utils);
	}

	return true;
}

// notifies that the underlying operator has a parameter change
void CodeGenManager::NotifyParameterChange() {
	// no support for parameter change yet
	assert(false);
}

// Invalidate all generated functions
bool CodeGenManager::InvalidateGeneratedFunctions() {
	// no support for invalidation of generated function
	assert(false);
	return false;
}
