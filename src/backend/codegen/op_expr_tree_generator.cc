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

#include "codegen/expr_tree_generator.h"
#include "codegen/op_expr_tree_generator.h"
#include "codegen/pg_int_expr_generator.h"

#include "llvm/IR/Value.h"

extern "C" {
#include "postgres.h"
#include "utils/elog.h"
#include "nodes/execnodes.h"
}

using gpcodegen::OpExprTreeGenerator;
using gpcodegen::ExprTreeGenerator;
using gpcodegen::CodegenUtils;
using gpcodegen::PGFuncGeneratorInterface;
using gpcodegen::PGFuncGenerator;
using gpcodegen::CodeGenFuncMap;
using llvm::IRBuilder;

CodeGenFuncMap
OpExprTreeGenerator::supported_function_;

void OpExprTreeGenerator::InitializeSupportedFunction() {
  if (!supported_function_.empty()) { return; }

  supported_function_[149] = std::unique_ptr<PGFuncGeneratorInterface>(
      new PGIRBuilderFuncGenerator<decltype(&IRBuilder<>::CreateICmpSLE),
      int32_t, int32_t>(149, "int4le", &IRBuilder<>::CreateICmpSLE));

  supported_function_[1088] = std::unique_ptr<PGFuncGeneratorInterface>(
      new PGIRBuilderFuncGenerator<decltype(&IRBuilder<>::CreateICmpSLE),
      int32_t, int32_t>(1088, "date_le", &IRBuilder<>::CreateICmpSLE));

  supported_function_[999] = std::unique_ptr<PGFuncGeneratorInterface>(
        new PGGenericFuncGenerator<int32_t, int32_t>(
            999, "mult", &PGIntExprGenerator::MultWithOverflow<int32_t, int32_t, int32_t>));
}

OpExprTreeGenerator::OpExprTreeGenerator(
    ExprState* expr_state,
    std::vector<std::unique_ptr<ExprTreeGenerator>>& arguments) :
        arguments_(std::move(arguments)),
        ExprTreeGenerator(expr_state, ExprTreeNodeType::kOperator) {
}

bool OpExprTreeGenerator::VerifyAndCreateExprTree(
    ExprState* expr_state,
    ExprContext* econtext,
    std::unique_ptr<ExprTreeGenerator>& expr_tree) {
  assert(nullptr != expr_state && nullptr != expr_state->expr && T_OpExpr == nodeTag(expr_state->expr));

  OpExpr* op_expr = (OpExpr*)expr_state->expr;
  expr_tree.reset(nullptr);
  CodeGenFuncMap::iterator itr =  supported_function_.find(op_expr->opfuncid);
  if (itr == supported_function_.end()) {
    // Operators are stored in pg_proc table. See postgres.bki for more details.
    elog(DEBUG1, "Unsupported operator %d.", op_expr->opno);
    return false;
  }

  List *arguments = ((FuncExprState *)expr_state)->args;
  if (nullptr == arguments) {
    elog(DEBUG1, "No argument for the function");
    return false;
  }
  // In ExecEvalFuncArgs
  if (list_length(arguments) != itr->second->GetTotalArgCount()) {
    elog(DEBUG1, "Wrong number of arguments (!= %d)", itr->second->GetTotalArgCount());
    return false;
  }

  ListCell   *arg = nullptr;
  bool supported_tree = true;
  std::vector<std::unique_ptr<ExprTreeGenerator>> expr_tree_arguments;
  foreach(arg, arguments)
  {
    // retrieve argument's ExprState
    ExprState  *argstate = (ExprState *) lfirst(arg);
    assert(nullptr != argstate);
    std::unique_ptr<ExprTreeGenerator> arg(nullptr);
    supported_tree &= ExprTreeGenerator::VerifyAndCreateExprTree(argstate,
                                                                econtext,
                                                                arg);
    if (!supported_tree) {
      break;
    }
    assert(nullptr != arg);
    expr_tree_arguments.push_back(std::move(arg));
  }
  if (!supported_tree) {
    return supported_tree;
  }
  expr_tree.reset(new OpExprTreeGenerator(expr_state, expr_tree_arguments));
  return true;
}

bool OpExprTreeGenerator::GenerateCode(CodegenUtils* codegen_utils,
                                       ExprContext* econtext,
                                       llvm::Value* llvm_isnull_arg,
                                       llvm::Value* & value) {
  value = nullptr;
  OpExpr* op_expr = (OpExpr*)expr_state()->expr;
  CodeGenFuncMap::iterator itr =  supported_function_.find(op_expr->opfuncid);

  if (itr == supported_function_.end()) {
      // Operators are stored in pg_proc table. See postgres.bki for more details.
      elog(WARNING, "Unsupported operator %d.", op_expr->opfuncid);
      return false;
  }

  if (arguments_.size() != itr->second->GetTotalArgCount()) {
    elog(WARNING, "Expected argument size to be %d\n", itr->second->GetTotalArgCount());
    return false;
  }
  bool arg_generated = true;
  std::vector<llvm::Value*> llvm_arguments;
  for(auto& arg : arguments_) {
    llvm::Value* llvm_arg = nullptr;
    arg_generated &= arg->GenerateCode(codegen_utils, econtext, llvm_isnull_arg, llvm_arg);
    if (!arg_generated) {
      return false;
    }
    llvm_arguments.push_back(llvm_arg);
  }
  return itr->second->GenerateCode(codegen_utils, llvm_arguments, value);
}
