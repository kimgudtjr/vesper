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



/**
 * @brief	커맨드라인 옵션
**/
typedef struct _OPTION
{
	bool			run_debugger;

	union
	{
		struct 
		{
			#pragma todo("add parameters for debuggee")
			const wchar_t*	debuggee;
			const wchar_t*	db_path;
		} param_debug;

		struct 
		{
			const wchar_t*	db_path;
			
		} param_analyze;			
	} param;		
} OPTION, *POPTION;


static Debugger _debugger;
static analyzer	_analyzer;

BOOL control_handler(_In_ DWORD control_type);
void usage(_In_ const wchar_t* argv0);
bool parse_commnad_line(_In_ int argc, _In_ wchar_t* argv[], _Out_ OPTION& option);
bool run_debugger(_In_ const POPTION option);
bool run_analyzer(_In_ const POPTION option);

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
int _tmain(int argc, wchar_t* argv[])
{
	int ret = -1;

	//> initialize file log
	std::wstring log_file;
	get_current_module_path(log_file);
	log_file += L".log";
		
	if (true != slog_initialize(
						slog_debug, 
						log_to_file | log_to_con, 
						log_file.c_str()))
	{
		log_err L"slog_initialize( file = %s )", log_file.c_str() log_end
		return -1;
	}
	slog_set_log_format(false, true, true);

	//> install control handler
	if(TRUE != SetConsoleCtrlHandler((PHANDLER_ROUTINE)control_handler, TRUE ))
	{
		log_err 
			L"SetConsoleCtrlHandler(), gle = %u", 
			GetLastError()
		log_end
		return -1;		
	}

	//> analyze command line
	OPTION run_option = {0};
	if (true != parse_commnad_line(argc, argv, run_option))
	{
		log_err L"invalid parameter" log_end
		usage(argv[0]);

		return -1;
	}
	
	//> run 
	if (true == run_option.run_debugger)
	{
		if (true != run_debugger(&run_option))
		{
			log_err 
				L"run_debugger( debuggee = %s, db_path = %s )", 
				run_option.param.param_debug.debuggee, 
				run_option.param.param_debug.db_path
			log_end
			return -1;
		}
	}
	else
	{
		if (true != run_analyzer(&run_option))
		{
			log_err 
				L"run_analyzer( db_path = %s )",
				run_option.param.param_analyze.db_path
			log_end
			return -1;	
		}
	}
	
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

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
void usage(_In_ const wchar_t* argv0)
{
	log_err
		L"usage : "\
		L"	%s [-debug] [file_path] [db_path]"\
		L"				or"\
		L"	%s [-analyze] [db_path]", 
		argv0, argv0
	log_end
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
parse_commnad_line(
	_In_ int argc, 
	_In_ wchar_t* argv[], 
	_Out_ OPTION& run_option
	)
{
	_ASSERTE(0 != argc);
	_ASSERTE(NULL != argv);
	if (0 == argc || NULL == argv) return false;

	if (argc < 3) return false;

	std::wstring tmp = argv[1];
	if (0 == tmp.compare(L"-debug"))
	{
		//> vesper.exe -debug c:\debuggee.exe c:\debuggee.db3
		run_option.run_debugger = true;

		_ASSERT(NULL != argv[2]);
		_ASSERT(NULL != argv[3]);
		if (NULL == argv[2] || NULL == argv[3]) return false;
		
		run_option.param.param_debug.debuggee = argv[2];
		run_option.param.param_debug.db_path = argv[3];
	}
	else if (0 == tmp.compare(L"-analyze"))
	{
		//> vesper.exe -analyze c:\debuggee.db3
		run_option.run_debugger = false;

		_ASSERT(NULL != argv[2]);
		if (NULL == argv[2]) return false;

		run_option.param.param_analyze.db_path = argv[2];
	}
	else
	{
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
bool run_debugger(_In_ const POPTION option)
{
	_ASSERTE(NULL != option);
	if (NULL == option) return false;
	_ASSERTE(true == option->run_debugger);
	if (true != option->run_debugger) return false;

	bool ret = false;
	#pragma warning(disable: 4127)
	do 
	{
		//> crate debugger object & start debugger
		if (true != _debugger.initialize(option->param.param_debug.db_path))
		{
			log_err
				L"_debugger.initialize()"
			log_end			
			break;
		}

		if (true != _debugger.start_and_wait(option->param.param_debug.debuggee, NULL))
		{
			log_err 
				L"_debugger.start( %s )", 
				option->param.param_debug.debuggee
			log_end
			break;
		}
	
		log_info L"debugger stopped successfully..." log_end
		ret = true;
	} while (false);
	#pragma warning(default: 4127)

	_debugger.finalize();
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
bool run_analyzer(_In_ const POPTION option)
{
	_ASSERTE(NULL != option);
	if (NULL == option) return false;
	_ASSERTE(false == option->run_debugger);
	if (true == option->run_debugger) return false;
	
	bool ret = false;
	#pragma warning(disable: 4127)
	do 
	{
		if (true != _analyzer.initialize(option->param.param_analyze.db_path))
		{
			log_err
				L"_analyzer.initialize( db = %s )", 
				option->param.param_analyze.db_path
			log_end
			break;
		}

		if (true != _analyzer.start_analyzer())
		{
			log_err L"_analyzer.start_analyzer()" log_end
			break;
		}

		log_info L"branch information analyzed successfully." log_end
		ret = true;
	} while (false);
	#pragma warning(default: 4127)

	_analyzer.finalize();
	return ret;
}

