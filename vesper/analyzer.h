/**----------------------------------------------------------------------------
 * analyzer.h
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com)
 *-----------------------------------------------------------------------------
 * 2014:3:21 11:12 created
**---------------------------------------------------------------------------*/
#pragma once

#include "CppSQLite3.h"
#include "SymbolEngine.h"


/**
 * @brief	ms symbol wrapper
**/
class CSymbolEngineEx : public CSymbolEngine
{
public:
	virtual void OnEngineNotify( const std::wstring& Message ) 
	{
		log_info L"%s%s", m_Prefix.c_str(), Message.c_str() log_end
	}

	void SetPrefix( const std::wstring& Prefix )
	{ 
		m_Prefix = Prefix; 
	}

protected:
	std::wstring m_Prefix;
};

/**
 * @brief	local db 에서 branch, module 정보를 읽어 symbol 정보와 매칭
**/
class analyzer
{
public:
	analyzer(void);
	virtual ~analyzer(void);

	bool	initialize(_In_ const wchar_t* db_path);
	void	finalize();

	bool	load_module_symbols();

private:
	bool	is_database_valid();

private:
	bool			_initialized;
	std::wstring	_db_path;
	CppSQLite3DB	_db;
	CSymbolEngineEx	_symbol;

};

