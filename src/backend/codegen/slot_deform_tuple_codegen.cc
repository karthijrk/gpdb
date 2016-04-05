//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2016 Pivotal Software, Inc.
//
//	@filename:
//		codegen_utils.cpp
//
//	@doc:
//		Contains different code generators
//
//---------------------------------------------------------------------------
#include "codegen/slot_deform_tuple_codegen.h"
#include <cstdint>
#include <string>

#include "codegen/utils/clang_compiler.h"
#include "codegen/utils/utility.h"
#include "codegen/utils/instance_method_wrappers.h"
#include "codegen/utils/codegen_utils.h"

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

extern "C" {
#include "utils/elog.h"
}

using namespace gpcodegen;

constexpr char SlotDeformTupleCodeGen::kSlotDeformTupleNamePrefix[];

SlotDeformTupleCodeGen::SlotDeformTupleCodeGen(TupleTableSlot* slot,
		SlotDeformTupleFn regular_func_ptr,
		SlotDeformTupleFn* ptr_to_regular_func_ptr) :
				BaseCodeGen(kSlotDeformTupleNamePrefix, regular_func_ptr,
						ptr_to_regular_func_ptr), slot_(slot) {

}

static void ElogWrapper(const char* func_name) {
	elog(INFO, "Calling wrapped function: %s", func_name);
}

bool SlotDeformTupleCodeGen::GenerateSimpleSlotDeformTuple(
		gpcodegen::CodeGenUtils* codegen_utils) {
	llvm::Function* llvm_elog_wrapper = codegen_utils->RegisterExternalFunction(
			ElogWrapper);
	assert(llvm_elog_wrapper != nullptr);

	auto regular_func_pointer = GetRegularFuncPointer();
	llvm::Function* llvm_regular_function =
			codegen_utils->RegisterExternalFunction(regular_func_pointer);
	assert(llvm_regular_function != nullptr);

	llvm::Function* llvm_function = codegen_utils->CreateFunctionTypeDef<
			decltype(regular_func_pointer)>(GetUniqueFuncName());

	llvm::BasicBlock* function_body = codegen_utils->CreateBasicBlock("fn_body",
			llvm_function);

	codegen_utils->ir_builder()->SetInsertPoint(function_body);
	llvm::Value* func_name_llvm = codegen_utils->GetConstant(
			GetOrigFuncName().c_str());
	codegen_utils->ir_builder()->CreateCall(llvm_elog_wrapper,
			{ func_name_llvm });

	std::vector<llvm::Value*> forwarded_args;

	for (llvm::Argument& arg : llvm_function->args()) {
		forwarded_args.push_back(&arg);
	}

	llvm::CallInst* call = codegen_utils->ir_builder()->CreateCall(
			llvm_regular_function, forwarded_args);

	// Return the result of the call, or void if the function returns void.
	if (std::is_same<
			gpcodegen::codegen_utils_detail::FunctionTypeUnpacker<
			decltype(regular_func_pointer)>::R, void>::value) {
		codegen_utils->ir_builder()->CreateRetVoid();
	} else {
		codegen_utils->ir_builder()->CreateRet(call);
	}

	return true;
}

bool SlotDeformTupleCodeGen::DoCodeGeneration(CodeGenUtils* codegen_utils) {
	//elog(WARNING, "GenerateCode: %p, %s", codegen_utils, GetUniqueFuncName().c_str());

	GenerateSimpleSlotDeformTuple(codegen_utils);

	return true;
}
