//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2016 Pivotal Software, Inc.
//
//	@filename:
//		codegen_wrapper.h
//
//	@doc:
//		C wrappers for initialization of code generation.
//
//---------------------------------------------------------------------------
#ifndef CODEGEN_WRAPPER_H_
#define CODEGEN_WRAPPER_H_

#include "pg_config.h"

/*
 * Code that needs to be shared irrespective of whether USE_CODEGEN is enabled or not.
 */
struct TupleTableSlot;
typedef void (*SlotDeformTupleFn) (struct TupleTableSlot *slot, int natts);

#ifndef USE_CODEGEN

#define InitCodeGen();
#define CodeGeneratorManager_Create(module_name) NULL
#define CodeGeneratorManager_GenerateCode(manager);
#define CodeGeneratorManager_PrepareGeneratedFunctions(manager) true
#define CodeGeneratorManager_NotifyParameterChange(manager) true
#define CodeGeneratorManager_Destroy(manager);
#define GetActiveCodeGeneratorManager() NULL
#define SetActiveCodeGeneratorManager(manager);

#define START_CODE_GENERATOR_MANAGER(newManager)
#define END_CODE_GENERATOR_MANAGER()

#define init_codegen()
#define call_slot_deform_tuple(slot, attno) slot_deform_tuple(slot, attno)
#define enroll_slot_deform_tuple_codegen(regular_func, ptr_to_chosen_func, slot)

#else

#ifndef __cplusplus
#include "c.h"
#endif

/*
 * Forward extern declaration of slot deform tuple if code gen is enabled
 */
extern void slot_deform_tuple(struct TupleTableSlot *slot, int natts);

/*
 * @brief Life span of Code generator instance
 *
 * @note Each code generator is responsible to generate code for one specific function.
 *       Each generated function has a life span to indicate to the manager about when to
 *       invalidate and regenerate this function. The enroller is responsible to know
 *       how long a generated code should be valid.
 *
 *
 */
typedef enum CodeGenFuncLifespan
{
	// Does not depend on parameter changes
	CodeGenFuncLifespan_Parameter_Invariant,
	// Has to be regenerated as the parameter changes
	CodeGenFuncLifespan_Parameter_Variant
} CodeGenFuncLifespan;


#ifdef __cplusplus
extern "C" {
#endif

/*
 * Do one-time global initialization of LLVM library. Returns 0
 * on success, nonzero on error.
 */
int
InitCodeGen();

/*
 * Creates a manager for an operator
 */
void*
CodeGeneratorManager_Create(const char* module_name);

/*
 * Calls all the registered CodeGenInterface to generate code
 */
bool
CodeGeneratorManager_GenerateCode(void* manager);

/*
 * Compiles and prepares all the CodeGen function pointers
 */
bool
CodeGeneratorManager_PrepareGeneratedFunctions(void* manager);

/*
 * Notifies a manager that the underlying operator has a parameter change
 */
bool
CodeGeneratorManager_NotifyParameterChange(void* manager);

/*
 * Destroys a manager for an operator
 */
void
CodeGeneratorManager_Destroy(void* manager);

/*
 * Get the active code generator manager
 */
void*
GetActiveCodeGeneratorManager();

/*
 * Set the active code generator manager
 */
void
SetActiveCodeGeneratorManager(void* manager);

/*
 * returns the pointer to the CodeGenFuncInfo
 */
void*
SlotDeformTupleCodeGen_Enroll(SlotDeformTupleFn regular_func_ptr,
                              SlotDeformTupleFn* ptr_to_regular_func_ptr,
                              struct TupleTableSlot* slot);


#ifdef __cplusplus
}  // extern "C"
#endif

/*
 * START_CODE_GENERATOR_MANAGER would switch to the specified code generator manager,
 * saving the oldCodeGeneratorManager. Must be paired with END_CODE_GENERATOR_MANAGER
 */
#define START_CODE_GENERATOR_MANAGER(newManager)  \
	do { \
		void *oldManager = NULL; \
		Assert(newManager != NULL); \
		oldManager = GetActiveCodeGeneratorManager(); \
		SetActiveCodeGeneratorManager(newManager);\
/*
 * END_CODE_GENERATOR_MANAGER would restore the previous code generator manager that was
 * active at the time of START_CODE_GENERATOR_MANAGER call
 */
#define END_CODE_GENERATOR_MANAGER()  \
		SetActiveCodeGeneratorManager(oldManager);\
	} while (0);


/*
 * Initialize LLVM library
 */
#define init_codegen() \
	if (codegen) { \
			if (InitCodeGen() != 0) { \
				ereport(FATAL, \
					(errcode(ERRCODE_INTERNAL_ERROR), \
					errmsg("failed to initialize LLVM library"), \
					errhint("LLVM library for code generation failed " \
						"to initialize. You may wish to disable " \
						"code generation by turning off the " \
						"\"codegen\" GUC."))); \
			} \
		} \

/*
 * Call slot_deform_tuple using function pointer slot_deform_tuple_fn.
 * Function pointer may point to regular version or generated function
 */
#define call_slot_deform_tuple(slot, attno) \
		slot->slot_deform_tuple_gen_info.slot_deform_tuple_fn(slot, attno)

/*
 * Enroll given slot to codegen manager.
 * The enrollment process also ensures that the slot_deform_tuple_fn pointer
 * is set to the regular version initially
 */
#define enroll_slot_deform_tuple_codegen(regular_func, ptr_to_regular_func_ptr, slot) \
		slot->slot_deform_tuple_gen_info.code_generator = SlotDeformTupleCodeGen_Enroll( \
				regular_func, ptr_to_regular_func_ptr, slot); \
		Assert(slot->slot_deform_tuple_gen_info.slot_deform_tuple_fn == regular_func); \


#endif //USE_CODEGEN

#endif  // CODEGEN_WRAPPER_H_
