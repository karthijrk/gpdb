//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2016 Pivotal Software, Inc.
//
//	@filename:
//		init_codegen.cpp
//
//	@doc:
//		C wrappers for initialization of Balerion codegen library.
//
//---------------------------------------------------------------------------

#include "codegen/init_codegen.h"
#include "codegen/slot_projection_codegen.h"

#include "balerion/code_generator.h"
#include <iostream>

// Perform global set-up tasks for Balerion codegen library. Returns 0 on
// success, nonzero on error.
extern "C" int InitCodeGen() {
  return balerion::CodeGenerator::InitializeGlobal() ? 0 : 1;
}

extern "C" void* ConstructCodeGenerator() {
	SlotProjectionCodeGen* code_gen = new SlotProjectionCodeGen();
	code_gen->GenerateDummyIRModule();

	void* ret_val = reinterpret_cast<void*>(code_gen);
	return ret_val;
}

extern "C" void PrepareForExecution(void* code_generator)
{
	reinterpret_cast<SlotProjectionCodeGen*>(code_generator)->PrepareForExecution();
}

extern "C" void DestructCodeGenerator(void* code_generator) {
	delete static_cast<SlotProjectionCodeGen*>(code_generator);
}

extern "C" int (*GetDummyFunction(void* code_generator)) (int) {
	return static_cast<SlotProjectionCodeGen*>(code_generator)->GetDummyIRModule();
}
