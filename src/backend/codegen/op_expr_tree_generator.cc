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

#include "llvm/IR/Value.h"

extern "C" {
#include "postgres.h"
#include "utils/elog.h"
#include "nodes/execnodes.h"
}

using gpcodegen::OpExprTreeGenerator;
using gpcodegen::ExprTreeGenerator;
using gpcodegen::CodegenUtils;

OpExprTreeGenerator::OpExprTreeGenerator(
    OpExpr* op_expr,
    std::vector<std::unique_ptr<ExprTreeGenerator>>& arguments) :
        op_expr_(op_expr),
        arguments_(std::move(arguments)),
        ExprTreeGenerator(ExprTreeNodeType::kOperator) {
}

bool OpExprTreeGenerator::VerifyAndCreateExprTree(
    Expr* expr,
    ExprContext* econtext,
    std::unique_ptr<ExprTreeGenerator>& expr_tree) {
  assert(nullptr != expr && T_OpExpr == nodeTag(expr));

  OpExpr* op_expr = (OpExpr*)expr;
  expr_tree.reset(nullptr);
  if (op_expr->opno != 523 /* "<=" */ && op_expr->opno != 1096 /* date "<=" */) {
    // Operators are stored in pg_proc table. See postgres.bki for more details.
    elog(DEBUG1, "Unsupported operator %d.", op_expr->opno);
    return false;
  }

  List *arguments = op_expr->args;
  // In ExecEvalFuncArgs
  if (list_length(arguments) != 2) {
    elog(DEBUG1, "Wrong number of arguments (!= 2)");
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
    supported_tree &= ExprTreeGenerator::VerifyAndCreateExprTree(argstate->expr,
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
  expr_tree.reset(new OpExprTreeGenerator(op_expr, expr_tree_arguments));
}

bool OpExprTreeGenerator::GenerateCode(CodegenUtils* codegen_utils,
                                       ExprContext* econtext,
                                       llvm::Value* llvm_isnull_arg,
                                       llvm::Value* & value) {
  value = nullptr;
  if (op_expr_->opno != 523 && op_expr_->opno != 1096) {
    return false;
  }
  if (arguments_.size() != 2) {
    elog(WARNING, "Expected argument size to be 2\n");
    return false;
  }

  llvm::Value* llvm_arg_val0 = nullptr;
  llvm::Value* llvm_arg_val1 = nullptr;

  if (!arguments_[0]->GenerateCode(codegen_utils, econtext, llvm_isnull_arg, llvm_arg_val0) ||
      !arguments_[1]->GenerateCode(codegen_utils, econtext, llvm_isnull_arg, llvm_arg_val1)) {
    return false;
  }
  auto irb = codegen_utils->ir_builder();
  if (op_expr_->opno == 523) {
    value = irb->CreateICmpSLE(llvm_arg_val0, llvm_arg_val1);
  }
  else {
    value = irb->CreateICmpSLE(
        irb->CreateTrunc(llvm_arg_val0, codegen_utils->GetType<int32_t>()),
        irb->CreateTrunc(llvm_arg_val1, codegen_utils->GetType<int32_t>()));
  }
  return true;
}
