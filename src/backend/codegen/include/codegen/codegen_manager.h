//---------------------------------------------------------------------------
//  Greenplum Database
//  Copyright (C) 2016 Pivotal Software, Inc.
//
//  @filename:
//    codegen_manager.h
//
//  @doc:
//    Object that manage all CodeGen and CodeGenUtils
//
//---------------------------------------------------------------------------

#ifndef GPCODEGEN_CODEGEN_MANAGER_H_  // NOLINT(build/header_guard)
#define GPCODEGEN_CODEGEN_MANAGER_H_

#include <memory>
#include <vector>

#include "codegen/utils/macros.h"
#include "codegen/codegen_wrapper.h"

namespace gpcodegen {
/** \addtogroup gpcodegen
 *  @{
 */

// Forward declaration of CodeGenUtils to manage llvm module
class CodeGenUtils;

// Forward declaration of a CodeGenInterface that will be managed by manager
class CodeGenInterface;

/**
 * @brief Object that manages all code gen.
 **/
class CodeGenManager {
 public:
  /**
   * @brief Constructor.
   *
   **/
  CodeGenManager();

  ~CodeGenManager() = default;

  /**
   * @brief Enroll a code generator with manager
   *
   * @note Manager manages the memory of enrolled generator.
   *
   * @param funcLifespan Based on life span corresponding CodeGen_Utils will be used to generate
   * @param generator    Generator that needs to be enrolled with manager.
   * @return true on successful enrollment.
   **/
  bool EnrollCodeGenerator(CodeGenFuncLifespan funcLifespan,
                           CodeGenInterface* generator);

  /**
   * @brief Make all enrolled generators to generate code.
   *
   * @return The number of enrolled codegen that successfully generated code.
   **/
  size_t GenerateCode();

  /**
   * @brief Compile all the generated functions. On success, caller gets
   * 		to call the generated method.
   *
   * @return true on successful compilation or return false
   **/
  bool PrepareGeneratedFunctions();

  /**
   * @brief 	Notifies the manager of a parameter change.
   *
   * @note 	This is called during a ReScan or other parameter change process.
   * 			Upon receiving this notification the manager may invalidate all the
   * 			generated code that depend on parameters.
   *
   **/
  void NotifyParameterChange();

  /**
   * @brief Invalidate all generated functions.
   *
   * @return true if successfully invalidated.
   **/
  bool InvalidateGeneratedFunctions();

  /**
   * @return Number of enrolled generators.
   **/
  size_t GetEnrollmentCount() {
    return enrolled_code_generators_.size();
  }

 private:
  // CodeGenUtils provides a facade to LLVM subsystem.
  std::unique_ptr<gpcodegen::CodeGenUtils> codegen_utils_;

  // List of all enrolled code generators.
  std::vector<std::unique_ptr<CodeGenInterface>> enrolled_code_generators_;

  DISALLOW_COPY_AND_ASSIGN(CodeGenManager);
};

/** @} */

}  // namespace gpcodegen
#endif  // GPCODEGEN_CODEGEN_MANAGER_H_
