/**----------------------------------------------------------------------------
 * stdafx.h
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 2013:12:22 21:05 created
**---------------------------------------------------------------------------*/
#pragma once

#include "targetver.h"

#include <boost/noncopyable.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include "boost/shared_ptr.hpp"
#include <sstream>
#include <list>

#include <stdio.h>
#include <tchar.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <crtdbg.h>

#include "slogger.h"
#include "Win32Utils.h"

#define CODE_BUFFER_SIZE	16

//> for distrorm3
#if defined(_AMD64_)
	//> configure for distorm3
	#define SUPPORT_64BIT_OFFSET

#elif defined(_X86_)
	//> 

#else
	#error !!unsupported architecture!!
#endif			


//> for dbghelp 
#define DBGHELP_TRANSLATE_TCHAR
#include "dbghelp.h"
