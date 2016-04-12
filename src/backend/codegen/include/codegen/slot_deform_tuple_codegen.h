//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2016 Pivotal Software, Inc.
//
//	@filename:
//		slot_deform_tuple_codegen.h
//
//	@doc:
//		Headers for slot_deform_tuple codegen
//
//---------------------------------------------------------------------------

#ifndef SLOT_DEFORM_TUPLE_CODEGEN_H_
#define SLOT_DEFORM_TUPLE_CODEGEN_H_

#include "codegen/codegen_wrapper.h"
#include "codegen/base_codegen.h"

namespace gpcodegen {

/** \addtogroup gpcodegen
 *  @{
 */

class SlotDeformTupleCodeGen: public BaseCodeGen<SlotDeformTupleFn> {
public:
	/**
	 * @brief Constructor
	 *
	 * @param slot         The slot to use for generating code.
	 * @param regular_func_ptr       Regular version of the generated function.
	 * @param ptr_to_chosen_func_ptr Reference to the function pointer that the caller will call.
	 *
	 * @note 	The ptr_to_chosen_func_ptr can refer to either the generated function or the
	 * 			corresponding regular version.
	 *
	 **/
	explicit SlotDeformTupleCodeGen(TupleTableSlot* slot,
			SlotDeformTupleFn regular_func_ptr,
			SlotDeformTupleFn* ptr_to_regular_func_ptr);

	virtual ~SlotDeformTupleCodeGen() = default;

protected:
	virtual bool DoCodeGeneration(gpcodegen::CodegenUtils* codegen_utils)
			override final;

private:
	TupleTableSlot* slot_;

	static constexpr char kSlotDeformTupleNamePrefix[] = "slot_deform_tuple";

	/**
	 * @brief Generates runtime code that calls slot_deform_tuple as an external function.
	 *
	 * @param codegen_utils Utility to ease the code generation process.
	 * @return true on successful generation.
	 **/
	bool GenerateSimpleSlotDeformTuple(gpcodegen::CodegenUtils* codegen_utils);
};

/** @} */

}
#endif // SLOT_DEFORM_TUPLE_CODEGEN_H_
