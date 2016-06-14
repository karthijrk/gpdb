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
#ifndef GPCODEGEN_EXPR_TREE_GENERATOR_H_  // NOLINT(build/header_guard)
#define GPCODEGEN_EXPR_TREE_GENERATOR_H_

#include <string>
#include <vector>
#include <memory>

#include "codegen/utils/codegen_utils.h"

#include "llvm/IR/Value.h"

typedef struct ExprContext ExprContext;
typedef struct ExprState ExprState;
typedef struct Expr Expr;
typedef struct OpExpr OpExpr;
typedef struct Var Var;
typedef struct Const Const;

namespace gpcodegen {

/** \addtogroup gpcodegen
 *  @{
 */

enum class ExprTreeNodeType {
  kConst = 0,
  kVar = 1,
  kOperator = 2
};

class ExprTreeGenerator {
 public:
  static bool VerifyAndCreateExprTree(
      Expr* expr,
      ExprContext* econtext,
      std::unique_ptr<ExprTreeGenerator>& expr_tree);

  virtual bool GenerateCode(gpcodegen::CodegenUtils* codegen_utils,
                            ExprContext* econtext,
                            llvm::Value* llvm_isnull_arg,
                            llvm::Value* & value) = 0;
protected:
  ExprTreeGenerator(ExprTreeNodeType node_type) :
                      node_type_(node_type) {}
 private:
  ExprTreeNodeType node_type_;

  DISALLOW_COPY_AND_ASSIGN(ExprTreeGenerator);
};


/** @} */
}  // namespace gpcodegen

#endif  // GPCODEGEN_EXPR_TREE_GENERATOR_H_
