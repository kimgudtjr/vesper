/**----------------------------------------------------------------------------
 * vesper_support.h
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 2014:2:14 10:34 created
**---------------------------------------------------------------------------*/
#pragma once

#include "scm_context.h"


#include <WinIoCtl.h>
#define	_fc_drv_device_type	0x0000AA72
#define _fc_iocode(_code, _BufferType, _accessRight) ( (DWORD) CTL_CODE(_fc_drv_device_type, _code, _BufferType, _accessRight) )
#define _io_test			_fc_iocode(0x0801, METHOD_BUFFERED, FILE_ANY_ACCESS)


#define VESPER_SYS_NAME		L"vesper.sys"
#define VESPER_SVC_NAME		L"vesper"
#define VESPER_SVC_DISPLAY	L"vesper ring0 supporter"

class vesper_support
{
public:
	vesper_support()
		:
	_initialzied(false), 
	_scm(NULL)
	{
	
	}

	~vesper_support()
	{
		finalize();
	}

	bool initialize()
	{
		_ASSERTE(true != _initialzied);
		if (true == _initialzied) return true;

		std::wstring current_dir;
		if (true != get_current_module_dir(current_dir))
		{
			log_err L"get_current_module_dir()" log_end
			return false;
		}
		std::wstring vesper_sys = current_dir + L"\\" + VESPER_SYS_NAME;
		
		if (true != is_file_existsW(vesper_sys.c_str()))
		{
			log_err L"no file, vesper sys = %s", vesper_sys.c_str() log_end
			return false;
		}
		
		_scm = new scm_context(vesper_sys.c_str(), VESPER_SVC_NAME, VESPER_SVC_DISPLAY, true);
		if (true != _scm->install_driver())
		{
			log_err L"_scm->install_driver()" log_end		
			return false;
		}

		if (true != _scm->start_driver())
		{
			log_err L"_scm->start_driver()" log_end
			return false;
		}
		log_info L"vesper service started successfully..." log_end

		_initialzied = true;
		return true;
	}

	void finalize()
	{
		if (true != _initialzied) return;

		_ASSERTE(NULL != _scm);
		if (NULL != _scm) 
		{
			_scm->stop_driver();
			_scm->uninstall_driver();
			delete _scm; _scm = NULL;
		}

		_initialzied = false;
	}

	bool enable_btf()
	{
		_ASSERTE(true == _initialzied);
		if (true != _initialzied) return false;

		UINT32 bytes_return = 0;
		if (true != _scm->send_command(_io_test, 0, NULL, 0, NULL, &bytes_return))
		{
			log_err L"send_command(_io_test)" log_end
			return false;
		}

		return true;
	}

private:
	bool			_initialzied;
	scm_context*	_scm;


};

