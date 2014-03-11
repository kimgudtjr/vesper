/**----------------------------------------------------------------------------
 * vesper_support.cpp
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 2014:2:12 22:31 created
**---------------------------------------------------------------------------*/
#include "DriverHeaders.h"
#include "vesper_support.h"
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
NTSTATUS enable_btf()
{
	MSR msr = {0};
	NTSTATUS status = read_msr(MSR_IA32_DEBUGCTL, &msr);
	if (TRUE!=NT_SUCCESS(status))
	{
		log_err "read_msr(MSR_IA32_DEBUGCTL), status = 0x%08x", status log_end
		return status;
	}

	if (MSR_IA32_DEBUGCTL_BTF != (msr.low & MSR_IA32_DEBUGCTL_BTF))
	{
		msr.low |= MSR_IA32_DEBUGCTL_BTF;
		status = write_msr(MSR_IA32_DEBUGCTL, &msr);
		if (TRUE != NT_SUCCESS(status))
		{
			log_err "write_msr(IA32_DEBUGCTL_BTF), status = 0x%08x", status log_end
			return status;
		}

#ifdef DBG
		//read_msr(MSR_IA32_DEBUGCTL, &msr);
		//log_info "set msr low = 0x%08x, high = 0x%08x", msr.low, msr.high log_end
#endif	
	}
	
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
NTSTATUS disable_btf()
{
	MSR msr = {0};
	NTSTATUS status = read_msr(MSR_IA32_DEBUGCTL, &msr);
	if (TRUE!=NT_SUCCESS(status))
	{
		log_err "read_msr(MSR_IA32_DEBUGCTL), status = 0x%08x", status log_end
		return status;
	}

	if (MSR_IA32_DEBUGCTL_BTF == (msr.low & MSR_IA32_DEBUGCTL_BTF))
	{
		msr.low &= ~MSR_IA32_DEBUGCTL_BTF;
		status = write_msr(MSR_IA32_DEBUGCTL, &msr);
		if (TRUE != NT_SUCCESS(status))
		{
			log_err "write_msr(IA32_DEBUGCTL_BTF), status = 0x%08x", status log_end
			return status;
		}

#ifdef DBG
		read_msr(MSR_IA32_DEBUGCTL, &msr);
		log_info "set msr low = 0x%08x, high = 0x%08x", msr.low, msr.high log_end
#endif	
	}	

	return STATUS_SUCCESS;

}