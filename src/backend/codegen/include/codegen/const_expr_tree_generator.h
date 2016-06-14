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
#ifndef GPCODEGEN_CONST_EXPR_TREE_GENERATOR_H_  // NOLINT(build/header_guard)
#define GPCODEGEN_CONST_EXPR_TREE_GENERATOR_H_

#include "codegen/expr_tree_generator.h"

#include "llvm/IR/Value.h"

namespace gpcodegen {

/** \addtogroup gpcodegen
 *  @{
 */

class ConstExprTreeGenerator : public ExprTreeGenerator {
 public:
  static bool VerifyAndCreateExprTree(
        ExprState* expr_state,
        ExprContext* econtext,
        std::unique_ptr<ExprTreeGenerator>& expr_tree);

  bool GenerateCode(gpcodegen::CodegenUtils* codegen_utils,
                    ExprContext* econtext,
                    llvm::Value* llvm_isnull_arg,
                    llvm::Value* & value) final;

 protected:
  ConstExprTreeGenerator(ExprState* expr_state);
};

/** @} */
}  // namespace gpcodegen

#endif  // GPCODEGEN_CONST_EXPR_TREE_GENERATOR_H_
