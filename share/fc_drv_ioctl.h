/**************************************************************************************
*
*	Copyright (C) 2007 AhnLab, Inc. All rights reserved.
*
*	This program is strictly confidential and not be used in outside the office...and 
*	some other copyright agreement etc.
*
*	File:
*		d:\Work.PoC\IQ_mung\trunk\src\share\fc_drv_ioctl.h
*
*	Author:
*		Yonghwan.Noh ( somma@ahnlab.com )
*
*	DESCRIPTION :
*
*
*	HISTORY :
*  	2012-10-12 by Yonghwan.Noh
* 		1. file created
**************************************************************************************/
#ifndef _NTIFS_INCLUDED_ 
#include <WinIoCtl.h>
#endif//_NTIFS_INCLUDED_


#define	_fc_drv_device_type	0x0000AA72
#define _fc_iocode(_code, _BufferType, _accessRight) ( (DWORD) CTL_CODE(_fc_drv_device_type, _code, _BufferType, _accessRight) )

// IOCTL 코드 정의
//
//	- app 와 driver 간에 데이터 교환이 없다면 METHOD_NEITHER, 데이터 교환이 있는 경우 METHOD_BUFFERED,
//		대용량 데이터 교환의 경우 METHOD_DIRECT 를 사용
//		

//	FUNCTION 코드는 12비트이며 이중 0x0800 (2048) ~ 0x0FFF (4095) 영역을 자유롭게 사용
//  
#define _io_test			_fc_iocode(0x0801, METHOD_BUFFERED, FILE_ANY_ACCESS)

