/**----------------------------------------------------------------------------
 * DebugCore.h
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 2014:1:27 22:40 created
**---------------------------------------------------------------------------*/
#pragma once

/**
 * @brief	debug event 처리를 위한 콜백함수
 * @return	continue status value (e.g. DBG_CONTINUE, DBG_EXCEPTION_NOT_HANDLED)
**/
typedef 
DWORD (__stdcall *func_debug_callback)(
	_In_ const DEBUG_EVENT* debug_event,
	_In_ DWORD_PTR tag
	);

class DebugCore : private boost::noncopyable
{
public:
	DebugCore();	
	~DebugCore();

	bool initialize(_In_ func_debug_callback debug_callback, _In_ DWORD_PTR tag);
	void finalize();
	
	bool start_debug(_In_ const wchar_t* debuggee_path, _In_ const wchar_t* parameters);
	void stop_debug();

private:
	bool					_initialized;

	func_debug_callback		_debug_callback;
	DWORD_PTR				_debug_callback_tag;

	volatile bool			_stop_debug_loop;	
	boost::thread*			_debug_thread;

private:
	bool create_debuggee(_In_ const wchar_t* debuggee_path, _In_ const wchar_t* commandline);
	void debugger_thread(_In_ const wchar_t* debuggee_path, _In_ const wchar_t* parameters);	
};

