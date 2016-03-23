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

#include "c.h"

struct TupleTableSlot;

typedef void (*SlotDeformTupleFn) (struct TupleTableSlot *slot, int natts);

/**
 * @brief Life span of Code generator instance
 *
 * @note Each code generator is responsible to generate code for one specific function.
 *       Each generated function has a life span to indicate to the manager about when to
 *       invalidate and regenerate this function. The enroller is responsible to know
 *       how long a generated code should be valid.
 *
 **/
typedef enum CodeGenFuncLifespan
{
	// does not depend on parameter changes
	CodeGenFuncLifespan_Parameter_Invariant,
	// has to be regenerated as the parameter changes
	CodeGenFuncLifespan_Parameter_Variant
} CodeGenFuncLifespan;

#ifdef __cplusplus
extern "C" {
#endif

// Do one-time global initialization of LLVM library. Returns 0
// on success, nonzero on error.
int InitCodeGen();

// creates a manager for an operator
void* CodeGeneratorManager_Create();

// calls all the registered CodeGenFuncInfo to generate code
bool CodeGeneratorManager_GenerateCode(void* manager);

// compiles and prepares all the code gened function pointers
bool CodeGeneratorManager_PrepareGeneratedFunctions(void* manager);

// notifies a manager that the underlying operator has a parameter change
bool CodeGeneratorManager_NotifyParameterChange(void* manager);

// destroys a manager for an operator
void CodeGeneratorManager_Destroy(void* manager);

// get the active code generator manager
void* GetActiveCodeGeneratorManager();

// set the active code generator manager
void SetActiveCodeGeneratorManager(void* manager);

// returns the pointer to the CodeGenFuncInfo
void* SlotDeformTupleCodeGen_Enroll(struct TupleTableSlot* slot, SlotDeformTupleFn regular_func_ptr, SlotDeformTupleFn* ptr_to_regular_func_ptr);


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


#endif  // CODEGEN_WRAPPER_H_
