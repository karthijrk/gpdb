/*
 * slot_projection_codegen.h
 *
 *  Created on: Feb 5, 2016
 *      Author: rahmaf2
 */

#ifndef SRC_BACKEND_CODEGEN_SLOT_PROJECTION_CODEGEN_H_
#define SRC_BACKEND_CODEGEN_SLOT_PROJECTION_CODEGEN_H_

#include "balerion/clang_compiler.h"
#include "balerion/code_generator.h"

class SlotProjectionCodeGen {
public:
	SlotProjectionCodeGen();
	virtual ~SlotProjectionCodeGen() {};
	void GenerateDummyIRModule();
	bool GenerateSlotDeformTuple(TupleDesc tupleDesc);
	void PrepareForExecution();
	//int (*GetDummyIRModule(int)) ();
	auto GetDummyIRModule() -> int(*) (int);

private:
	std::unique_ptr<balerion::CodeGenerator> code_generator_;
};

#endif /* SRC_BACKEND_CODEGEN_SLOT_PROJECTION_CODEGEN_H_ */
