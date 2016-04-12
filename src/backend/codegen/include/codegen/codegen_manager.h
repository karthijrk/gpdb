//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2016 Pivotal Software, Inc.
//
//	@filename:
//		codegen_manager.h
//
//	@doc:
//		Object that manage all CodeGen and CodegenUtils
//
//---------------------------------------------------------------------------

#ifndef CODEGEN_MANAGER_H_
#define CODEGEN_MANAGER_H_

#include <memory>
#include <vector>

#include "codegen/utils/macros.h"
#include "codegen/codegen_wrapper.h"

namespace gpcodegen {
/** \addtogroup gpcodegen
 *  @{
 */

// Forward declaration of CodegenUtils to manage llvm module
class CodegenUtils;

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
	explicit CodeGenManager();

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
	// CodegenUtils provides a facade to LLVM subsystem.
	std::unique_ptr<gpcodegen::CodegenUtils> codegen_utils_;

	// List of all enrolled code generators.
	std::vector<std::unique_ptr<CodeGenInterface>> enrolled_code_generators_;

	DISALLOW_COPY_AND_ASSIGN(CodeGenManager);

};

/** @} */

}
#endif  // CODEGEN_MANAGER_H_
