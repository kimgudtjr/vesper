/**----------------------------------------------------------------------------
 * debug_core.h
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 2014:1:27 22:51 created
**---------------------------------------------------------------------------*/
#pragma once

#include "vesper_support.h"
#include "distorm.h"

#include "debug_core.h"
#include "branch_log.h"
#include "breakpoint.h"

#include "process_thread_info.h"
#include "module_info.h"

DWORD __stdcall DebugCoreCallback(_In_ const DEBUG_EVENT* debug_event, _In_ DWORD_PTR tag);

/**
 * @brief	
**/
class Debugger
{
public:
	Debugger();
	~Debugger();

	bool initialize(_In_ const wchar_t* db_path);
	void finalize();

	bool start_and_wait(_In_ const wchar_t* debuggee_path, _In_ const wchar_t* parameters);
	void stop();
	
private:
	bool					_initialized;
	volatile bool			_stop_debuggee;
	vesper_support			_vesper_support;


	DebugCore				_core;
	BranchLogger			_logger;

	bool					_initial_bp_seen;	
	ProcessInfo				_process_info;
	std::list<ThreadInfo>	_thread_list;
	std::list<ModuleInfo>	_dll_list;

	std::list<BreakPoint*>	_bp_list;
	
	friend 
	DWORD __stdcall DebugCoreCallback(_In_ const DEBUG_EVENT* debug_event, _In_ DWORD_PTR tag);
	
	DWORD handle_debug_event(_In_ const DEBUG_EVENT* debug_event);

	DWORD handle_exception(_In_ DWORD pid, _In_ DWORD tid, _In_ const EXCEPTION_DEBUG_INFO* edi);
	DWORD handle_exception_breakpoint(_In_ DWORD pid, _In_ DWORD tid, _In_ const EXCEPTION_DEBUG_INFO* edi);
	DWORD handle_single_step(_In_ DWORD pid, _In_ DWORD tid, _In_ const EXCEPTION_DEBUG_INFO* edi);
	DWORD handle_general_exception(_In_ DWORD pid, _In_ DWORD tid, _In_ const EXCEPTION_DEBUG_INFO* edi);
	DWORD handle_create_thread(_In_ DWORD pid, _In_ DWORD tid, _In_ const CREATE_THREAD_DEBUG_INFO* ctdi);
	DWORD handle_create_process(_In_ DWORD pid, _In_ DWORD tid, _In_ const CREATE_PROCESS_DEBUG_INFO* cpdi);
	DWORD handle_exit_thread(_In_ DWORD pid, _In_ DWORD tid, _In_ const EXIT_THREAD_DEBUG_INFO* etdi);
	DWORD handle_exit_process(_In_ DWORD pid, _In_ DWORD tid, _In_ const EXIT_PROCESS_DEBUG_INFO* epdi);
	DWORD handle_load_dll(_In_ DWORD pid, _In_ DWORD tid, _In_ const LOAD_DLL_DEBUG_INFO* lddi);
	DWORD handle_unload_dll(_In_ DWORD pid, _In_ DWORD tid, _In_ const UNLOAD_DLL_DEBUG_INFO* uddi);
	DWORD handle_output_debug_string(_In_ HANDLE process_handle, _In_ const OUTPUT_DEBUG_STRING_INFO* odsi);
	DWORD handle_rip(_In_ DWORD pid, _In_ DWORD tid, _In_ const RIP_INFO* rip);	

private:
	static const wchar_t* exception_to_string(_In_ DWORD exception_code);
	HANDLE get_thread_handle(_In_ DWORD tid);
	bool disas(_In_ UINT_PTR base_address, _In_ UINT_PTR delta, _In_ UINT32 buf_len, _In_ const UINT8* buf);
	bool get_module_size(_In_ DWORD_PTR module_base, _Out_ DWORD module_size);

};


