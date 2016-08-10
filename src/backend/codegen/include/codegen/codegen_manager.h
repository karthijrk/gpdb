//---------------------------------------------------------------------------
//  Greenplum Database
//  Copyright (C) 2016 Pivotal Software, Inc.
//
//  @filename:
//    codegen_manager.h
//
//  @doc:
//    Object that manage all CodegenInterface and GpCodegenUtils
//
//---------------------------------------------------------------------------

#ifndef GPCODEGEN_CODEGEN_MANAGER_H_  // NOLINT(build/header_guard)
#define GPCODEGEN_CODEGEN_MANAGER_H_

#include <memory>
#include <vector>
#include <string>

#include "codegen/utils/macros.h"
#include "codegen/codegen_wrapper.h"
#include "codegen/codegen_callsite.h"

namespace gpcodegen {
/** \addtogroup gpcodegen
 *  @{
 */

// Forward declaration of GpCodegenUtils to manage llvm module
class GpCodegenUtils;
class CodegenCallsiteInterface;
class CodegenInterface;

/**
 * @brief Object that manages all code gen.
 **/
class CodegenManager {
 public:
  /**
   * @brief Constructor.
   *
   * @param module_name A human-readable name for the module that this
   *        CodegenManager will manage.
   **/
  explicit CodegenManager(const std::string& module_name);

  ~CodegenManager() = default;

  /**
   * @brief Enroll a CodegenCallsite with manager
   *
   * @note Manager manages the memory of enrolled callsite object.
   *
   * @tparam CodegenType Type of Code Generator class
   * @tparam FuncType Type of the regular function
   * @tparam Args Variable argument that ClassType will take in its constructor
   *
   * @param funcLifespan Life span of the enrolling callsite object. Based on life span,
   *                     corresponding GpCodegenUtils will be used for code generation
   * @param regular_func_ptr Regular version of the target function.
   * @param ptr_to_chosen_func_ptr Reference to the function pointer that the caller will call.
   * @param args Variable length argument for ClassType
   *
   * @return true on successful enrollment.
   **/
  template <typename CodegenType, typename FuncType, typename ...Args>
  CodegenCallsiteInterface* EnrollCodegenCallsite(CodegenFuncLifespan funcLifespan,
                                                  FuncType regular_func_ptr,
                                                  FuncType* ptr_to_chosen_func_ptr,
                                                  Args&&... args) {  // NOLINT(build/c++11)
    CodegenType* generator = new CodegenType(
        this, std::forward<Args>(args)...);
    CodegenCallsiteInterface* callsite =
        new CodegenCallsite<CodegenType, FuncType>(
            this,
            generator,
            regular_func_ptr,
            ptr_to_chosen_func_ptr);
    // Only CodegenFuncLifespan_Parameter_Invariant is supported as of now
    assert(funcLifespan == CodegenFuncLifespan_Parameter_Invariant);
    assert(nullptr != callsite);
    enrolled_callsites_.emplace_back(callsite);
    return callsite;
  }

  bool EnrollSharedCodegen(CodegenInterface* generator);

  /**
   * @brief Request all enrolled callsites to generate code.
   *
   * @return The number of enrolled codegen that successfully generated code.
   **/
  unsigned int GenerateCode();

  /**
   * @brief Compile all the generated functions. On success,
   *        a pointer to the generated method becomes available to the caller.
   *
   * @return The number of enrolled codegen that successully generated code
   *         and 0 on failure
   **/
  unsigned int PrepareGeneratedFunctions();

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
   * @return Number of enrolled callsites.
   **/
  size_t GetEnrollmentCount() {
    return enrolled_callsites_.size();
  }

  /*
   * @brief Accumulate the explain string with a dump of all the underlying LLVM
   *        modules
   */
  void AccumulateExplainString();

  /*
   * @brief Return the previous accumulated explain string
   */
  const std::string& GetExplainString();

 private:
  // GpCodegenUtils provides a facade to LLVM subsystem.
  std::unique_ptr<gpcodegen::GpCodegenUtils> codegen_utils_;

  std::string module_name_;

  // List of all enrolled codegen callsites
  std::vector<std::unique_ptr<CodegenCallsiteInterface>> enrolled_callsites_;

  // List of all generator cached
  std::vector<std::unique_ptr<CodegenInterface>> cached_code_generators_;

  // Holds the dumped IR of all underlying modules for EXPLAIN CODEGEN queries
  std::string explain_string_;

  DISALLOW_COPY_AND_ASSIGN(CodegenManager);
};

/** @} */

}  // namespace gpcodegen
#endif  // GPCODEGEN_CODEGEN_MANAGER_H_
