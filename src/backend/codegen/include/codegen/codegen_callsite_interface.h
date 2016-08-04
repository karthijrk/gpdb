//---------------------------------------------------------------------------
//  Greenplum Database
//  Copyright (C) 2016 Pivotal Software, Inc.
//
//  @filename:
//    codegen_callsite_interface.h
//
//  @doc:
//    Interface for all code generator for call sites from gpdb
//
//---------------------------------------------------------------------------
#ifndef GPCODEGEN_CODEGEN_CALLSITE_INTERFACE_H_  // NOLINT(build/header_guard)
#define GPCODEGEN_CODEGEN_CALLSITE_INTERFACE_H_

#include <string>
#include <vector>

namespace gpcodegen {

/** \addtogroup gpcodegen
 *  @{
 */

// Forward declaration
class GpCodegenUtils;

/**
 * @brief Interface for all code generators.
 **/
class CodegenCallsiteInterface {
 public:
  virtual ~CodegenCallsiteInterface() = default;

  /**
   * @brief Hook to request for and initialize any dependencies
   *
   * @return true on success
   */
  virtual bool InitDependencies() = 0;

  /**
   * @brief Generates specialized code at run time.
   *
   *
   * @param codegen_utils Utility to ease the code generation process.
   * @return true on successful generation.
   **/
  virtual bool GenerateCode(gpcodegen::GpCodegenUtils* codegen_utils) = 0;

  /**
   * @brief Sets up the caller to use the corresponding regular version of the
   *        target function.
   *
   *
   * @return true on setting to regular version.
   **/
  virtual bool SetToRegular() = 0;

  /**
   * @brief Sets up the caller to use the generated function instead of the
   *        regular version.
   *
   * @param codegen_utils Facilitates in obtaining the function pointer from
   *        the compiled module.
   * @return true on successfully setting to generated functions
   **/
  virtual bool SetToGenerated(gpcodegen::GpCodegenUtils* codegen_utils) = 0;

  /**
   * @brief Resets the state of the generator, including reverting back to
   *        the regular version of the function.
   *
   **/
  virtual void Reset() = 0;

  /**
   * @return true if the code generation is successful.
   *
   **/
  virtual bool IsGenerated() const = 0;
};

/** @} */

}  // namespace gpcodegen

#endif  // GPCODEGEN_CODEGEN_CALLSITE_INTERFACE_H_
