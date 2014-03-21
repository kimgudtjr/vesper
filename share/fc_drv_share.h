/**************************************************************************************
*
*	Copyright (C) 2007 AhnLab, Inc. All rights reserved.
*
*	This program is strictly confidential and not be used in outside the office...and 
*	some other copyright agreement etc.
*
*	File:
*		d:\Work.PoC\IQ_mung\trunk\src\share\fc_drv_share.h
*
*	Author:
*		Yonghwan.Noh ( somma@ahnlab.com )
*
*	DESCRIPTION :
*
*
*	HISTORY :
*  	2012-3-12 by Yonghwan.Noh
* 		1. file created
**************************************************************************************/
#ifndef		_fc_drv_share_
#define		_fc_drv_share_

#ifndef _NTIFS_INCLUDED_ 

#include <NTSecAPI.h>				// UNICODE_STRING

#endif//_NTIFS_INCLUDED_



#define		_driver_name			"vesperk"
#define		_driver_name_u			L"vesperk"

#define		_nt_device_name			L"\\Device\\"_driver_name_u
#define		_dos_device_name		L"\\DosDevices\\"_driver_name_u
#define		_service_name			_driver_name_u
#define		_service_name_display	L"vesper support service"


#define		_nt_procname_length		16
#define		_max_image_name_length	1024


// 파일 경로를 표현하기 위한 구조체 (ZwQuerySystemInformationProcess() 버퍼 구조 호환)
//
//	[64 00] [66 00] [94 89 d6 f8]   [\.D.e.v.i.c.e.\.H.a.r.]
//                  -------------   ------------------------ 
//                        |_________↑
//	-----------------------------   ------------------------
//			UNICODE_STRING                  BUFFER [MAX_PATH] 
typedef struct _wdg_image_name
{
	UNICODE_STRING	string;
	wchar_t			buffer[_max_image_name_length];
} wdg_image_name, *pwdg_image_name;

#endif//_fc_drv_share_