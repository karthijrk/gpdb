#include "postgres.h"

#include "codegen/codegen_wrapper.h"

// Do one-time global initialization of LLVM library. Returns 0
// on success, nonzero on error.
int
InitCodeGen()
{
	elog(ERROR, "mock implementation of InitCodeGen called");
	return 0;
}

// creates a manager for an operator
void*
CodeGeneratorManager_Create()
{
	elog(ERROR, "mock implementation of CodeGeneratorManager_Create called");
	return NULL;
}

// calls all the registered CodeGenFuncInfo to generate code
bool
CodeGeneratorManager_GenerateCode(void* manager)
{
	elog(ERROR, "mock implementation of CodeGeneratorManager_GenerateCode called");
	return true;
}

// compiles and prepares all the code gened function pointers
bool
CodeGeneratorManager_PrepareGeneratedFunctions(void* manager)
{
	elog(ERROR, "mock implementation of CodeGeneratorManager_PrepareGeneratedFunctions called");
	return true;
}

// notifies a manager that the underlying operator has a parameter change
bool
CodeGeneratorManager_NotifyParameterChange(void* manager)
{
	elog(ERROR, "mock implementation of CodeGeneratorManager_NotifyParameterChange called");
	return true;
}

// destroys a manager for an operator
void
CodeGeneratorManager_Destroy(void* manager)
{
	elog(ERROR, "mock implementation of CodeGeneratorManager_Destroy called");
}

// get the active code generator manager
void*
GetActiveCodeGeneratorManager()
{
	elog(ERROR, "mock implementation of GetActiveCodeGeneratorManager called");
	return NULL;
}

// set the active code generator manager
void
SetActiveCodeGeneratorManager(void* manager)
{
	elog(ERROR, "mock implementation of SetActiveCodeGeneratorManager called");
}

// returns the pointer to the CodeGenFuncInfo
void*
SlotDeformTupleCodeGen_Enroll(struct TupleTableSlot* slot,
    SlotDeformTupleFn regular_func_ptr,
    SlotDeformTupleFn* ptr_to_regular_func_ptr)
{
  *ptr_to_regular_func_ptr = regular_func_ptr;
	elog(ERROR, "mock implementation of SlotDeformTupleCodeGen_Enroll called");
	return NULL;
}

