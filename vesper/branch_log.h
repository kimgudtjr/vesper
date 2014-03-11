/**----------------------------------------------------------------------------
 * branch_log.h
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 2014:2:25 22:13 created
**---------------------------------------------------------------------------*/
#pragma once

#include "CppSQLite3.h"

class BranchLogger
{
public:
	BranchLogger();
	~BranchLogger();

	bool	intialize();
	void	finalize();
	
	bool	log_exception_info(				
				_In_ DWORD tid, 
				_In_ const EXCEPTION_DEBUG_INFO* edi, 
				_In_ UINT32	code_size,
				_In_ const UINT8* code
				);
	bool	log_module_load(_In_ std::wstring& module_path, _In_ UINT_PTR base_addr);
	INT64	get_last_row_id();
private:
	bool			_initialized;
	std::wstring	_path;
	CppSQLite3DB	_db;
	

	bool	generate_db_path(_Out_ std::wstring& dbpath);	


#ifdef _test_define_
	friend bool test_BranchLogger_log_exception_info(_In_ BranchLogger& bl);
	friend bool test_BranchLogger_log_module_load(_In_ BranchLogger& bl);
#endif
};