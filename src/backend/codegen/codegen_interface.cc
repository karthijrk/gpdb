//---------------------------------------------------------------------------
//  Greenplum Database
//  Copyright (C) 2016 Pivotal Software, Inc.
//
//  @filename:
//    codegen_interface.cc
//
//  @doc:
//    Implementation of codegen interface's static function
//
//---------------------------------------------------------------------------
#include "codegen/codegen_interface.h"

using gpcodegen::CodeGenInterface;

// Initalization of unique counter
unsigned CodeGenInterface::unique_counter_ = 0;

std::string CodeGenInterface::GenerateUniqueName(
    const std::string& orig_func_name) {
  return orig_func_name + std::to_string(unique_counter_++);
}
