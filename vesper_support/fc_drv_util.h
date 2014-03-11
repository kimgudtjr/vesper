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
#include "fc_drv_share.h"

typedef 
NTSTATUS 
(__stdcall *fn_zw_query_information_process) (
    __in HANDLE ProcessHandle,
	__in PROCESSINFOCLASS ProcessInformationClass,
	__out_bcount(ProcessInformationLength) PVOID ProcessInformation,
	__in ULONG ProcessInformationLength,
	__out_opt PULONG ReturnLength
    );


#define ERSRC_INIT(_resource_ptr)							\
		ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);		\
		ExInitializeResourceLite(_resource_ptr);

#define ERSRC_ENTER_EXCLUSIVE(_resource_ptr)				\
    __try{                                                  \
		ASSERT(KeGetCurrentIrql() <= APC_LEVEL);			\
		KeEnterCriticalRegion();							\
        ExAcquireResourceExclusiveLite(_resource_ptr, TRUE);

#define ERSRC_ENTER_SHARE(_resource_ptr)					\
    __try{                                                  \
        ASSERT(KeGetCurrentIrql() <= APC_LEVEL);			\
		KeEnterCriticalRegion();							\
		ExAcquireResourceSharedLite(_resource_ptr, TRUE);

#define ERSRC_LEAVE(_resource_ptr)							\
    }__finally{                                             \
        ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);		\
        ExReleaseResourceLite(_resource_ptr);               \
		KeLeaveCriticalRegion();							\
    }


// FAST_MUTEX
#define FAST_MUTEX_INIT(_fast_mutex_ptr)					\
	ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);			\
	ExInitializeFastMutex(_fast_mutex_ptr);

#define FAST_MUTEX_ENTER(_fast_mutex_ptr)					\
	__try{ ASSERT(KeGetCurrentIrql() <= APC_LEVEL);			\
	ExAcquireFastMutex(_fast_mutex_ptr); 

#define FAST_MUTEX_LEAVE(_fast_mutex_ptr)					\
	} __finally {											\
		ASSERT(KeGetCurrentIrql() == APC_LEVEL);			\
		ExReleaseFastMutex(_fast_mutex_ptr);				\
	}

NTSTATUS 
get_proc_image_by_eprocess(
	__in PEPROCESS eprocess, 
	__inout const pwdg_image_name image_name
	);

NTSTATUS 
get_proc_image_by_handle(
	__in HANDLE process_handle, 
	__inout const pwdg_image_name image_name
	);



BOOLEAN
set_process_name_offset(
	);

PCHAR 
get_process_name(
	__in const PEPROCESS eprocess, 
	__in char* const name	
	);
