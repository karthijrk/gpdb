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
#include "nodes/execnodes.h"
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

  llvm::Function* exec_eval_expr_func = codegen_utils->
      CreateFunction<ExecEvalExprFn>(
          GetUniqueFuncName());

  // Function arguments to ExecVariableList
  llvm::Value* llvm_expression_arg = ArgumentByPosition(exec_eval_expr_func, 0);
  llvm::Value* llvm_econtext_arg = ArgumentByPosition(exec_eval_expr_func, 1);
  llvm::Value* llvm_isnull_arg = ArgumentByPosition(exec_eval_expr_func, 2);
  llvm::Value* llvm_isDone_arg = ArgumentByPosition(exec_eval_expr_func, 3);

  // BasicBlock of function entry.
  llvm::BasicBlock* entry_block = codegen_utils->CreateBasicBlock(
      "entry", exec_eval_expr_func);

  auto irb = codegen_utils->ir_builder();

  irb->SetInsertPoint(entry_block);

  if (exprstate_ && exprstate_->expr) {
    // Codegen Operation expression
    Expr *expr_ = exprstate_->expr;
    if (nodeTag(expr_) != T_OpExpr) {
      elog(DEBUG1, "Unsupported expression. Support only T_OpExpr");
      return false;
    }

    // In ExecEvalOper
    OpExpr *op = (OpExpr *) expr_;
    elog(DEBUG1, "Operator oid = %d", op->opno);

    if (op->opno != 523 /* "<=" */) {
      elog(DEBUG1, "Unsupported operator %d.", op->opno);
      return false;
    }

    elog(DEBUG1, "Found supported operator (<=)");
    // In ExecMakeFunctionResult
    // retrieve operator's arguments
    List *arguments = ((FuncExprState *)exprstate_)->args;
    // In ExecEvalFuncArgs
    if (list_length(arguments) != 2) {
      elog(DEBUG1, "Wrong number of arguments (!= 2)");
      return false;
    }

    llvm::Value* const_value;

    ListCell   *arg;
    foreach(arg, arguments)
    {
      // for each argument retrieve the ExprState
      ExprState  *argstate = (ExprState *) lfirst(arg);

      // Currently we support only variable and constant arguments
      if (nodeTag(argstate->expr) == T_Var) {
        // In ExecEvalVar
        Var *variable = (Var *) argstate->expr;
        int attnum = variable->varattno;
        elog(DEBUG1, "Variable attnum = %d", attnum);
      }
      else if (nodeTag(argstate->expr) == T_Const) {
        // In ExecEvalConst
        Const *con = (Const *) argstate->expr;
        int value = con->constvalue;
        const_value = codegen_utils->GetConstant(con->constvalue);
        elog(DEBUG1, "Constant value= %d", value);
      }
      else {
        elog(DEBUG1, "Unsupported argument type.");
        return false;
      }
    }
    // after we have all these we are able to execute it and call **int4le**

  }

  elogwrapper.CreateElog(DEBUG1, "Falling back to regular expression evaluation.");

  codegen_utils->CreateFallback<ExecEvalExprFn>(
      codegen_utils->RegisterExternalFunction(GetRegularFuncPointer()),
      exec_eval_expr_func);

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
