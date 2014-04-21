/**----------------------------------------------------------------------------
 * branch_log.cpp
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 2014:2:25 21:40 created
**---------------------------------------------------------------------------*/
#include "stdafx.h"
#include "branch_log.h"
#include "branch_log_data.h"


/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
BranchLogger::BranchLogger() : _initialized(false)
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
BranchLogger::~BranchLogger()
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
bool BranchLogger::intialize(_In_ const wchar_t* db_path)
{
	_ASSERTE(NULL != db_path);
	if (NULL == db_path) return false;

	bool ret = false;
	
	_db_path = db_path;

	try
	{
		if (true == is_file_existsW(_db_path.c_str())) 
		{
			log_info 
				L"db = %s already exists. give up!", 
				_db_path.c_str()
			log_end
			
			return false;
		}

		_db.open(WcsToMbsEx(_db_path.c_str()).c_str());		
		_db.execDML(CREATE_LBR_TABLE);
		_db.execDML(CREATE_MODULE_TABLE);
		
		_initialized = true;
		ret = true;
	}
	catch (CppSQLite3Exception* e)
	{
		log_err 
			L"sqlite3 exception. code = %s, msg = %s", 
			e->errorCode(),
			MbsToWcsEx(e->errorMessage()).c_str()
		log_end
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
void BranchLogger::finalize()
{
	if (true != _initialized) return;

	try
	{
		_db.close();
	}
	catch (CppSQLite3Exception* e)
	{
		log_err 
			L"sqlite3 exception. code = %s, msg = %s", 
			e->errorCode(),
			MbsToWcsEx(e->errorMessage()).c_str()
		log_end
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
bool 
BranchLogger::log_exception_info(
	_In_ DWORD tid, 
	_In_ const EXCEPTION_DEBUG_INFO* edi, 
	_In_ UINT32	code_size,
	_In_ const UINT8* code
	)
{
	_ASSERTE(true == _initialized);
	//_ASSERTE(NULL != code);
	//_ASSERTE(CODE_BUFFER_SIZE == code_size);
	
	if (true != _initialized) return false;
	if (NULL == code) return false;	
	if (CODE_BUFFER_SIZE != code_size) return false;

	//> build hex string
	std::string hex_string;
	if (true != bin_to_hexa(code_size, code, false, hex_string))
	{
		log_err L"bin_to_hexa()" log_end
		return false;
	}
	
	try
	{
		char sql_buf[2048];
		HRESULT hr = StringCbPrintfA(
						sql_buf, 
						sizeof(sql_buf), 
						INSERT_LBR, 
						tid,
						0 == edi->dwFirstChance ? 'n' : 'y',						
						edi->ExceptionRecord.ExceptionCode,
						edi->ExceptionRecord.ExceptionFlags,
						edi->ExceptionRecord.ExceptionAddress,
						edi->ExceptionRecord.NumberParameters,
						edi->ExceptionRecord.ExceptionInformation[0],
						edi->ExceptionRecord.ExceptionInformation[1],
						edi->ExceptionRecord.ExceptionInformation[2],
						edi->ExceptionRecord.ExceptionInformation[3],
						hex_string.c_str()
						);
		if (TRUE != SUCCEEDED(hr))
		{
			log_err L"StringCbPrintfA(), hr = 0x%08x", hr log_end
			return false;
		}

		_db.execDML(sql_buf);
	}
	catch (CppSQLite3Exception* e)
	{
		log_err 
			L"sqlite3 exception. code = %s, msg = %s", 
			e->errorCode(),
			MbsToWcsEx(e->errorMessage()).c_str()
		log_end
	}

	return true;	// always return true
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
BranchLogger::log_module_load(
	_In_ const wchar_t* module_path, 
	_In_ UINT_PTR base_addr, 
	_In_ DWORD module_size
	)
{
	_ASSERTE(true == _initialized);
	if (true != _initialized) return false;

	if (NULL == module_path) return false;
	if (0 == base_addr) return false;

	try
	{
		std::string module_patha = WcsToMbsEx(module_path);

		char sql_buf[2048];
		HRESULT hr = StringCbPrintfA(
						sql_buf, 
						sizeof(sql_buf), 
						INSERT_MODULE, 
						module_patha.c_str(),
						base_addr, 
						module_size
						);
		if (TRUE != SUCCEEDED(hr))
		{
			log_err L"StringCbPrintfA(), hr = 0x%08x", hr log_end
			return false;
		}

		_db.execDML(sql_buf);
	}
	catch (CppSQLite3Exception* e)
	{
		log_err 
			L"sqlite3 exception. code = %s, msg = %s", 
			e->errorCode(),
			MbsToWcsEx(e->errorMessage()).c_str()
		log_end
	}

	return true;	// always return true
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
INT64 BranchLogger::get_last_row_id()
{
	_ASSERTE(true == _initialized);
	if (true != _initialized) return false;

	return _db.lastRowId();
}


