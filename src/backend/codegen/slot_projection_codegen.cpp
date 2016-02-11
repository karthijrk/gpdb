//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2016 Pivotal Software, Inc.
//
//	@filename:
//		slot_projection_codegen.cpp
//
//	@doc:
//		Helper class to generate IR code for slot_deform_tuple
//
//---------------------------------------------------------------------------
#include "codegen/slot_projection_codegen.h"
#include <cstdint>
#include <string>

#include "balerion/clang_compiler.h"
#include "balerion/code_generator.h"
#include "balerion/utility.h"
#include "balerion/instance_method_wrappers.h"

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


SlotProjectionCodeGen::SlotProjectionCodeGen() {
	code_generator_.reset(new balerion::CodeGenerator("test_module"));
}

void SlotProjectionCodeGen::GenerateDummyIRModule() {
    llvm::Function* project_scalar_function
  	  = code_generator_->CreateFunction<int, int>(
  			  "dummy_func");

    // BasicBlocks for function entry.
    llvm::BasicBlock* entry_block = code_generator_->CreateBasicBlock(
  	  "entry", project_scalar_function);

    llvm::Value* input = balerion::ArgumentByPosition(project_scalar_function, 0);

    code_generator_->ir_builder()->SetInsertPoint(entry_block);

    llvm::Value* ret_val =
      		code_generator_->ir_builder()->CreateAdd(input,
      			{code_generator_->GetConstant(1u)});

    code_generator_->ir_builder()->CreateRet(ret_val);
//    code_generator_->ir_builder()->CreateRet(
//        code_generator_->GetConstant<int>(42));

}

void SlotProjectionCodeGen::PrepareForExecution()
{
	code_generator_->PrepareForExecution(balerion::CodeGenerator::OptimizationLevel::kNone, true);
}

/*int (*SlotProjectionCodeGen::GetDummyIRModule(int)) () {
	  return code_generator_->GetFunctionPointer<int, int>(
	          "dummy_func");
}*/


auto SlotProjectionCodeGen::GetDummyIRModule() -> int(*) (int) {
	return code_generator_->GetFunctionPointer<int, int>(
		          "dummy_func");
}
