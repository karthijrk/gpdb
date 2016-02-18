//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2016 Pivotal Software, Inc.
//
//	@filename:
//		init_codegen.h
//
//	@doc:
//		C wrappers for initialization of Balerion codegen library.
//
//---------------------------------------------------------------------------
#ifndef CODEGEN_INIT_CODEGEN_H_
#define CODEGEN_INIT_CODEGEN_H_

#include "postgres.h"
#include "catalog/pg_attribute.h"
#include "access/tupdesc.h"

#ifdef __cplusplus
extern "C" {
#endif

// Do one-time global initialization of Balerion and LLVM libraries. Returns 0
// on success, nonzero on error.
int InitCodeGen();

void* ConstructCodeGenerator();

void PrepareForExecution(void* code_generator);

void DestructCodeGenerator(void* code_generator);

int (*GetDummyFunction(void* code_generator)) (int);

void GenerateSlotDeformTuple(void* code_generator, TupleDesc tupleDesc);

void (*GetSlotDeformTupleFunction(void* code_generator)) (char*, void*);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // CODEGEN_INIT_CODEGEN_H_
