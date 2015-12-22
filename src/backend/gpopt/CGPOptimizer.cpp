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

#include "gpopt/CGPOptimizer.h"
#include "gpopt/utils/COptTasks.h"

// the following headers are needed to reference optimizer library initializers
#include "naucrates/init.h"
#include "gpopt/init.h"
#include "gpos/_api.h"
#include "gpopt/mdcache/CMDCache.h"

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
	void (*gpos)() = gpos_init;
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
	return COptTasks::PplstmtOptimize(pquery, pfUnexpectedFailure);
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
//		LogOptimizerMDCacheSize
//
//	@doc:
//		Print MD cache details
//
//---------------------------------------------------------------------------
extern "C"
{
void
LogOptimizerMDCacheSize(int log_level)
{
	if (NULL != gpopt::CMDCache::Pcache()) {
			ULLONG entries = gpopt::CMDCache::Pcache()->UlpEntries();
			ULLONG total_size = gpopt::CMDCache::Pcache()->UllTotalAllocatedSize();
			elog(log_level, "MD Cache : size = %llu MB, # of bytes = %llu bytes, # of entries = %lu",
					(total_size / (1024 * 1024)), total_size, entries);
		}
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

// EOF
