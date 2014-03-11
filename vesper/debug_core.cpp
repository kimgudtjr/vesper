/**----------------------------------------------------------------------------
 * debug_core.cpp
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 2014:1:27 22:49 created
**---------------------------------------------------------------------------*/
#include "stdafx.h"
#include "debug_core.h"

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
DebugCore::DebugCore()
:	_initialized(false),
	_debug_callback(NULL),
	_debug_callback_tag(0),
	_stop_debug_loop(false),
	_debug_thread(NULL)
{
	
}

DebugCore::~DebugCore()
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
bool 
DebugCore::initialize(
	_In_ func_debug_callback debug_callback, 
	_In_ DWORD_PTR tag
	)
{
	_ASSERTE(NULL != debug_callback);
	if (NULL == debug_callback) return false;

	_debug_callback = debug_callback;
	_debug_callback_tag = tag;
	
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
void DebugCore::finalize()
{
	if (true != _initialized) return;

	_initialized = false;
	

	stop_debug(); 

	_debug_callback = NULL;
	_debug_callback_tag = NULL;
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
DebugCore::start_debug(
	_In_ const wchar_t* debuggee_path, 
	_In_ const wchar_t* parameters
	)
{
	_ASSERTE(true == _initialized);
	if (true != _initialized)
	{
		log_err L"not initialized" log_end
		return false;
	}

	_ASSERTE(NULL != debuggee_path);
	if (true != is_file_existsW(debuggee_path))
	{
		log_err
			L"no file exists. file path = %s", 
			debuggee_path
		log_end
		return false;
	}

	_ASSERTE(NULL == _debug_thread);
	if (NULL != _debug_thread) 
	{
		log_err L"debug thread already created" log_end
		return false;
	}
	
	//> debug thread ����
	_debug_thread = new boost::thread(boost::bind(
											&DebugCore::debugger_thread, 
											this, 
											debuggee_path, 
											parameters));
	if (NULL == _debug_thread)
	{
		log_err L"creating DebugCore thread failed" log_end
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
void DebugCore::stop_debug()
{
	if (true != _initialized) return;
	if (NULL == _debug_thread) return;

	_stop_debug_loop = true;
	
	Sleep(10);						// wait... some time. :-)
	//_debug_thread->join();		// dead lock! do not join my self!

	delete _debug_thread;
	_debug_thread = NULL;
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
DebugCore::create_debuggee(
	_In_ const wchar_t* debuggee_path, 
	_In_ const wchar_t* commandline
	)
{
	_ASSERTE(NULL != debuggee_path);
	if (NULL == debuggee_path) return false;

#pragma TODO("���� ���μ���(debugger)�� 32 ��Ʈ�̰�, target �� 64 ��Ʈ �� ��쿡 ���� ó���� �ʿ���")
	/*	
	#1 wow �� ����, 64 ��Ʈ app �� ���� 
	#2 debugger �� 64 ��Ʈ������ �����, 64��Ʈ�� ����Ÿ� ���� (�̰� ����� �� ����� ��)

	CWow64Util wow;
	if (TRUE == wow.IsWow64Process(GetCurrentProcess()))
	{
		wow.TurnOffWow64FSRedirection();
	}
	*/

	wchar_t* cmdbuf = NULL;

	if (NULL != commandline)
	{
		size_t length = wcslen(commandline);
		cmdbuf = (wchar_t*) malloc(length);
		if (NULL == cmdbuf)
		{
			log_err L"insufficient resources, malloc( %u )", length log_end
			return false;
		}	

		if (TRUE != SUCCEEDED(StringCbPrintfW(
									cmdbuf, 
									length, 
									L"%s", 
									commandline)))
		{
			log_err 
				L"build command line failed. invalid command line parameters = %s",
				commandline
			log_end	

			free(cmdbuf); cmdbuf = NULL;
			return false;
		}
	}

	STARTUPINFO			si = {0};
	PROCESS_INFORMATION pi = {0};

	if (TRUE != CreateProcessW(
					debuggee_path, 
					cmdbuf, 
					NULL, 
					NULL, 
					FALSE, 
					CREATE_NEW_CONSOLE | DEBUG_ONLY_THIS_PROCESS, 
					NULL, 
					NULL, 
					&si, 
					&pi))
	{
		log_err 
			L"CreateProcessW( path = %s, cmdline = %s ), gle = %u", 
			debuggee_path, commandline, GetLastError() 
		log_end

		free(cmdbuf); cmdbuf = NULL;
		return false;
	}
	log_dbg
		L"debuggee = %s created successfully.", 
		debuggee_path
	log_end

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	free(cmdbuf); cmdbuf = NULL;
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
void 
DebugCore::debugger_thread(
	_In_ const wchar_t* debuggee_path, 
	_In_ const wchar_t* parameters
	)
{
	#pragma TODO("���� ����/���� ���� ������ Ŭ���� ����� ���")

	_ASSERTE(NULL != debuggee_path);	
	if (NULL == debuggee_path || true != is_file_existsW(debuggee_path))
	{
		log_err L"invalid parameter" log_end
		return;
	}
	
	//> create debuggee process
	if (true != create_debuggee(debuggee_path, parameters))
	{
		log_err L"create_debuggee( )" log_end
		return;
	}
	
	DEBUG_EVENT debug_event = {0};	
	while (true != _stop_debug_loop)
	{
		if (TRUE != WaitForDebugEvent(&debug_event, 100)) continue;

		//> debug callback ȣ�� 
		DWORD continue_status = _debug_callback(&debug_event, _debug_callback_tag);

		//> continue_status ����
		//>
		//>	DBG_CONTINUE
		//>		EXCEPTION_DEBUG_EVENT �� ���
		//>			��� exception processing �� ���߰� �����带 ��� ����
		//>		EXCEPTION_DEBUG_EVENT �� �ƴ� ��� 
		//>			������ ��� ���� 
		//>
		//>	DBG_EXCEPTION_NOT_HANDLED
		//>		EXCEPTION_DEBUG_EVENT �� ���
		//>			exception processing �� ��� ����.
		//>			first-chance exception �� ��� seh �ڵ鷯�� search, dispatch ������ ����
		//>			first-chance exception �� �ƴ϶�� ���μ����� ���� ��
		//>		EXCEPTION_DEBUG_EVENT �� �ƴ� ��� 
		//>			������ ��� ���� 
		if (TRUE != ContinueDebugEvent(
						debug_event.dwProcessId, 
						debug_event.dwThreadId, 
						continue_status))
		{
			log_err 
				L"ContinueDebugEvent( pid=%u, tid=%u, status=DBG_CONTINUE )", 
				debug_event.dwProcessId, 
				debug_event.dwThreadId
			log_end			
			
			_stop_debug_loop = true;
			continue;
		}

		//> debuggee is terminating
		if (EXIT_PROCESS_DEBUG_EVENT == debug_event.dwDebugEventCode)
		{
			_stop_debug_loop = true;
			continue;
		}
	}

	log_info L"debug loop returned gracefully." log_end
}

