/**----------------------------------------------------------------------------
 * main.cpp
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 2014:1:9 10:53 created
**---------------------------------------------------------------------------*/
#include "stdafx.h"
#include "Debugger.h"
#include "analyzer.h"


static Debugger _debugger;
static analyzer	_analyzer;

BOOL control_handler(_In_ DWORD control_type);


/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
int _tmain(int argc, _TCHAR* argv[])
{
	int ret = -1;

	//> initialize file log
	std::wstring log_file;
	get_current_module_path(log_file);
	log_file += L".log";
		
	//if (true != slog_initialize(slog_debug, log_to_file | log_to_con, log_file.c_str()))
	if (true != slog_initialize(slog_debug, log_to_file, log_file.c_str()))
	{
		log_err L"slog_initialize( file = %s )", log_file.c_str() log_end
		return -1;
	}
	slog_set_log_format(false, true, true);

	//> parse command line
	//> vesper.exe [-debug] [file_path] [db_path]
	//> or
	//> vesper.exe [-analyze] [db_path]
	


	do 
	{
		//> parse command line
		//> BranchTracer.exe c:\windows\system32\notepad.exe
		if (argc != 2)
		{
			log_err
				L"usage: %s [debuggee path]", 
				argv[0]
			log_end
			ret = -1;
			break;
		}
	
		//> install control handler
		if(TRUE != SetConsoleCtrlHandler((PHANDLER_ROUTINE)control_handler, TRUE ))
		{
			log_err 
				L"SetConsoleCtrlHandler(), gle = %u", 
				GetLastError()
			log_end
			ret = -1;
			break;
		}

		//> crate debugger object & start debugger
		if (true != _debugger.initialize())
		{
			log_err
				L"_debugger.initialize()"
			log_end
			ret = -1;
			break;
		}

		if (true != _debugger.start_and_wait(argv[1], NULL))
		{
			log_err 
				L"_debugger.start( %s )", 
				argv[1]
			log_end
			ret = -1;
			break;
		}
	
		_debugger.finalize();
		log_info L"debugger stopped successfully..." log_end
		
		ret = 0;
	} while (false);

	

	//> finalize slogger
	slog_finalize();
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
BOOL control_handler(_In_ DWORD control_type) 
{ 
	switch(control_type) 
	{ 
	case CTRL_C_EVENT: 
		// Handle the CTRL-C signal. 
		log_dbg L"Ctrl-C signal received" log_end
		_debugger.stop();	
		return( TRUE );
		
	case CTRL_CLOSE_EVENT: 
		// CTRL-CLOSE: confirm that the user wants to exit. 
		log_dbg L"Ctrl-Close signal received" log_end
		_debugger.stop();
		return( TRUE ); 

	case CTRL_BREAK_EVENT: 
		// Pass other signals to the next handler. 
		log_dbg L"Ctrl-Break signal received" log_end
		_debugger.stop();
		return FALSE; 

	case CTRL_LOGOFF_EVENT: 
		log_dbg L"Ctrl-Logoff signal received" log_end
		_debugger.stop();
		return FALSE; 

	case CTRL_SHUTDOWN_EVENT: 
		log_dbg L"Ctrl-Shutdown signal received" log_end
		_debugger.stop();
		return FALSE; 

	default: 
		return FALSE; 
	} 
}

