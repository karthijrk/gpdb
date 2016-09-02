/*
 * int4mul.h
 *
 *  Created on: Sep 2, 2016
 *      Author: krajaraman
 */

#ifndef SRC_BACKEND_CODEGEN_INCLUDE_CODEGEN_INT4MUL_H_
#define SRC_BACKEND_CODEGEN_INCLUDE_CODEGEN_INT4MUL_H_

#include <llvm/Pass.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/FormattedStream.h>
#include <llvm/Support/MathExtras.h>
#include <algorithm>

namespace llvm {
llvm::Function* GenerateInt4Mul(llvm::Module* mod);
}

#endif /* SRC_BACKEND_CODEGEN_INCLUDE_CODEGEN_INT4MUL_H_ */
