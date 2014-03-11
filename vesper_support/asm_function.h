/**----------------------------------------------------------------------------
 * asm_function.h
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 18:10:2013   20:20 created
**---------------------------------------------------------------------------*/
#pragma once

#ifdef __cplusplus 
extern "C" {
#endif
	
#include "arch.h"


void _break();

#if defined(_WIN64)
//> x64 code
	
	ULONG64 x64_read_msr(IN UINT32 msr_index);
	void x64_write_msr(IN UINT32 msr_index, IN UINT32 msr_low, IN UINT32 msr_high);

#elif defined(_X86_)
//> x86 code
	void __stdcall x86_read_msr(IN UINT32 msr_index, OUT MSR* msr);
	void __stdcall x86_write_msr(IN UINT32 msr_index, IN UINT32 msr_low, IN UINT32 msr_high);
#else
	#error !!!UNKNOWN ARCHITECTURE!!!
#endif

NTSTATUS read_msr(IN UINT32 msr_index, OUT PMSR msr);
NTSTATUS write_msr(IN UINT32 msr_index, IN const PMSR msr);





#ifdef __cplusplus 
}
#endif
