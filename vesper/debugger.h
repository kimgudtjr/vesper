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

#include "debug_core.h"
#include "branch_log.h"
#include "breakpoint.h"
#include "vesper_support.h"
#include "distorm.h"


DWORD __stdcall DebugCoreCallback(_In_ const DEBUG_EVENT* debug_event, _In_ DWORD_PTR tag);


/**
 * @brief	process information
 * @brief	pid, tid 정보가 CREATE_PROCESS_DEBUG_INFO 에 없어서 별도 타입으로 정의 함 
**/
class ProcessInfo
{
public:
	ProcessInfo(): pid(0), tid(0)
	{		
	}

	bool initialize(_In_ DWORD _pid, _In_ DWORD _tid, _In_ const CREATE_PROCESS_DEBUG_INFO* _cpdi)
	{
		pid = _pid;
		tid = _tid;
		RtlCopyMemory(&cpdi, _cpdi, sizeof(cpdi));
		return true;
	}

	DWORD pid;
	DWORD tid;
	CREATE_PROCESS_DEBUG_INFO cpdi;
};

/**
 * @brief	create thread information 
 * @brief	CREATE_THREAD_DEBUG_INFO 구조체에 tid 가 없어서 별도 타입으로 정의 함
**/
class ThreadInfo
{
public:
	ThreadInfo(_In_ DWORD _tid, _In_ const CREATE_THREAD_DEBUG_INFO* ctdi )
	: 
	tid(_tid), 
	handle(ctdi->hThread), 
	local_base(ctdi->lpThreadLocalBase), 
	start_address(ctdi->lpStartAddress)
	{
	}

	DWORD tid;
	HANDLE handle;
    LPVOID local_base;
    LPTHREAD_START_ROUTINE start_address;
};

/**
 * @brief	loaded dll information
**/
class DllInfo
{
public:
	DllInfo():
		file_handle(INVALID_HANDLE_VALUE),
		dll_base_address(0),
		debug_info_file_offset(0),
		debug_info_size(0),	
		is_unicode(false)
	{
	}
	~DllInfo()
	{
	}

	bool initialize(_In_ const LOAD_DLL_DEBUG_INFO* dll_debug_info)
	{
		_ASSERTE(NULL != dll_debug_info);
		if (NULL==dll_debug_info) return false;

		file_handle				= dll_debug_info->hFile;
		dll_base_address		= (DWORD_PTR)dll_debug_info->lpBaseOfDll;
		debug_info_file_offset	= dll_debug_info->dwDebugInfoFileOffset;
		debug_info_size			= dll_debug_info->nDebugInfoSize;
		is_unicode				= (0 != dll_debug_info->fUnicode) ? true : false;
	
		if (true != get_filepath_by_handle(file_handle, image_name))
		{
			log_err L"get_filepath_by_handle(proces = 0x%08x, file = 0x%08x)" log_end
			return false;
		}

		return true;
	}

	HANDLE			file_handle;
	DWORD_PTR		dll_base_address;
	DWORD			debug_info_file_offset;
	DWORD			debug_info_size;
	std::wstring	image_name;
	bool			is_unicode;
};


/**
 * @brief	
**/
class Debugger
{
public:
	Debugger();
	~Debugger();

	bool initialize();
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
	std::list<DllInfo>		_dll_list;

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


};


