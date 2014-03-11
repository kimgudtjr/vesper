/**----------------------------------------------------------------------------
 * Debugger.cpp
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 2014:1:27 22:53 created
**---------------------------------------------------------------------------*/
#include "stdafx.h"
#include "Debugger.h"

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
DWORD __stdcall DebugCoreCallback(_In_ const DEBUG_EVENT* debug_event, _In_ DWORD_PTR tag)
{
	_ASSERTE(NULL != debug_event);
	_ASSERTE(NULL != tag);
	if (NULL == debug_event || NULL == tag) return DBG_EXCEPTION_NOT_HANDLED;
	
	Debugger* debugger = (Debugger*) tag;
	DWORD ret = debugger->handle_debug_event(debug_event);

	//> debuggee is terminating
	if (EXIT_PROCESS_DEBUG_EVENT == debug_event->dwDebugEventCode)
	{
		debugger->stop();
	}

	return ret;	
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
Debugger::Debugger()
	:
	_initialized(false),
	_stop_debuggee(false),	
	_initial_bp_seen(false)
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
Debugger::~Debugger()
{
	finalize();
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
bool Debugger::initialize()
{
	_ASSERTE(true != _initialized);
	if (true == _initialized) return false;

	//> intialize database logger
	if (true != _logger.intialize())
	{
		log_err L"_logger.intialize()" log_end
		return false;
	}		

	//> start vesper service
	if (true != _vesper_support.initialize())
	{
		log_err L"_vesper_support.initialize()" log_end
		return false;
	}

	//> initailize DebugCore
	if (true != _core.initialize(DebugCoreCallback, (DWORD_PTR)this))
	{
		log_err L"_core.initialize( callback = 0x%p )", DebugCoreCallback log_end
		return false;
	}
	log_info L"debugger core initialized successfully..." log_end

	_initialized = true;
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
void Debugger::finalize()
{
	if (true != _initialized) return;

	stop();
	_core.finalize();	

	//> unload vesper service
	_vesper_support.finalize();

	//> free BreakPoint objects
	std::list<BreakPoint*>::iterator its = _bp_list.begin();
	std::list<BreakPoint*>::iterator ite = _bp_list.end();
	for( ; its != ite; ++its)
	{
		BreakPoint* bp = *its;
		delete bp;
	}
	_bp_list.clear();

	//> free thread objects
	_thread_list.clear();

	//> free dll objects
	_dll_list.clear();

	//> finalize database logger
	_logger.finalize();

	_initialized = false;
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
Debugger::start_and_wait(
	_In_ const wchar_t* debuggee_path, 
	_In_ const wchar_t* parameters
	)
{
	_ASSERTE(true == _initialized);
	if (true != _initialized) return false;

	if (true != _core.start_debug(debuggee_path, parameters))
	{
		log_err 
			L"_core.create_debuggee( debuggee = %s, parameter = %s )", 
			debuggee_path, 
			parameters
		log_end
		return false;
	}

	//> wait for 
	//>		- debuggee is exiting
	//>		- user wants to stop debugging
	for (;;)
	{
		if (true != _stop_debuggee) 
		{
			Sleep(1);
		}
		else
		{
			break;
		}
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
void Debugger::stop()
{
	_core.stop_debug();
	_stop_debuggee = true;
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
DWORD Debugger::handle_debug_event(_In_ const DEBUG_EVENT* debug_event)
{
	DWORD ret = DBG_CONTINUE;
	
	switch(debug_event->dwDebugEventCode)
	{
	case EXCEPTION_DEBUG_EVENT:		return handle_exception(debug_event->dwProcessId, debug_event->dwThreadId, &debug_event->u.Exception);
	case CREATE_THREAD_DEBUG_EVENT: return handle_create_thread(debug_event->dwProcessId, debug_event->dwThreadId, &debug_event->u.CreateThread);
	case CREATE_PROCESS_DEBUG_EVENT:return handle_create_process(debug_event->dwProcessId, debug_event->dwThreadId, &debug_event->u.CreateProcessInfo);
	case EXIT_THREAD_DEBUG_EVENT:	return handle_exit_thread(debug_event->dwProcessId, debug_event->dwThreadId, &debug_event->u.ExitThread);
	case EXIT_PROCESS_DEBUG_EVENT:	return handle_exit_process(debug_event->dwProcessId, debug_event->dwThreadId, &debug_event->u.ExitProcess);
	case LOAD_DLL_DEBUG_EVENT:		return handle_load_dll(debug_event->dwProcessId, debug_event->dwThreadId, &debug_event->u.LoadDll);
	case UNLOAD_DLL_DEBUG_EVENT:	return handle_unload_dll(debug_event->dwProcessId, debug_event->dwThreadId, &debug_event->u.UnloadDll);
	case OUTPUT_DEBUG_STRING_EVENT:	return handle_output_debug_string(_process_info.cpdi.hProcess, &debug_event->u.DebugString);
	case RIP_EVENT:					return handle_rip(debug_event->dwProcessId, debug_event->dwThreadId, &debug_event->u.RipInfo);
	default:
		break;		
	}

	return ret;
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
DWORD 
Debugger::handle_exception(
	_In_ DWORD pid, 
	_In_ DWORD tid, 
	_In_ const EXCEPTION_DEBUG_INFO* edi
	)
{
	DWORD ret = DBG_EXCEPTION_NOT_HANDLED;

	switch (edi->ExceptionRecord.ExceptionCode)
	{
	case EXCEPTION_BREAKPOINT:
		ret = handle_exception_breakpoint(pid, tid, edi);
		break;
	case EXCEPTION_SINGLE_STEP:
		ret = handle_single_step(pid, tid, edi);
		break;
	default:
		ret = handle_general_exception(pid, tid, edi);
		break;
	}
	return ret;
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
DWORD 
Debugger::handle_exception_breakpoint(
	_In_ DWORD pid, 
	_In_ DWORD tid, 
	_In_ const EXCEPTION_DEBUG_INFO* edi
	)
{
	UNREFERENCED_PARAMETER(pid);
	_ASSERTE(EXCEPTION_BREAKPOINT == edi->ExceptionRecord.ExceptionCode);

	DWORD ret = DBG_CONTINUE;

	if (false == _initial_bp_seen)
	{
		_initial_bp_seen = true;		
		log_info 
			L"initial breakpoint hit at 0x%08x", 
			edi->ExceptionRecord.ExceptionAddress
		log_end
	}
	else
	{
		HANDLE hthread = get_thread_handle(tid);
		if (NULL == hthread) return DBG_EXCEPTION_NOT_HANDLED;
		
		CONTEXT context = {0};
		//context.ContextFlags = CONTEXT_FULL;
		context.ContextFlags = CONTEXT_ALL;
		if (TRUE != GetThreadContext(hthread, &context))
		{
			log_err
				L"GetThreadContext( tid = 0x%08x ), gle = %u", 
				tid, 
				GetLastError()
			log_end
			return DBG_EXCEPTION_NOT_HANDLED;
		}


		std::list<BreakPoint*>::iterator its = _bp_list.begin();
		std::list<BreakPoint*>::iterator ite = _bp_list.end();
		for( ; its != ite; ++its)
		{
			BreakPoint* bp = *its;
			if (bp->get_address() == (DWORD_PTR)edi->ExceptionRecord.ExceptionAddress)
			{
				if(true != bp->handle_breakpoint(hthread, context))
				{
					log_err
						L"BreakPoint::handle_breakpoint(), hprocess = 0x%08x, address = 0x%p", 
						bp->get_process_handle(), 
						bp->get_address()
					log_end

					ret = DBG_EXCEPTION_NOT_HANDLED;
					break;
				}

				//> oneshot bp 이면 처리 후 breakpoint 리스트에서 제거
				if (bpt_one_shot == bp->get_type())
				{
					delete bp; 
					_bp_list.erase(its);
					break;
				}
			}
		}

	}

	return ret;
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
DWORD 
Debugger::handle_single_step(
	_In_ DWORD pid, 
	_In_ DWORD tid, 
	_In_ const EXCEPTION_DEBUG_INFO* edi
	)
{
	static UINT_PTR prev_addr = 0x00;

	_ASSERTE(EXCEPTION_SINGLE_STEP == edi->ExceptionRecord.ExceptionCode);

	DWORD ret = DBG_CONTINUE;

	HANDLE hthread = get_thread_handle(tid);
	if (NULL == hthread) return DBG_EXCEPTION_NOT_HANDLED;

	CONTEXT context = {0};
	context.ContextFlags = CONTEXT_ALL;
	if (TRUE != GetThreadContext(hthread, &context))
	{
		log_err
			L"GetThreadContext( tid = 0x%08x ), gle = %u", 
			tid, 
			GetLastError()
		log_end		
		return DBG_EXCEPTION_NOT_HANDLED;
	}

	std::list<BreakPoint*>::iterator its = _bp_list.begin();
	std::list<BreakPoint*>::iterator ite = _bp_list.end();
	for( ; its != ite; ++its)
	{
		BreakPoint* bp = *its;
		if (bp->get_single_step_thread() != hthread) continue;

		_ASSERTE(_process_info.pid == pid);
		SIZE_T bytes_read = 0;
		UINT8 buf[CODE_BUFFER_SIZE] = {0};
		if (TRUE != ReadProcessMemory(
						_process_info.cpdi.hProcess, 
						edi->ExceptionRecord.ExceptionAddress, 
						buf, 
						CODE_BUFFER_SIZE, 
						&bytes_read
						))
		{
			_ASSERTE(!"oops! ReadProcessMemory");
		}

		//> log to database
		_logger.log_exception_info(					
					tid, 
					edi, 
					CODE_BUFFER_SIZE,
					buf);
		
		/*
		UINT_PTR delta = (UINT_PTR)edi->ExceptionRecord.ExceptionAddress - prev_addr ;
		prev_addr = (UINT_PTR)edi->ExceptionRecord.ExceptionAddress;
		disas((UINT_PTR)edi->ExceptionRecord.ExceptionAddress, delta, 16, buf);		
		*/


		//> set rflags.tf and ia32_debug_contrl.btf
		if (true != _vesper_support.enable_btf())
		{
			log_err L"_vesper_support.enable_btf()" log_end
			ret = DBG_CONTINUE;		
		}
		
		if(true != bp->handle_singlestep(hthread, context))
		{
			log_err
				L"BreakPoint::handle_singlestep(), hprocess = 0x%08x, address = 0x%p", 
				bp->get_process_handle(), 
				bp->get_address()
			log_end

			ret = DBG_EXCEPTION_NOT_HANDLED;
			break;
		}

		
	}


	return ret;
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
DWORD 
Debugger::handle_general_exception(
	_In_ DWORD pid, 
	_In_ DWORD tid, 
	_In_ const EXCEPTION_DEBUG_INFO* edi
	)
{
	if (0 != edi->dwFirstChance)
	{
		//> this is firsts chance exception. 
		//> show some information
		log_info 
			L"first chance exception, pid = %u, tid = %u, exception = %s, address = 0x%p", 
			pid, 
			tid, 
			exception_to_string(edi->ExceptionRecord.ExceptionCode),
			edi->ExceptionRecord.ExceptionAddress
		log_end
	}
	else
	{
		log_info 
			L"fatal exception, pid = %u, tid = %u, exception = %s, address = 0x%p", 
			pid, 
			tid, 
			exception_to_string(edi->ExceptionRecord.ExceptionCode),
			edi->ExceptionRecord.ExceptionAddress
		log_end
	}

	//> exception is not handled by me.
	return DBG_EXCEPTION_NOT_HANDLED;
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
DWORD 
Debugger::handle_create_thread(
	_In_ DWORD pid, 
	_In_ DWORD tid, 
	_In_ const CREATE_THREAD_DEBUG_INFO* ctdi
	)
{
	DWORD ret = DBG_CONTINUE;

	ThreadInfo ti(tid, ctdi);
	_thread_list.push_back(ti);

	log_info 
		L"create thread, pid = %u, tid = %u, ctdi->tid = %u, ctdi->handle = 0x%p, ctdi->local = 0x%p, ctdi->start = 0x%p", 
		pid, 
		tid, 
		ti.tid,
		ti.handle, 
		ti.local_base, 
		ti.start_address
	log_end
	
	do 
	{
		//BreakPoint* bp = new OneShotBreakPoint();
		//BreakPoint* bp = new LocationBreakPoint();
		BreakPoint* bp = new TraceBreakPoint();
		if (true != bp->set_breakpoint(
							_process_info.cpdi.hProcess, 
							(DWORD_PTR)ti.start_address))
		{
			log_err
				L"bp.set_breakpoint( hproc = 0x%08x, addr = 0x%p, bpt_location )", 
				_process_info.cpdi.hProcess, ti.start_address
			log_end
			break;
		}

		_bp_list.push_back(bp);
	} while (false);

	return ret;
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
DWORD 
Debugger::handle_create_process(
	_In_ DWORD pid, 
	_In_ DWORD tid, 
	_In_ const CREATE_PROCESS_DEBUG_INFO* cpdi
	)
{
	_process_info.initialize(pid, tid, cpdi);

	log_info
		L"create process, pid = 0x%08x, handle = 0x%p, image base = 0x%p, start = 0x%p, image name = %s", 
		_process_info.pid, 
		_process_info.tid, 
		_process_info.cpdi.lpBaseOfImage, 
		_process_info.cpdi.lpStartAddress, 
		_process_info.cpdi.lpImageName
	log_end

	//> simulate thread creat event for the first thread
	CREATE_THREAD_DEBUG_INFO ctdi;
    ctdi.hThread = cpdi->hThread;
    ctdi.lpStartAddress = cpdi->lpStartAddress;
	ctdi.lpThreadLocalBase = cpdi->lpThreadLocalBase;
	return handle_create_thread(pid, tid, &ctdi);
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
DWORD 
Debugger::handle_exit_thread(
	_In_ DWORD pid, 
	_In_ DWORD tid, 
	_In_ const EXIT_THREAD_DEBUG_INFO* etdi
	)
{
	DWORD ret = DBG_CONTINUE;

	std::list<ThreadInfo>::iterator is = _thread_list.begin();
	std::list<ThreadInfo>::iterator ie = _thread_list.end();			
	for(; is != ie; ++is)
	{
		if (tid == is->tid)
		{
			log_dbg
				L"thread exit, pid = %u, tid = %u, exit code = 0x%08x", 
				pid, 
				tid, 						
				etdi->dwExitCode
			log_end

			_thread_list.erase(is);
			break;
		}
	}

	return ret;
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
DWORD 
Debugger::handle_exit_process(
	_In_ DWORD pid, 
	_In_ DWORD tid, 
	_In_ const EXIT_PROCESS_DEBUG_INFO* epdi
	)
{
	DWORD ret = DBG_CONTINUE;

	log_info 
		L"exit process, pid = %u, tid = %u, exit code = 0x%08x", 
		pid, 
		tid, 
		epdi->dwExitCode
	log_end

	return ret;
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
DWORD 
Debugger::handle_load_dll(
	_In_ DWORD pid, 
	_In_ DWORD tid, 
	_In_ const LOAD_DLL_DEBUG_INFO* lddi
	)
{
	UNREFERENCED_PARAMETER(pid);
	UNREFERENCED_PARAMETER(tid);

	DWORD ret = DBG_CONTINUE;
	
	DllInfo dll_info;			
	if (true == dll_info.initialize(lddi))
	{
		_dll_list.push_back(dll_info);
	}	

	//log_info
	//	L"load dll, pid = %u, tid = %u, file handle = 0x%p, base = 0x%p, unicode = %s, name = %s",
	//	pid, 
	//	tid, 
	//	dll_info.file_handle, 
	//	dll_info.dll_base_address, 
	//	true == dll_info.is_unicode ? L"true" : L"false", 
	//	dll_info.image_name.c_str()
	//log_end

#pragma TODO("서로다른 모듈이 같은 주소에 load / unload 를 반복하는 경우 어떻하지?")
	_logger.log_module_load(dll_info.image_name, dll_info.dll_base_address);
	
	return ret;
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
DWORD 
Debugger::handle_unload_dll(
	_In_ DWORD pid, 
	_In_ DWORD tid, 
	_In_ const UNLOAD_DLL_DEBUG_INFO* uddi
	)
{
	UNREFERENCED_PARAMETER(pid);
	UNREFERENCED_PARAMETER(tid);

	DWORD ret = DBG_CONTINUE;

	std::list<DllInfo>::iterator is = _dll_list.begin();
	std::list<DllInfo>::iterator ie = _dll_list.end();
	for(; is != ie; ++is)
	{
		if (is->dll_base_address == (DWORD_PTR)uddi->lpBaseOfDll)
		{
			//log_info
			//	L"uload dll, pid = %u, tid = %u, name = %s", 
			//	pid, 
			//	tid, 
			//	is->image_name.c_str()
			//log_end
			_dll_list.erase(is);
			break;
		}
	}

	return ret;
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
DWORD 
Debugger::handle_output_debug_string(
	_In_ HANDLE process_handle,	
	_In_ const OUTPUT_DEBUG_STRING_INFO* odsi
	)
{
	DWORD ret = DBG_CONTINUE;

	//> calculate buffer size for ods message.
	//> OUTPUT_DEBUG_STRING_INFO::nDebugStringLength includes null char, but
	//> I added additionl NULL char.
	ULONG_PTR ods_size = 0;
	if (0 == odsi->fUnicode)
	{
		//> ansi
		ods_size = (odsi->nDebugStringLength + 1) * sizeof(char);
	}
	else
	{
		//> unicode
		ods_size = (odsi->nDebugStringLength + 1) * sizeof(wchar_t);
	}
	#pragma warning(disable: 4127)
	do 
	{
		raii_void_ptr ods_buf( malloc(ods_size), raii_free );
		if (NULL == ods_buf.get())
		{
			log_err
				L"insufficient memory, malloc( %u )", 
				ods_size
			log_end
			break;
		}

		ULONG_PTR cb_read = 0;
		if (TRUE != ReadProcessMemory(
						process_handle, 
						odsi->lpDebugStringData, 
						ods_buf.get(), 
						ods_size, 
						&cb_read))
		{
			log_err 
				L"ReadProcessMemory( process handle = 0x%p, addr = 0x%p, size = %u ), gle = %u",
				process_handle, 
				odsi->lpDebugStringData, 
				ods_size, 
				GetLastError()
			log_end
			break;
		}
		
		if (0 == odsi->fUnicode)
		{
			log_info L"ods, %S", ods_buf.get() log_end
		}
		else
		{
			log_info L"ods, %s", ods_buf.get() log_end
		}
		
	} while (false);
	#pragma warning(default: 4127)
	
	return ret;
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
DWORD 
Debugger::handle_rip(
	_In_ DWORD pid, 
	_In_ DWORD tid, 
	_In_ const RIP_INFO* rip)
{
	DWORD ret = DBG_CONTINUE;

	log_info
		L"rip, pid = %u, tid = %u, err = 0x%08x, type = 0x%08x", 
		pid, 
		tid, 
		rip->dwError, 
		rip->dwType
	log_end

	return ret;
}


 /**
  * @brief	
  * @param	
  * @see		
  * @remarks	original code from 
  * @remarks	Debugging Applications for Microsoft .NET and Microsoft Windows
  * @remarks	Copyright ?1997-2003 John Robbins
  * @code		
  * @endcode	
  * @return	
 **/
 const wchar_t* Debugger::exception_to_string(_In_ DWORD exception_code)
 {
    switch(exception_code)
    {
    case EXCEPTION_ACCESS_VIOLATION: return _T("Access Violation");
    case EXCEPTION_DATATYPE_MISALIGNMENT: return _T("Datatype Misalignment");
    case EXCEPTION_BREAKPOINT: return _T("Breakpoint");
    case EXCEPTION_SINGLE_STEP: return _T("Single Step");
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return _T("Array Bounds Exceeded");
    case EXCEPTION_FLT_DENORMAL_OPERAND: return _T("Floating-Point Denormal Operand");
    case EXCEPTION_FLT_DIVIDE_BY_ZERO: return _T("Floating-Point Divide By Zero");
    case EXCEPTION_FLT_INEXACT_RESULT: return _T("Floating-Point Inexact Result");
    case EXCEPTION_FLT_INVALID_OPERATION: return _T("Floating-Point Invalid Operation");
    case EXCEPTION_FLT_OVERFLOW: return _T("Floating-Point Overflow");
    case EXCEPTION_FLT_STACK_CHECK: return _T("Floating-Point Stack Check");
    case EXCEPTION_FLT_UNDERFLOW: return _T("Floating-Point Underflow");
    case EXCEPTION_INT_DIVIDE_BY_ZERO: return _T("Integer Divide By Zero");
    case EXCEPTION_INT_OVERFLOW: return _T("Integer Overflow");
    case EXCEPTION_PRIV_INSTRUCTION: return _T("Privileged Instruction");
    case EXCEPTION_IN_PAGE_ERROR: return _T("Page In Error");
    case EXCEPTION_ILLEGAL_INSTRUCTION: return _T("Illegal Instruction");
	case EXCEPTION_NONCONTINUABLE_EXCEPTION: return _T("Noncontinuable exception");
    case EXCEPTION_STACK_OVERFLOW: return _T("Stack Overflow");
    case EXCEPTION_INVALID_DISPOSITION: return _T("Invalid Disposition");
    case EXCEPTION_GUARD_PAGE: return _T("Guard Page");
    case EXCEPTION_INVALID_HANDLE: return _T("Invalid Handle");
    case CONTROL_C_EXIT: return _T("Ctrl+C Exit");
    case 0XC0000135: return _T("DLL Not Found");
    case 0XC0000142: return _T("DLL Initialization Failed");
    case 0XC06D007E: return _T("Module Not Found");
    case 0xc06d007f: return _T("Procedure Not Found");
    case 0xe06d7363: return _T("Microsoft C++ Exception");
    case STATUS_NO_MEMORY: return _T("No Memory");
    default: return _T("!!!!<<UNKNOWN EXCEPTION>>!!!!");
    }
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
HANDLE Debugger::get_thread_handle(_In_ DWORD tid)
{
	std::list<ThreadInfo>::iterator its = _thread_list.begin();
	std::list<ThreadInfo>::iterator ite = _thread_list.end();
	for( ;its != ite; ++its)
	{
		if (its->tid == tid)
		{
			return its->handle;
		}
	}

	return NULL;
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
bool Debugger::disas(_In_ UINT_PTR base_address, _In_ UINT_PTR delta, _In_ UINT32 buf_len, _In_ const UINT8* buf)
{
	// The number of the array of instructions the decoder function will use to 
	// return the disassembled instructions.
	// Play with this value for performance...
	#define MAX_INSTRUCTIONS (10)

	_DecodedInst disassembled[MAX_INSTRUCTIONS];
	unsigned int decodedInstructionsCount = 0;
	_OffsetType offset = 0;

	_DecodeResult res = distorm_decode(
							offset,
							(const unsigned char*)buf,
							buf_len,
							Decode64Bits,
							disassembled,
							MAX_INSTRUCTIONS,
							&decodedInstructionsCount
							);

	if (DECRES_SUCCESS != res) return false;

	log_info
		L"0x%p (%08d)  (%02d)  %-24S  %S%s%S",
		base_address + disassembled[0].offset,
		delta,
		disassembled[0].size,
		(char*)disassembled[0].instructionHex.p,
		(char*)disassembled[0].mnemonic.p,
		disassembled[0].operands.length != 0 ? L" " : L"",
		(char*)disassembled[0].operands.p
	log_end
	/*
	for (int i = 0; i < decodedInstructionsCount; i++) 
	{
		printf(
			"%08I64x (%02d) %-24s %s%s%s\r\n",
			disassembled[i].offset,
			disassembled[i].size,
			(char*)disassembled[i].instructionHex.p,
			(char*)disassembled[i].mnemonic.p,
			disassembled[i].operands.length != 0 ? " " : "",
			(char*)disassembled[i].operands.p);
	}
	*/
	return true;

}