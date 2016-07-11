//---------------------------------------------------------------------------
//  Greenplum Database
//  Copyright 2016 Pivotal Software, Inc.
//
//  @filename:
//    gp_codegen_utils.h
//
//  @doc:
//    Object that extends the functionality of CodegenUtils by adding GPDB
//    specific functionality and utilities to aid in the runtime code generation
//    for a LLVM module
//
//  @test:
//
//
//---------------------------------------------------------------------------

#ifndef GPCODEGEN_GP_CODEGEN_UTILS_H_  // NOLINT(build/header_guard)
#define GPCODEGEN_GP_CODEGEN_UTILS_H_

#include "codegen/utils/codegen_utils.h"
#include "codegen/codegen_wrapper.h"

extern "C" {
#include "utils/elog.h"
}

namespace gpcodegen {

class GpCodegenUtils : public CodegenUtils {
 public:
  /**
   * @brief Constructor.
   *
   * @param module_name A human-readable name for the module that this
   *        CodegenUtils will manage.
   **/
  explicit GpCodegenUtils(llvm::StringRef module_name)
  : CodegenUtils(module_name) {
  }

  ~GpCodegenUtils() {
  }

  /*
   * @brief Create LLVM instructions to call elog_start() and elog_finish().
   *
   * @warning This method does not create instructions for any sort of exception
   *          handling. In the case that elog throws an error, the code with jump
   *          straight out of the compiled module back to the last location in GPDB
   *          that setjump was called.
   *
   * @param llvm_elevel llvm::Value pointer to an integer representing the error level
   * @param llvm_fmt    llvm::Value pointer to the format string
   * @tparam args       llvm::Value pointers to arguments to elog()
   */
  template<typename... V>
  void CreateElog(
      llvm::Value* llvm_elevel,
      llvm::Value* llvm_fmt,
      const V ... args ) {
    assert(NULL != llvm_elevel);
    assert(NULL != llvm_fmt);

    llvm::Function* llvm_elog_start =
        GetOrRegisterExternalFunction(elog_start);
    llvm::Function* llvm_elog_finish =
        GetOrRegisterExternalFunction(elog_finish);

    ir_builder()->CreateCall(
        llvm_elog_start, {
            GetConstant(""),   // Filename
            GetConstant(0),    // line number
            GetConstant("")    // function name
        });
    ir_builder()->CreateCall(
        llvm_elog_finish, {
            llvm_elevel,
            llvm_fmt,
            args...
        });
  }

  /*
   * @brief Create LLVM instructions to call elog_start() and elog_finish().
   *        A convenient alternative that automatically converts an integer elevel and
   *        format string to LLVM constants.
   *
   * @warning This method does not create instructions for any sort of exception
   *          handling. In the case that elog throws an error, the code with jump
   *          straight out of the compiled module back to the last location in GPDB
   *          that setjump was called.
   *
   * @param elevel Integer representing the error level
   * @param fmt    Format string
   * @tparam args  llvm::Value pointers to arguments to elog()
   */
  template<typename... V>
    void CreateElog(
        int elevel,
        const char* fmt,
        const V ... args ) {
    CreateElog(GetConstant(elevel), GetConstant(fmt), args...);
  }

  /**
   * @brief Create a Cast instruction to convert given llvm::Value of Datum type
   *        to given Cpp type
   *
   * @note Depend on cpp type's size, it will do trunc or bit cast or
   *       int2ptr cast. This is same as converting gpdb's Datum to cpptype.
   *
   * @tparam CppType  Destination cpp type
   * @param value     LLVM Value on which casting has to be done.
   *
   * @return LLVM Value that casted to given Cpp type.
   **/
  template <typename CppType>
  llvm::Value* CreateDatumToCppTypeCast(llvm::Value* value) {
    assert(nullptr != value);
    llvm::Type* llvm_dest_type = GetType<CppType>();
    // If it is a pointer, do direct pointer cast
    if (llvm_dest_type->isPointerTy()) {
      return ir_builder()->CreateIntToPtr(value, llvm_dest_type);
    }
    unsigned dest_size = llvm_dest_type->getScalarSizeInBits();
    assert(0 != dest_size);

    llvm::Type* llvm_src_type = value->getType();
    unsigned src_size = llvm_src_type->getScalarSizeInBits();

    // Given src value, it should be of type Datum(int64_t)
    assert(llvm_src_type->isIntegerTy());
    assert(src_size == sizeof(Datum)*8);

    // Datum size should be the largest
    assert(dest_size <= src_size);

    // Get dest type as integer type with same size
    llvm::Type* llvm_dest_as_int_type = llvm::IntegerType::get(*context(),
                                                             dest_size);

    llvm::Value* llvm_casted_value = value;

    if (dest_size < src_size) {
      llvm_casted_value = ir_builder()->CreateTrunc(value,
                                                    llvm_dest_as_int_type);
    }

    if (llvm_src_type->getTypeID() != llvm_dest_type->getTypeID()) {
      return ir_builder()->CreateBitCast(llvm_casted_value, llvm_dest_type);
    }

    return llvm_casted_value;
  }

  /**
   * @brief Create a Cast instruction to convert given llvm::Value of any type
   *        to Datum
   *
   * @note Depend on given value's type size, it will do extent or bit cast or
   *       ptr2int cast. This is same as converting any cpptype to Datum in gpdb.
   *
   * @param value     LLVM Value on which casting has to be done.
   * @param is_src_unsigned true if given value is of unsigned type
   *
   *
   * @return LLVM Value that casted to Datum type.
   **/
  llvm::Value* CreateCppTypeToDatumCast(llvm::Value* value,
                                        bool is_src_unsigned=false);

  /**
   * @brief Create a Cast instruction to convert given llvm::Value to given Cpp
   *        type
   *
   * @note Depend on type's size, it will do extent or trunc or bit cast. This
   *       is same as converting gpdb's Datum to cpptype and viceversa.
   *
   * @tparam CppType  Destination cpp type
   * @param value LLVM Value on which casting has to be done.
   *
   * @return LLVM Value that casted to given Cpp type.
   **/
  template <typename CppType>
  llvm::Value* CreateDatumCast(llvm::Value* value, bool is_src_unsigned=false) {
    assert(nullptr != value);
    llvm::Type* llvm_src_type = value->getType();
    unsigned src_size = llvm_src_type->getScalarSizeInBits();
    if (llvm_src_type->isPointerTy()) {
      src_size = llvm_src_type->getPointerElementType()->getScalarSizeInBits();
    }

    llvm::Type* llvm_dest_type = GetType<CppType>();
    unsigned dest_size = llvm_dest_type->getScalarSizeInBits();
    if (llvm_dest_type->isPointerTy()) {
      dest_size = llvm_dest_type->getPointerElementType()->getScalarSizeInBits();
    }

    assert(0 != src_size);
    assert(0 != dest_size);

    llvm::Type* llvm_dest_size_type = llvm::IntegerType::get(*context(),
                                                             dest_size);

    // Convert given value to int type to do ext / trunc
    llvm::Value* llvm_int_value = value;
    if (llvm_src_type->isPointerTy()) {
      llvm_int_value = ir_builder()->CreatePtrToInt(value, llvm_dest_size_type);
    }
    else if (!llvm_src_type->isIntegerTy()) {
      llvm_int_value = ir_builder()->CreateBitCast(
          value, llvm::IntegerType::get(*context(), src_size));
    }
    llvm::Value* llvm_size_value = llvm_int_value;
    if (src_size < dest_size) {
      // Do signed extension for signed integer type whose size is greater than
      // 1. For bool we create integer type of size 1 and hence we don't do
      // signed extension.
      if (llvm_src_type->isIntegerTy() &&
          src_size > 1 &&
          !is_src_unsigned) {
        llvm_size_value = ir_builder()->CreateSExt(llvm_int_value, llvm_dest_size_type);
      }
      else {
        llvm_size_value = ir_builder()->CreateZExt(llvm_int_value, llvm_dest_size_type);
      }
    } else if (src_size > dest_size) {
      llvm_size_value = ir_builder()->CreateTrunc(llvm_int_value, llvm_dest_size_type);
    }
    if (llvm_dest_type->isPointerTy()) {
      return ir_builder()->CreateIntToPtr(llvm_int_value, llvm_dest_type);
    }
    else if (llvm_src_type->getTypeID() != llvm_dest_type->getTypeID()) {
      return ir_builder()->CreateBitCast(llvm_size_value, llvm_dest_type);
    }
    return llvm_size_value;
  }

};

}  // namespace gpcodegen

#endif  // GPCODEGEN_GP_CODEGEN_UTILS_H
// EOF
