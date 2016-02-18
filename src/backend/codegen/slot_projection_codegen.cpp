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
//#include "access/htup.h"
//#include "access/tupmacs.h"
//#include "c.h"
#include "catalog/pg_attribute.h"

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

bool SlotProjectionCodeGen::GenerateSlotDeformTuple(TupleDesc tupleDesc) {
	int natts = tupleDesc->natts;

//	bool		hasnulls = HeapTupleHasNulls(tuple);
//
//	if (hasnulls)
//	{
//		return false;
//	}

	COMPILE_ASSERT(sizeof(Datum) == sizeof(int64));
	// void slot_deform_tuple_func(char* data_start_adress, void* values, void* isnull)
    llvm::Function* slot_deform_tuple_func
  	  = code_generator_->CreateFunction<void, char*, int64*>(
  			  "slot_deform_tuple_gen");

    // BasicBlocks for function entry.
    llvm::BasicBlock* entry_block = code_generator_->CreateBasicBlock(
  	  "entry", slot_deform_tuple_func);

    llvm::Value* input = balerion::ArgumentByPosition(slot_deform_tuple_func, 0);
    llvm::Value* out_values = balerion::ArgumentByPosition(slot_deform_tuple_func, 1);

	auto irb =
			code_generator_->ir_builder();

	irb->SetInsertPoint(entry_block);

    llvm::Value* true_const = code_generator_->GetConstant(true);
    llvm::Value* datum_size_const = code_generator_->GetConstant(sizeof(Datum));
    llvm::Value* bool_size_const = code_generator_->GetConstant(sizeof(bool));

	int off = 0;

	Form_pg_attribute *att = tupleDesc->attrs;
	for (int attnum = 0; attnum < natts; attnum++)
	{
		Form_pg_attribute thisatt = att[attnum];
		off = att_align(off, thisatt->attalign);

		if (thisatt->attlen < 0)
		{
			// TODO: Cleanup code generator
			return false;
		}

		// Load tp + off
		// store value
		// store isnull = true
		// add value + sizeof(Datum)
		// add isnull + sizeof(bool)

		// The next address of the input array where we need to read.
		llvm::Value* next_address_load =
			irb->CreateInBoundsGEP(input,
				{code_generator_->GetConstant(off)});

		llvm::Value* next_address_store =
			irb->CreateInBoundsGEP(out_values,
				{code_generator_->GetConstant(attnum)});

		llvm::Value* colVal = nullptr;

		// Load the value from the calculated input address.
		switch(thisatt->attlen)
		{
		case sizeof(char):
			// Read 1 byte at next_address_load
			colVal = irb->CreateLoad(next_address_load);
			// store colVal into out_values[attnum]
			break;
		case sizeof(int16):
			colVal = irb->CreateLoad(code_generator_->GetType<int16>(), next_address_load);
			break;
		case sizeof(int32):
			colVal = irb->CreateLoad(code_generator_->GetType<int32>(), next_address_load);
			break;
		case sizeof(int64):
			colVal = irb->CreateLoad(code_generator_->GetType<int64>(), next_address_load);
			break;
		default:
			//TODO Cleanup
			return false;
		}

		llvm::Value* int64ColVal = irb->CreateZExt(colVal, code_generator_->GetType<int64>());
		irb->CreateStore(int64ColVal, next_address_store);

		llvm::LoadInst* load_instruction =
			code_generator_->ir_builder()->CreateLoad(next_address_load, "input");

		off += thisatt->attlen;
	}

    //code_generator_->ir_builder()->CreateRet(code_generator_->GetConstant(true));

//	/*
//	 * Save state for next execution
//	 */
//	slot->PRIVATE_tts_nvalid = attnum;
//	slot->PRIVATE_tts_off = off;
//	slot->PRIVATE_tts_slow = slow;

    return true;
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

SlotDeformTupleFn SlotProjectionCodeGen::GetSlotDeformTupleFunc() {
	return code_generator_->GetFunctionPointer<void, char*, void*>(
		          "slot_deform_tuple_gen");
}
