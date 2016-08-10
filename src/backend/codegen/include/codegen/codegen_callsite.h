//---------------------------------------------------------------------------
//  Greenplum Database
//  Copyright (C) 2016 Pivotal Software, Inc.
//
//  @filename:
//    codegen_callsite.h
//
//  @doc:
//    Codegen Callsite for all enroller from gpdb.
//
//---------------------------------------------------------------------------
#ifndef GPCODEGEN_CODEGEN_CALLSITE_H_  // NOLINT(build/header_guard)
#define GPCODEGEN_CODEGEN_CALLSITE_H_

extern "C" {
#include <utils/elog.h>
}

#include <string>
#include <vector>
#include <memory>
#include "codegen/codegen_callsite_interface.h"
#include "codegen/utils/gp_codegen_utils.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/Verifier.h"

namespace gpcodegen {

/** \addtogroup gpcodegen
 *  @{
 */
class CodegenManager;

/**
 * @brief Base code generator with common implementation that other
 *      code generators can use.
 *
 * @tparam FuncPtrType Function type for regular version of target functions
 *         or generated functions.
 **/
template <typename CodegenType, typename FuncPtrType>
class CodegenCallsite: public CodegenCallsiteInterface {
 public:
  /**
   * @brief Destroys the code generator and reverts back to using regular
   *        version of the target function.
   **/
  virtual ~CodegenCallsite() {
    SetToRegular(regular_func_ptr_, ptr_to_chosen_func_ptr_);
  }

  bool InitDependencies() override {
    return generator_->InitDependencies();
  }

  bool GenerateCode(gpcodegen::GpCodegenUtils* codegen_utils) final {
    return generator_->GenerateCode(codegen_utils);
  }

  bool SetToRegular() final {
    assert(nullptr != regular_func_ptr_);
    SetToRegular(regular_func_ptr_, ptr_to_chosen_func_ptr_);
    return true;
  }

  bool SetToGenerated(gpcodegen::GpCodegenUtils* codegen_utils) final {
    if (false == IsGenerated()) {
      assert(*ptr_to_chosen_func_ptr_ == regular_func_ptr_);
      return false;
    }

    FuncPtrType compiled_func_ptr = codegen_utils->GetFunctionPointer<
        FuncPtrType>(generator_->GetUniqueFuncName());

    if (nullptr != compiled_func_ptr) {
      *ptr_to_chosen_func_ptr_ = compiled_func_ptr;
      return true;
    }
    return false;
  }

  void Reset() final {
    SetToRegular();
    generator_->Reset();
  }

  bool IsGenerated() const final {
    return generator_->IsGenerated();
  }

  /**
   * @return Regular version of the target function.
   *
   **/
  FuncPtrType GetRegularFuncPointer() {
    return regular_func_ptr_;
  }

  /**
   * @brief Sets up the caller to use the corresponding regular version of the
   *        target function.
   *
   * @param regular_func_ptr       Regular version of the target function.
   * @param ptr_to_chosen_func_ptr Reference to caller.
   *
   * @return true on setting to regular version.
   **/
  static bool SetToRegular(FuncPtrType regular_func_ptr,
                           FuncPtrType* ptr_to_chosen_func_ptr) {
    assert(nullptr != ptr_to_chosen_func_ptr);
    assert(nullptr != regular_func_ptr);
    *ptr_to_chosen_func_ptr = regular_func_ptr;
    return true;
  }

 protected:
  /**
   * @brief Constructor
   *
   * @param manager                The manager in which this is enrolled.
   * @param orig_func_name         Original function name.
   * @param regular_func_ptr       Regular version of the target function.
   * @param ptr_to_chosen_func_ptr Reference to the function pointer that the caller will call.
   *
   * @note  The ptr_to_chosen_func_ptr can refer to either the generated function or the
   *      corresponding regular version.
   *
   **/
  explicit CodegenCallsite(gpcodegen::CodegenManager* manager,
                           CodegenType* generator,
                           FuncPtrType regular_func_ptr,
                           FuncPtrType* ptr_to_chosen_func_ptr)  // NOLINT(build/c++11))
  : manager_(manager),
    regular_func_ptr_(regular_func_ptr),
    ptr_to_chosen_func_ptr_(ptr_to_chosen_func_ptr) {
    generator_.reset(generator);
    // Initialize the caller to use regular version of target function.
    SetToRegular(regular_func_ptr, ptr_to_chosen_func_ptr);
  }

  gpcodegen::CodegenManager* manager() const {
    return manager_;
  }

 private:
  CodegenManager* manager_;
  FuncPtrType regular_func_ptr_;
  FuncPtrType* ptr_to_chosen_func_ptr_;
  std::unique_ptr<CodegenType> generator_;

  friend class CodegenManager;
};

/** @} */
}  // namespace gpcodegen

#endif  // GPCODEGEN_CODEGEN_CALLSITE_H_
