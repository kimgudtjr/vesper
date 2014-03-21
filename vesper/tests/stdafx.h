/**----------------------------------------------------------------------------
 * stdafx.h
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 2014:3:2 20:45 created
**---------------------------------------------------------------------------*/
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

//> reported as vs 2010 bug, ms says that will be patch this bug next major vs release, vs2012.
//
//1>C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\include\intsafe.h(171): warning C4005: 'INT16_MAX' : macro redefinition
//1>          C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\include\stdint.h(77) : see previous definition of 'INT16_MAX'
#pragma warning(disable:4005)
#include <intsafe.h>
#pragma warning(default:4005)

#include "slogger.h"
#include "Win32Utils.h"
#include "mini_test.h"

#define CODE_BUFFER_SIZE	16

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


#define _test_define_