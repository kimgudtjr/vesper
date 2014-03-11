/******************************************************************************
 * fc_drv_util.cpp
 ******************************************************************************
 * 
 ******************************************************************************
 * All rights reserved by somma (fixbrain@gmail.com)
 ******************************************************************************
 * 2011/01/11   20:57 created
******************************************************************************/
#include "DriverHeaders.h"
#include "fc_drv_util.h"
#include "DriverDebug.h"

static ULONG _process_name_offset = 0;

/**
* @brief	
* @param	
* @see		
* @remarks	
* @code		
* @endcode	
* @return	
*/
NTSTATUS 
get_proc_image_by_eprocess(
	__in PEPROCESS eprocess, 
	__inout const pwdg_image_name image_name
	)
{
	ASSERT(NULL != eprocess);
	ASSERT(NULL != image_name);	
	if ((NULL != eprocess) && (NULL != image_name))
	{
		HANDLE process_handle=NULL;
		NTSTATUS st = ObOpenObjectByPointer(
								eprocess, 
								OBJ_KERNEL_HANDLE, 
								NULL,
								0,
								*PsProcessType, 
								KernelMode, 
								&process_handle);
		if (!NT_SUCCESS(st))
		{
			log_err "ObOpenObjectByPointer( eprocess=0x%p) failed", eprocess log_end
			return st;
		}

		return ( get_proc_image_by_handle(process_handle, image_name) );
	}

	return STATUS_INVALID_PARAMETER;
}

/**
* @brief	http://www.osronline.com/article.cfm?article=472
			xp and xp higher only
* @param	
* @see		
* @remarks	
* @code		
* @endcode	
* @return	
*/
NTSTATUS 
get_proc_image_by_handle(
	__in HANDLE process_handle, 
	__inout const pwdg_image_name image_name
	)
{
    NTSTATUS status;
    ULONG returnedLength;
    static fn_zw_query_information_process _ZwQueryInformationProcess= NULL;

    PAGED_CODE(); // this eliminates the possibility of the IDLE Thread/Process

    if (NULL == _ZwQueryInformationProcess) 
	{
        UNICODE_STRING routineName;
        RtlInitUnicodeString(&routineName, L"ZwQueryInformationProcess");
        _ZwQueryInformationProcess = 
			(fn_zw_query_information_process) MmGetSystemRoutineAddress(&routineName);
        if (NULL == _ZwQueryInformationProcess) 
		{
            log_err "can not resolve ZwQueryInformationProcess" log_end
			return STATUS_UNSUCCESSFUL;
        }
    }

	//
    // Step one - get the size we need
    //
    status = _ZwQueryInformationProcess( 
					process_handle,				//NtCurrentProcess(), 
                    ProcessImageFileName,
                    NULL,						// buffer
                    0,							// buffer size
                    &returnedLength);
    if (STATUS_INFO_LENGTH_MISMATCH != status) 
	{
        return status;
    }

	//
    // Is the passed-in buffer going to be big enough for us?  
    // This function returns a single contiguous buffer model...
    //
	if (returnedLength - sizeof(UNICODE_STRING) > _max_image_name_length)
	{
		log_err "_max_image_name_length (%u bytes) is too small", _max_image_name_length log_end
		return STATUS_BUFFER_OVERFLOW;
	}

    //
    // Now lets go get the data
    //
	//	ZwQueryInformationProcess() 의 세번째 파라미터 ProcessInformation 은 
	//	UNICODE_STRING 구조체 포인터 이며 single contguous buffer model 을 이용한다. 
	//	함수가 성공하면 return 되는 UNICODE_STRING ProcessInformation 의 메모리 구조는 
	//		[Length (2byte)] [MaximumLength (2byte)] [Buffer (4byte)] [ string.....]
	//  형태이며 Buffer 는 [string..] 의 주소를 가리키고 있게 된다. 
	//	(모두 연속된 메모리이다!!)
	//
	//	[64 00] [66 00] [94 89 d6 f8]   [\.D.e.v.i.c.e.\.H.a.r.]
	//                  -------------   ------------------------ 
	//                        |_________↑
	//	-----------------------------   ------------------------
	//			UNICODE_STRING                  BUFFER [MAX_PATH] 
	//	=> SP_IMAGE_NAME 구조체는 이 메모리 구조와 동일하다. 
	//
	//	returnedLenth 는 UNICODE_STRING::Length/MaximumLenth 를 모두 포함한 길이이다. 
	//
	status = _ZwQueryInformationProcess( 
						process_handle,//NtCurrentProcess(), 
                        ProcessImageFileName,
                        image_name,
                        returnedLength,
                        &returnedLength);
	if (! NT_SUCCESS(status))
	{
		log_err "ZwQueryInformationProcess failed. status=0x%08x", status log_end
		return status;
	}

	// null-terminate
	image_name->buffer[ returnedLength - sizeof(UNICODE_STRING) ]= 0x00;	
		
    //
    // And tell the caller what happened.
    //    
    return status;
}


/**
* @brief	EPROCESS structure에서 ImageFileName FIELD의 위치를 구한다
            반드시 SYSTEM process의 context에서 호출되어야 함
            => DriverEntry 안에서 호출되면 됨.
* @param	
* @see		
* @remarks	
* @code		
* @endcode	
* @return	
*/
BOOLEAN
set_process_name_offset(
	)
{
	#define		SYSNAME         "System"
    PEPROCESS curproc = PsGetCurrentProcess();

    // 현재 process가 SYSTEM process란 가정하에 SYSTEM이란 문자열을 찾는다.
    //
    for( ULONG i = 0; i < 3*PAGE_SIZE; i++ ) 
    {
        if( !strncmp( SYSNAME, (PCHAR) curproc + i, strlen(SYSNAME) )) 
        {
            _process_name_offset = i;
            return TRUE;
        }
    }

    // Name not found - oh, well
    //
    log_err "can not find ImageFileName field offset. give up!" log_end
    return FALSE;
}


/**
* @brief	
* @param	
* @see		
* @remarks	
* @code		
		CHAR name[_nt_procname_length] = {0};
		_get_process_name(pEproc, name);

* @endcode	
* @return	
*/
PCHAR get_process_name(__in const PEPROCESS eprocess, __in char* const name)
{
    PEPROCESS       curproc = NULL;
    char            *nameptr = NULL;

    //
    // We only try and get the name if we located the name offset
    //
    if( 0 != _process_name_offset ) 
	{
        //
        // Get a pointer to the current process block
        //
        curproc = eprocess;

        //
        // Dig into it to extract the name. Make sure to leave enough room
        // in the buffer for the appended process ID.
        //
        nameptr   = (PCHAR) curproc + _process_name_offset;
        strncpy( name, nameptr, _nt_procname_length-1 );
        name[_nt_procname_length-1] = 0;
        ///!    sprintf( name + strlen(name), ":%d", (ULONG) PsGetCurrentProcessId());
    } 
	else 
	{
        strncpy( name, "???", _nt_procname_length-1);
    }

    return name;
}