/**----------------------------------------------------------------------------
 * asm_function.cpp
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 2014:2:12 15:41 created
**---------------------------------------------------------------------------*/
#pragma once

#include "asm_function.h"

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
NTSTATUS read_msr(IN UINT32 msr_index, OUT PMSR msr)
{
#if defined(_WIN64)
	ULONG64 tmp = x64_read_msr(msr_index);
	msr->low = (tmp & 0x00000000ffffffff);
	msr->high = (tmp >> 32);

#elif defined(_X86_)
	MSR msr_tmp = {0};
	x86_read_msr(msr_index, &msr_tmp);	
	msr->low = msr_tmp.low;
	msr->high = msr_tmp.high;

#endif
	
	return STATUS_SUCCESS;
}

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
NTSTATUS write_msr(IN UINT32 msr_index, IN const PMSR msr)
{	
#if defined(_WIN64)
	x64_write_msr(msr_index, msr->low, msr->high);
	
#elif defined(_X86_)	
	x86_write_msr(msr_index, msr->high, msr->low);	

#endif

	//> verify
	MSR v_msr = {0};
	read_msr(msr_index, &v_msr);
	
	if (v_msr.high == msr->high && v_msr.low == msr->low)
	{
		return STATUS_SUCCESS;
	}
	else
	{
		return STATUS_UNSUCCESSFUL;
	}
}
