/**----------------------------------------------------------------------------
 * vesper_support.h
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 2014:2:12 22:31 created
**---------------------------------------------------------------------------*/
#ifndef _vesper_support_
#define _vesper_support_

#ifdef __cplusplus
extern "C" {
#endif 

#include <ntifs.h>
#include <ntintsafe.h>

#ifdef __cplusplus
}
#endif 

#include "arch.h"

NTSTATUS enable_btf();
NTSTATUS disable_btf();

#endif//_vesper_support_