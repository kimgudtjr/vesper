/**----------------------------------------------------------------------------
 * analyzer.cpp
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com)
 *-----------------------------------------------------------------------------
 * 2014:3:21 11:12 created
**---------------------------------------------------------------------------*/
#include "StdAfx.h"
#include "analyzer.h"
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
analyzer::analyzer(void) : _initialized(false)
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
analyzer::~analyzer(void)
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
bool analyzer::initialize(_In_ const wchar_t* db_path)
{
	_ASSERTE(NULL != db_path);
	if (NULL == db_path) return false;

	if (true != is_file_existsW(db_path))
	{
		log_err L"no db file, path = %s", db_path log_end
		return false;
	}

	bool ret = false;
	_db_path = db_path;

	try
	{
		//> open database 
		_db.open(WcsToMbsEx(_db_path.c_str()).c_str());
		
		//> check database 
		if (true != is_database_valid())
		{
			log_err L"is_database_valid()" log_end
			return false;
		}

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
void analyzer::finalize()
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
bool analyzer::load_module_symbols()
{
	_ASSERTE(true == _initialized);
	if (true != _initialized) return false;

	bool ret = false;
	try
	{
		CppSQLite3Query query = _db.execQuery(SELECT_MODULE);

		while (true != query.eof())
		{
			std::string module_path = query.getStringField(0, "");
			std::string base_addr_str = query.getStringField(1, "");

			uint64_t base_addr = 0;
			if (true != str_to_uint64(base_addr_str.c_str(), base_addr)) 
			{
				log_err 
					L"str_to_uint64( str = %s )", 
					base_addr_str.c_str() 
				log_end
				goto _next;
			}

			log_info
				L"0x%u64 %S", base_addr, module_path.c_str()
			log_end
_next:
			query.nextRow();
		}

		ret = true;
	}
	catch (CppSQLite3Exception& e)
	{
		log_err 
			L"sqlite3 exception. code = %s, msg = %s", 
			e.errorCode(),
			MbsToWcsEx(e.errorMessage()).c_str()
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
bool analyzer::is_database_valid()
{
	if (true != _db.tableExists(LBR_TABLE))
	{
		log_err L"no table, table name = %s", LBR_TABLE log_end
		return false;
	}

	if (true != _db.tableExists(MODULE_TABLE))
	{
		log_err L"no table, table name = %s", MODULE_TABLE log_end
		return false;
	}

	return true;
}