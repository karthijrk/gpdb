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
#include "codegen/var_expr_tree_generator.h"

#include "llvm/IR/Value.h"

extern "C" {
#include "postgres.h"
#include "utils/elog.h"
#include "nodes/execnodes.h"
}

using gpcodegen::VarExprTreeGenerator;
using gpcodegen::ExprTreeGenerator;
using gpcodegen::CodegenUtils;

bool VarExprTreeGenerator::VerifyAndCreateExprTree(
    Expr* expr,
    ExprContext* econtext,
    std::unique_ptr<ExprTreeGenerator>& expr_tree) {
  assert(nullptr != expr && T_Var == nodeTag(expr));
  Var* var_expr = (Var*)expr;
  expr_tree.reset(new VarExprTreeGenerator(var_expr));
  return true;
}

VarExprTreeGenerator::VarExprTreeGenerator(Var* var_expr) :
    var_expr_(var_expr),
    ExprTreeGenerator(ExprTreeNodeType::kVar) {

}

bool VarExprTreeGenerator::GenerateCode(CodegenUtils* codegen_utils,
                                        ExprContext* econtext,
                                        llvm::Value* llvm_isnull_arg,
                                        llvm::Value* & value) {
  value = nullptr;
  int attnum = var_expr_->varattno;
  auto irb = codegen_utils->ir_builder();

  // slot = econtext->ecxt_scantuple; {{{
  // At code generation time, slot is NULL.
  // For that reason, we keep a double pointer to slot and at execution time
  // we load slot.
  TupleTableSlot **ptr_to_slot_ptr = NULL;
  switch (var_expr_->varno)
  {
    case INNER:  /* get the tuple from the inner node */
      ptr_to_slot_ptr = &econtext->ecxt_innertuple;
      break;

    case OUTER:  /* get the tuple from the outer node */
      ptr_to_slot_ptr = &econtext->ecxt_outertuple;
      break;

    default:     /* get the tuple from the relation being scanned */
      ptr_to_slot_ptr = &econtext->ecxt_scantuple;
      break;
  }

  llvm::Value *llvm_slot = irb->CreateLoad(
      codegen_utils->GetConstant(ptr_to_slot_ptr));
  //}}}

  llvm::Value *llvm_variable_varattno = codegen_utils->
      GetConstant<int32_t>(attnum);

  // External functions
  llvm::Function* llvm_slot_getattr =
      codegen_utils->RegisterExternalFunction(slot_getattr);

  // retrieve variable
  value = irb->CreateCall(
      llvm_slot_getattr, {
          llvm_slot,
          llvm_variable_varattno,
          llvm_isnull_arg /* TODO: Fix isNull */ });
  return true;
}
