/**----------------------------------------------------------------------------
 * breakpoint.cpp
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 2014:2:3 15:07 created
**---------------------------------------------------------------------------*/
#include "stdafx.h"
#include "breakpoint.h"
#include "cpu_helper.h"

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
BreakPoint::BreakPoint()
: 
_hprocess(NULL),
_type(bpt_none),
_state(bps_none),
_address(0), 
_opcode(0x00),
_single_step_thread(NULL)
{

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
BreakPoint::~BreakPoint()
{
	
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
bool 
BreakPoint::set_breakpoint(
	_In_ HANDLE hprocess, 
	_In_ DWORD_PTR address
	)
{
	_ASSERTE(NULL != hprocess);
	_ASSERTE(0 != address);
	if (NULL == hprocess || 0 == address) return false;

	_hprocess = hprocess;
	_address = address;
	
	bool ret = false;
	ch_param param = {0};
	param.hproc = _hprocess;
	ret = set_break_point(&param, _address, &_opcode);
	if (true != ret)
	{
		log_err
			L"set_break_point( hproc = 0x%08x, addr = 0x%p )", 
			hprocess, 
			_address
		log_end
		
		_state = bps_unresolved;
	}
	else
	{
		_state = bps_enabled;
	}
	
	return ret;
}

/**
 * @brief	clear breakpoint
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool BreakPoint::clear_breakpoint()
{
	if (!(bps_enabled == _state || bps_wait_for_trace == _state)) return true;
	if (NULL == _hprocess || 0 == _address) return true;

	ch_param param={0};
	param.hproc = _hprocess;	
	bool ret = ::clear_break_point(&param, _address, _opcode, false);
	if (true != ret)
	{
		log_err
			L"clear_break_point( hproc = 0x%08x, addr = 0x%p )", 
			_hprocess, 
			_address
		log_end
	}

	return ret;
	
}

/******************************************************************************
 * OneShotBreakPoint class
 *****************************************************************************/

OneShotBreakPoint::OneShotBreakPoint()
{
	_type = bpt_one_shot;
}

OneShotBreakPoint::~OneShotBreakPoint()
{
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
bool 
OneShotBreakPoint::handle_breakpoint(
	_In_ HANDLE hthread, 
	_In_ CONTEXT& context
	)
{
	ch_param param = {0};
	param.hproc = _hprocess;
	param.hthread = hthread;
	param.context = context;

	//> restore original opcode and move eip register back.
	bool ret = ::clear_break_point(&param, _address, _opcode, true);
	if (true != ret)
	{
		log_err
			L"clear_break_point( hproc = 0x%08x, addr = 0x%p )", 
			_hprocess, 
			_address
		log_end
		return false;
	}

	return true;
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
bool 
OneShotBreakPoint::handle_singlestep(
	_In_ HANDLE hthread, 
	_In_ CONTEXT& context
	)
{
	_ASSERTE(!"never called in OneShotBreakPoint");
	return true;
}



/******************************************************************************
 * LocationBreakPoint class
 *****************************************************************************/

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
LocationBreakPoint::LocationBreakPoint()
{
	_type = bpt_location;
}
LocationBreakPoint::~LocationBreakPoint()
{

}
/**
 * @brief	
 * @param	
 * @see		
 * @remarks	#1 원래 코드로 복원 & --eip & state = bps_wait_for_trace
 * @remarks	#2 single step thread 저장 & single step 활성화
 * @remarks	#3 run
 * @remarks	#4 single step handler 에서 
 * @remarks	#4-1 single step thread 비교
 * @remarks	#4-2 re-install bp & state = bps_enable
 * @remarks	#4-3 run
 * @code		
 * @endcode	
 * @return	
**/
bool 
LocationBreakPoint::handle_breakpoint(
	_In_ HANDLE hthread, 
	_In_ CONTEXT& context
	)
{
	_single_step_thread = hthread;

	ch_param param = {0};
	param.hproc = _hprocess;
	param.hthread = hthread;
	param.context = context;
	if (true != clear_break_point(&param, _address, _opcode, true))
	{
		log_err
			L"clear_break_point( hproc = 0x%08x, addr = 0x%p )", 
			_hprocess, 
			_address
		log_end
		return false;
	}

	if (true != set_single_step(&param))
	{
		log_err
			L"set_single_step( hproc = 0x%08x )", 
			_hprocess
		log_end
		return false;
	}

	_state = bps_wait_for_trace;
	log_info 
		L"hthread = 0x%p, address = 0x%p", hthread, _address
	log_end
		
	return true;
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
bool 
LocationBreakPoint::handle_singlestep(
	_In_ HANDLE hthread, 
	_In_ CONTEXT& context
	)
{
	_ASSERTE(bpt_location == get_type());
	_ASSERTE(bps_wait_for_trace == get_state());
	_ASSERTE(hthread == get_single_step_thread());
	if(	bpt_location != get_type() ||
		bps_wait_for_trace != get_state() ||
		hthread != get_single_step_thread() )
	{
		return false;
	}

	if (true != set_breakpoint(_hprocess, _address))
	{
		log_err 
			L"set_breakpoint( hprocess = 0x%08x, address = 0x%p )", 
			_hprocess, 
			_address
		log_end
		return false;
	}	
	
	log_info 
		L"hthread = 0x%p, address = 0x%p", hthread, _address
	log_end

	return true;
}


/******************************************************************************
 * TraceBreakPoint class
 *****************************************************************************/

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
TraceBreakPoint::TraceBreakPoint()
{
	_type = bpt_trace;	
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
TraceBreakPoint::~TraceBreakPoint()
{
	
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
bool TraceBreakPoint::handle_breakpoint(_In_ HANDLE hthread, _In_ CONTEXT& context)
{
	_single_step_thread = hthread;

	ch_param param = {0};
	param.hproc = _hprocess;
	param.hthread = hthread;
	param.context = context;
	if (true != clear_break_point(&param, _address, _opcode, true))
	{
		log_err
			L"clear_break_point( hproc = 0x%08x, addr = 0x%p )", 
			_hprocess, 
			_address
		log_end
		return false;
	}

	/*
	if (true != set_last_branch_enable(&param))
	{
		log_err
			L"set_last_branch_enable( hproc = 0x%08x )", 
			_hprocess
		log_end
		return false;
	}
	*/
	if (true != set_single_step(&param))
	{
		log_err
			L"set_single_step( hproc = 0x%08x )", 
			_hprocess
		log_end
		return false;
	}

	_state = bps_wait_for_trace;
	
	log_info 
		L"hthread = 0x%p, address = 0x%p", hthread, _address
	log_end
		
	return true;
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
bool TraceBreakPoint::handle_singlestep(_In_ HANDLE hthread, _In_ CONTEXT& context)
{
	_ASSERTE(bps_wait_for_trace == get_state());
	_ASSERTE(hthread == get_single_step_thread());
	if(	bps_wait_for_trace != get_state() ||
		hthread != get_single_step_thread() )
	{
		return false;
	}

	ch_param param = {0};
	param.hproc = _hprocess;
	param.hthread = hthread;
	param.context = context;

	//DebugBreak();

	/*
	if (true != set_last_branch_enable(&param))
	{
		log_err
			L"set_last_branch_enable( hproc = 0x%08x )", 
			_hprocess
		log_end
		return false;
	}
	*/
	if (true != set_single_step(&param))
	{
		log_err
			L"set_single_step( hproc = 0x%08x )", 
			_hprocess
		log_end
		return false;
	}

	_state = bps_wait_for_trace;	
	
    /*
	param.context.DebugControl;
    param.context.LastBranchToRip;
    param.context.LastBranchFromRip;
    param.context.LastExceptionToRip;
    param.context.LastExceptionFromRip;
	*/



	return true;
}