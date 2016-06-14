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
#ifndef GPCODEGEN_OP_EXPR_TREE_GENERATOR_H_  // NOLINT(build/header_guard)
#define GPCODEGEN_OP_EXPR_TREE_GENERATOR_H_

#include "codegen/expr_tree_generator.h"

#include "llvm/IR/Value.h"

namespace gpcodegen {

/** \addtogroup gpcodegen
 *  @{
 */


class OpExprTreeGenerator : public ExprTreeGenerator {
 public:
  static bool VerifyAndCreateExprTree(
        Expr* expr,
        ExprContext* econtext,
        std::unique_ptr<ExprTreeGenerator>& expr_tree);

  bool GenerateCode(gpcodegen::CodegenUtils* codegen_utils,
                    ExprContext* econtext,
                    llvm::Value* llvm_isnull_arg,
                    llvm::Value* & value) final;
 protected:
  OpExprTreeGenerator(OpExpr* op_expr,
                      std::vector<std::unique_ptr<ExprTreeGenerator>>& arguments);
 private:
  OpExpr* op_expr_;
  std::vector<std::unique_ptr<ExprTreeGenerator>> arguments_;

};


/** @} */
}  // namespace gpcodegen

#endif  // GPCODEGEN_OP_EXPR_TREE_GENERATOR_H_
