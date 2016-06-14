//---------------------------------------------------------------------------
//  Greenplum Database
//  Copyright (C) 2016 Pivotal Software, Inc.
//
//  @filename:
//    base_codegen.h
//
//  @doc:
//    Base class for expression tree to generate code
//
//---------------------------------------------------------------------------
#ifndef GPCODEGEN_PG_FUNC_GENERATOR_H_  // NOLINT(build/header_guard)
#define GPCODEGEN_PG_FUNC_GENERATOR_H_

#include <string>
#include <vector>
#include <memory>

#include "codegen/pg_func_generator_interface.h"
#include "codegen/utils/codegen_utils.h"

#include "llvm/IR/Value.h"

namespace gpcodegen {

/** \addtogroup gpcodegen
 *  @{
 */

template <typename FuncPtrType, typename Arg1, typename Arg2>
class PGFuncGenerator : public PGFuncGeneratorInterface {
 public:
  std::string GetName() final { return pg_func_name_; };
  int GetTotalArgCount() final { return 2; }
  bool GenerateCode(gpcodegen::CodegenUtils* codegen_utils,
                    std::vector<llvm::Value*>& llvm_args,
                    llvm::Value* & llvm_out_value) final {
    assert(nullptr != codegen_utils);
    assert(nullptr != mem_func_ptr_);
    if (llvm_args.size() != GetTotalArgCount()) {
      return false;
    }
    auto irb = codegen_utils->ir_builder();
    llvm::Value* llvm_arg0 = irb->CreateTrunc(llvm_args[0],
                                              codegen_utils->GetType<Arg1>());
    llvm::Value* llvm_arg1 = irb->CreateTrunc(llvm_args[1],
                                              codegen_utils->GetType<Arg2>());
    llvm_out_value = (irb->*mem_func_ptr_)(llvm_arg0, llvm_arg1, "");
    return true;
  }

  PGFuncGenerator(int pg_func_oid,
                  const std::string& pg_func_name,
                  FuncPtrType mem_func_ptr) : pg_func_oid_(pg_func_oid),
                      pg_func_name_(pg_func_name),
                      mem_func_ptr_(mem_func_ptr) {

  }
 private:
  std::string pg_func_name_;
  unsigned int pg_func_oid_;
  FuncPtrType mem_func_ptr_;

};

/** @} */
}  // namespace gpcodegen

#endif  // GPCODEGEN_PG_FUNC_GENERATOR_INTERFACE_H_
