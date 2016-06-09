//---------------------------------------------------------------------------
//  Greenplum Database
//  Copyright (C) 2016 Pivotal Software, Inc.
//
//  @filename:
//    exec_eval_expr_codegen.cc
//
//  @doc:
//    Generates code for ExecEvalExpr function.
//
//---------------------------------------------------------------------------
#include <algorithm>
#include <cstdint>
#include <string>

#include "codegen/exec_eval_expr_codegen.h"
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
#include "postgres.h"
#include "utils/elog.h"
}

using gpcodegen::ExecEvalExprCodegen;

constexpr char ExecEvalExprCodegen::kExecEvalExprPrefix[];

class ElogWrapper {
 public:
  ElogWrapper(gpcodegen::CodegenUtils* codegen_utils) :
    codegen_utils_(codegen_utils) {
    SetupElog();
  }
  ~ElogWrapper() {
    TearDownElog();
  }

  template<typename... V>
  void CreateElog(
      llvm::Value* llvm_elevel,
      llvm::Value* llvm_fmt,
      V ... args ) {

    assert(NULL != llvm_elevel);
    assert(NULL != llvm_fmt);

    codegen_utils_->ir_builder()->CreateCall(
        llvm_elog_start_, {
            codegen_utils_->GetConstant(""), // Filename
            codegen_utils_->GetConstant(0),  // line number
            codegen_utils_->GetConstant("")  // function name
    });
    codegen_utils_->ir_builder()->CreateCall(
        llvm_elog_finish_, {
            llvm_elevel,
            llvm_fmt,
            args...
    });
  }
  template<typename... V>
  void CreateElog(
      int elevel,
      const char* fmt,
      V ... args ) {
    CreateElog(codegen_utils_->GetConstant(elevel),
               codegen_utils_->GetConstant(fmt),
               args...);
  }
 private:
  llvm::Function* llvm_elog_start_;
  llvm::Function* llvm_elog_finish_;

  gpcodegen::CodegenUtils* codegen_utils_;

  void SetupElog(){
    assert(codegen_utils_ != nullptr);
    llvm_elog_start_ = codegen_utils_->RegisterExternalFunction(elog_start);
    assert(llvm_elog_start_ != nullptr);
    llvm_elog_finish_ = codegen_utils_->RegisterExternalFunction(elog_finish);
    assert(llvm_elog_finish_ != nullptr);
  }

  void TearDownElog(){
    llvm_elog_start_ = nullptr;
    llvm_elog_finish_ = nullptr;
  }

};


ExecEvalExprCodegen::ExecEvalExprCodegen
(
		ExecEvalExprFn regular_func_ptr,
		ExecEvalExprFn* ptr_to_regular_func_ptr,
    ExprState *exprstate) :
    BaseCodegen(kExecEvalExprPrefix, regular_func_ptr, ptr_to_regular_func_ptr),
    exprstate_(exprstate) {
}


bool ExecEvalExprCodegen::GenerateExecEvalExpr(
    gpcodegen::CodegenUtils* codegen_utils) {

  assert(NULL != codegen_utils);

  ElogWrapper elogwrapper(codegen_utils);

  llvm::Function* expr_state_func = codegen_utils->
        CreateFunction<ExecEvalExprFn>(
            GetUniqueFuncName());

  // BasicBlock of function entry.
  llvm::BasicBlock* entry_block = codegen_utils->CreateBasicBlock(
      "entry", expr_state_func);

  auto irb = codegen_utils->ir_builder();

  irb->SetInsertPoint(entry_block);

  elogwrapper.CreateElog(DEBUG1, "Falling back to regular expression evaluation.");

  codegen_utils->CreateFallback<ExecEvalExprFn>(
      codegen_utils->RegisterExternalFunction(GetRegularFuncPointer()),
	  expr_state_func);

  return true;
}


bool ExecEvalExprCodegen::GenerateCodeInternal(CodegenUtils* codegen_utils) {
  bool isGenerated = GenerateExecEvalExpr(codegen_utils);

  if (isGenerated) {
    elog(DEBUG1, "ExecEvalExpr was generated successfully!");
    return true;
  }
  else {
    elog(DEBUG1, "ExecEvalExpr generation failed!");
    return false;
  }
}
