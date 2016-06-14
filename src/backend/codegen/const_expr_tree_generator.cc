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
#include "codegen/const_expr_tree_generator.h"

#include "llvm/IR/Value.h"

extern "C" {
#include "postgres.h"
#include "utils/elog.h"
#include "nodes/execnodes.h"
}

using gpcodegen::ConstExprTreeGenerator;
using gpcodegen::ExprTreeGenerator;

bool ConstExprTreeGenerator::VerifyAndCreateExprTree(
    Expr* expr,
    ExprContext* econtext,
    std::unique_ptr<ExprTreeGenerator>& expr_tree) {
  assert(nullptr != expr && T_Const == nodeTag(expr));
  Const* const_expr = (Const*)expr;
  expr_tree.reset(new ConstExprTreeGenerator(const_expr));
  return true;
}

ConstExprTreeGenerator::ConstExprTreeGenerator(Const* const_expr) :
    const_expr_(const_expr),
    ExprTreeGenerator(ExprTreeNodeType::kConst) {

}

bool ConstExprTreeGenerator::GenerateCode(CodegenUtils* codegen_utils,
                                        ExprContext* econtext,
                                        llvm::Value* llvm_isnull_arg,
                                        llvm::Value* & value) {
  value = codegen_utils->GetConstant(const_expr_->constvalue);
  return true;
}
