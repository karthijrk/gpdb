//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2012 Greenplum, Inc.
//
//	@filename:
//		CGPOptimizer.cpp
//
//	@doc:
//		Entry point to GP optimizer
//
//	@test:
//
//
//---------------------------------------------------------------------------

#define ALLOW_abort

#include "gpopt/CGPOptimizer.h"
#include "gpopt/utils/COptTasks.h"

// the following headers are needed to reference optimizer library initializers
#include "naucrates/init.h"
#include "gpopt/init.h"
#include "gpos/_api.h"

#include "naucrates/exception.h"

//---------------------------------------------------------------------------
//	@function:
//		CGPOptimizer::TouchLibraryInitializers
//
//	@doc:
//		Touch library initializers to enforce linker to include them
//
//---------------------------------------------------------------------------
void
CGPOptimizer::TouchLibraryInitializers()
{
	void (*gpos)(gpos_init_params*) = gpos_init;
	void (*dxl)() = gpdxl_init;
	void (*opt)() = gpopt_init;
}


//---------------------------------------------------------------------------
//	@function:
//		CGPOptimizer::PlstmtOptimize
//
//	@doc:
//		Optimize given query using GP optimizer
//
//---------------------------------------------------------------------------
PlannedStmt *
CGPOptimizer::PplstmtOptimize
	(
	Query *pquery,
	bool *pfUnexpectedFailure // output : set to true if optimizer unexpectedly failed to produce plan
	)
{
	COptTasks::SOptContext octx;
	GPOS_TRY
	{
		return COptTasks::PplstmtOptimize(pquery, &octx, pfUnexpectedFailure);
	}
	GPOS_CATCH_EX(ex)
	{
		if (GPOS_MATCH_EX(ex, gpdxl::ExmaDXL, gpdxl::ExmiWarningAsError))
		{
		  Assert(NULL != octx.m_szErrorMsg);
		  elog(ERROR, "PQO unable to generate plan, %s", octx.m_szErrorMsg);
		}
		else if (GPOS_MATCH_EX(ex, gpdxl::ExmaGPDB, gpdxl::ExmiGPDBError))
		{
		  elog(ERROR, "GPDB exception. Aborting PQO plan generation.");
		}
		else if (GPOS_MATCH_EX(ex, gpdxl::ExmaGPDB, gpdxl::ExmiNoAvailableMemory))
		{
		  elog(ERROR, "PQO unable to generate plan, no available memory to allocate string buffer.");
		}
		else if (GPOS_MATCH_EX(ex, gpdxl::ExmaGPDB, gpdxl::ExmiInvalidComparisonTypeCode))
		{
		  elog(ERROR, "PQO unable to generate plan, Invalid comparison type code. Valid values are Eq, NEq, LT, LEq, GT, GEq.");
		}
	}
	GPOS_CATCH_END;
	return NULL;
}


//---------------------------------------------------------------------------
//	@function:
//		CGPOptimizer::SzDXL
//
//	@doc:
//		Serialize planned statement into DXL
//
//---------------------------------------------------------------------------
char *
CGPOptimizer::SzDXLPlan
	(
	Query *pquery
	)
{
	return COptTasks::SzOptimize(pquery);
}

//---------------------------------------------------------------------------
//	@function:
//		InitGPOPT()
//
//	@doc:
//		Initialize GPTOPT and dependent libraries
//
//---------------------------------------------------------------------------
void
CGPOptimizer::InitGPOPT ()
{
  // Use GPORCA's default allocators
  struct gpos_init_params params = { NULL, NULL };
  gpos_init(&params);
  gpdxl_init();
  gpopt_init();
}

//---------------------------------------------------------------------------
//	@function:
//		TerminateGPOPT()
//
//	@doc:
//		Terminate GPOPT and dependent libraries
//
//---------------------------------------------------------------------------
void
CGPOptimizer::TerminateGPOPT ()
{
  gpopt_terminate();
  gpdxl_terminate();
  gpos_terminate();
}

//---------------------------------------------------------------------------
//	@function:
//		PplstmtOptimize
//
//	@doc:
//		Expose GP optimizer API to C files
//
//---------------------------------------------------------------------------
extern "C"
{
PlannedStmt *PplstmtOptimize
	(
	Query *pquery,
	bool *pfUnexpectedFailure
	)
{
	return CGPOptimizer::PplstmtOptimize(pquery, pfUnexpectedFailure);
}
}

//---------------------------------------------------------------------------
//	@function:
//		SzDXLPlan
//
//	@doc:
//		Serialize planned statement to DXL
//
//---------------------------------------------------------------------------
extern "C"
{
char *SzDXLPlan
	(
	Query *pquery
	)
{
	return CGPOptimizer::SzDXLPlan(pquery);
}
}

//---------------------------------------------------------------------------
//	@function:
//		InitGPOPT()
//
//	@doc:
//		Initialize GPTOPT and dependent libraries
//
//---------------------------------------------------------------------------
extern "C"
{
void InitGPOPT ()
{
	return CGPOptimizer::InitGPOPT();
}
}

//---------------------------------------------------------------------------
//	@function:
//		TerminateGPOPT()
//
//	@doc:
//		Terminate GPOPT and dependent libraries
//
//---------------------------------------------------------------------------
extern "C"
{
void TerminateGPOPT ()
{
	return CGPOptimizer::TerminateGPOPT();
}
}

// EOF
