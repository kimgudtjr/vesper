/**----------------------------------------------------------------------------
 * _MyLib_test.cpp
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 2014:1:25 13:34 created
**---------------------------------------------------------------------------*/
#include "stdafx.h"

#include "..\branch_log.h"

static DWORD		_pass_count = 0;
static DWORD		_fail_count = 0;


bool test_BranchLogger();
bool test_BranchLogger_log_exception_info(_In_ BranchLogger& bl);
bool test_BranchLogger_log_module_load(_In_ BranchLogger& bl);

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
	bool ret = false;

	_ASSERTE( true == slog_initialize(slog_debug, log_to_con, NULL) );
	slog_set_log_format(true, false, true);
	
	test_BranchLogger();

	log_info
		L"-------------------------------------------------------------------------------"
	log_end
	log_info
		L"total test = %u, pass = %u, fail = %u", 
		_pass_count + _fail_count, 
		_pass_count, 
		_fail_count
	log_end

	getchar();

	return 0;
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
bool test_BranchLogger()
{
	BranchLogger _bl;
	if (true != _bl.intialize()) return false;

	bool ret = false;
	assert_bool_with_param(true, test_BranchLogger_log_exception_info, _bl);
	assert_bool_with_param(true, test_BranchLogger_log_module_load, _bl);	
	
	_bl.finalize();
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
bool test_BranchLogger_log_exception_info(_In_ BranchLogger& bl)
{
	DWORD tid = 0xffffff10;
	EXCEPTION_DEBUG_INFO edi;
	edi.dwFirstChance = 1;
	edi.ExceptionRecord.ExceptionCode = 0xc0000005;
	edi.ExceptionRecord.ExceptionFlags = 0xffff;
	edi.ExceptionRecord.ExceptionRecord = NULL;
	edi.ExceptionRecord.ExceptionAddress = (void*)0xffffffff10101010;
	edi.ExceptionRecord.NumberParameters = 4;
	edi.ExceptionRecord.ExceptionInformation[0] = 0xffff1010;
	edi.ExceptionRecord.ExceptionInformation[1] = 0xffff1011;
	edi.ExceptionRecord.ExceptionInformation[2] = 0xffff1012;
	edi.ExceptionRecord.ExceptionInformation[3] = 0xffff1013;
	UINT8 code_buffer[CODE_BUFFER_SIZE] = {0x00, 0x00, 0x90, 0x00, 0xcc, 0x00, 0x55, 0x8b};

	if (true != bl.log_exception_info(tid, &edi, CODE_BUFFER_SIZE, code_buffer)) return false;


	//> 검증
	INT64 id = bl.get_last_row_id();
	char sql_buf[1024];
	StringCbPrintfA(
		sql_buf, 
		sizeof(sql_buf), 
		"select "\
		"    tid, "\
		"    first_chance, "\
		"    exception_code, "\
		"    exception_flag, "\
		"    exception_addr, "\
		"    number_parameters, "\
		"    exception_info_1, "\
		"    exception_info_2, "\
		"    exception_info_3, "\
		"    exception_info_4, "\
		"    buffer "\
		"from lbr where idx = %I64d",
		id);
	CppSQLite3Query query = bl._db.execQuery(sql_buf);
	
	DWORD r_dword = (DWORD)query.getIntField(0);
	if (r_dword != tid) return false;

	std::string str = query.getStringField(1);
	if (0 != str.compare("y")) return false;

	r_dword = query.getIntField(2);
	if (edi.ExceptionRecord.ExceptionCode != r_dword) return false;

	UINT64 r_int64 = 0;
	str = query.getStringField(4);
	if (true != str_to_uint64(str.c_str(), r_int64)) return false;	
	if ((UINT64)edi.ExceptionRecord.ExceptionAddress != r_int64) return false;







	if (false != bl.log_exception_info(tid, &edi, CODE_BUFFER_SIZE, NULL)) return false;
	if (false != bl.log_exception_info(tid, &edi, 200, NULL)) return false;
	if (false != bl.log_exception_info(tid, &edi, CODE_BUFFER_SIZE, NULL)) return false;

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
bool test_BranchLogger_log_module_load(_In_ BranchLogger& bl)
{
	std::wstring module_path(L"c:\\windows\\system32\\kernel32.dll");
	if (true != bl.log_module_load(module_path, 0xffffffff7c809090)) return false;

	//> 검증
	INT64 id = bl.get_last_row_id();
	char sql_buf[1024];
	StringCbPrintfA(
		sql_buf, 
		sizeof(sql_buf), 
		"select module_path, base_addr from module where idx = %u", 
		id);
	CppSQLite3Query query = bl._db.execQuery(sql_buf);

	UINT64 r_addr = 0;
	std::string r_path = query.getStringField(0);
	const char* r_addr_str = query.getStringField(1, "0");
	if (0 != module_path.compare( MbsToWcsEx(r_path.c_str()) )) false;
	if (true != str_to_uint64(r_addr_str, r_addr)) return false;
	if (0xffffffff7c809090 != r_addr) return false;
	
	module_path = L"";
	if (true == bl.log_module_load(module_path, 0xffffffff7c809090)) return false;

	return true;
}