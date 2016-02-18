#include "codegen/init_codegen.h"
#include <cstdio>
#include <cassert>

int main()
{
	const int numAttr = 2;
	FormData_pg_attribute attr_structs[numAttr];
	Form_pg_attribute attrs[numAttr] {
		&attr_structs[0],
		&attr_structs[1],
	};

	struct tupleDesc tupDesc;
	tupDesc.natts = numAttr;
	tupDesc.attrs = attrs;

	for (int attnum = 0; attnum < numAttr; ++attnum)
	{
		attrs[attnum]->attalign = 's';
		attrs[attnum]->attlen = sizeof(int16);
	}

	assert(InitCodeGen() == 0);
	void *code_gen = ConstructCodeGenerator();

	GenerateSlotDeformTuple(code_gen, &tupDesc);
	PrepareForExecution(code_gen);
	auto fn = GetSlotDeformTupleFunction(code_gen);
	int16 data[2] = {1, 5};

	int64 values[2] = {0, 0};

	fn(reinterpret_cast<char*>(data), values);

	assert(values[0] == data[0]);
	assert(values[1] == data[1]);

	return 0;
}
