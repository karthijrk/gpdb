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

#include "balerion/code_generator.h"

extern "C" int InitCodeGen() {
  return balerion::CodeGenerator::InitializeGlobal() ? 0 : 1;
}
