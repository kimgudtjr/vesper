/*-----------------------------------------------------------------------------
 * DriverEntry.cpp
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by somma (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * - 10.11.2010 created
**---------------------------------------------------------------------------*/
#include "DriverHeaders.h"
#include "DriverDebug.h"
#include "fc_drv_util.h"
#include "fc_drv_ioctl.h"
#include "asm_function.h"
#include "arch.h"
#include "vesper_support.h"

// 
// global
// 
PDEVICE_EXTENSION		g_dev_ext = NULL;

// nt dispatch functions
NTSTATUS	__stdcall DispatchDummy(IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp);
NTSTATUS	__stdcall DispatchDeviceIoControl(IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp);
VOID		__stdcall DispatchUnload(IN PDRIVER_OBJECT DriverObject);
NTSTATUS	__stdcall DispatchCleanup(IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp);

NTSTATUS HMHandleIoControl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PULONG_PTR Info);

/**----------------------------------------------------------------------------
    \brief  DriverEntry

    \param  
    \return         
    \code
    \endcode        
-----------------------------------------------------------------------------*/
extern "C" 
NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	NTSTATUS            status;
	PDEVICE_OBJECT      deviceObject;
	UNICODE_STRING      ntName;
	UNICODE_STRING      win32Name;

	log_info
		"\n\n===============================================================================\n"\
		"driver Compiled at %s on %s \n"\
		"===============================================================================\n", 
		__TIME__, __DATE__
	log_end

	

	//> x86/x64 assembly function test
	MSR msr = {0};
	read_msr(MSR_IA32_DEBUGCTL, &msr);
	log_info "msr IA32_DEBUGCTL_MSR(IA32_DEBUGCTL_MSR), low = 0x%08x, high = 0x%08x", msr.low, msr.high  log_end	

/*
	msr.low |= 0x02;
	write_msr(IA32_DEBUGCTL_MSR, &msr);

	read_msr(IA32_DEBUGCTL_MSR, &msr);
	log_info "msr IA32_DEBUGCTL_MSR(IA32_DEBUGCTL_MSR), low = 0x%08x, high = 0x%08x", msr.low, msr.high  log_end	
*/	

    //> device 이름 생성
    RtlInitUnicodeString(&ntName, _nt_device_name);
    status = IoCreateDevice(
                    DriverObject, 
					sizeof(DEVICE_EXTENSION), 
					&ntName, 
					FILE_DEVICE_UNKNOWN, 
					FILE_DEVICE_SECURE_OPEN, 
					TRUE, 
					&deviceObject
                    );
    if (FALSE == NT_SUCCESS(status))
    {		    
        return status;
    }

	//> add your own initialization routine
	g_dev_ext = (PDEVICE_EXTENSION) deviceObject->DeviceExtension;
	RtlZeroMemory(g_dev_ext, sizeof(DEVICE_EXTENSION));
	
    //> symbolic link 생성
    RtlInitUnicodeString(&win32Name, _dos_device_name);	
    status = IoCreateSymbolicLink ( &win32Name, &ntName);
	if (FALSE == NT_SUCCESS(status))
    {
        IoDeleteDevice( DriverObject ->DeviceObject );
		return status;
    }

    // initialize function pointers
    for (int i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) 
    {
	    DriverObject->MajorFunction[i] = DispatchDummy;
    }
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchDeviceIoControl;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] = DispatchCleanup;   
    DriverObject->DriverUnload = DispatchUnload;
	
	g_dev_ext->initialized = true;
	return STATUS_SUCCESS;
}

/**
* @brief	
* @param	
* @see		
* @remarks	
* @code		
* @endcode	
* @return	
*/
NTSTATUS __stdcall DispatchDummy(
	IN  PDEVICE_OBJECT  DeviceObject,
	IN  PIRP            Irp
	)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	Irp ->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;	
	IoCompleteRequest( Irp, IO_NO_INCREMENT );

	return STATUS_SUCCESS;
}

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
__stdcall DispatchDeviceIoControl(
	IN  PDEVICE_OBJECT  DeviceObject,
	IN  PIRP            Irp
	)
{
	ULONG_PTR bytes_returned = 0;
	NTSTATUS status = HMHandleIoControl(DeviceObject, Irp, &bytes_returned);
	if (TRUE != NT_SUCCESS(status))
	{
		log_err
			"HMHandleIoControl(DeviceObject=0x%08p, Irp=0x%08p) failed, status=0x%08x", 
			DeviceObject, Irp, status
		log_end
	}
	
	Irp ->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = bytes_returned;
	IoCompleteRequest( Irp, IO_NO_INCREMENT );
	return status;
}

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
__stdcall 
DispatchCleanup(
	IN  PDEVICE_OBJECT  DeviceObject,
	IN  PIRP            Irp
	)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	// do something
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
	UNREFERENCED_PARAMETER(pdx);
	
	Irp ->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;	
	IoCompleteRequest( Irp, IO_NO_INCREMENT );

	log_info "driver cleanup called successfully" log_end
	return STATUS_SUCCESS;
}


/**
* @brief	
* @param	
* @see		
* @remarks	
* @code		
* @endcode	
* @return	
*/
VOID 
__stdcall 
DispatchUnload(
	IN  PDRIVER_OBJECT  DriverObject
	)
{
	UNICODE_STRING Win32NameString;
	PDEVICE_OBJECT DevObj = DriverObject->DeviceObject;

	// 
	// ADD DRIVER UNLOAD CODE
	// 
	disable_btf();
	
	// symbolic link 제거 및 디바이스 객체 삭제
	//
	RtlInitUnicodeString (&Win32NameString , _dos_device_name);	
	IoDeleteSymbolicLink (&Win32NameString);	
	IoDeleteDevice( DevObj );
	log_info "driver unloaded successfully" log_end
}

/**----------------------------------------------------------------------------
	\brief	DeviceIoControl 처리함수

	\param	
	\return
	\code
	
	\endcode		
-----------------------------------------------------------------------------*/
NTSTATUS 
HMHandleIoControl(
	IN PDEVICE_OBJECT DeviceObject, 
	IN PIRP Irp, 
	IN PULONG_PTR Info
	)
{
	ASSERT(NULL != DeviceObject);
	ASSERT(NULL != Irp);
	ASSERT(NULL != Info);
	if (NULL == DeviceObject || NULL == Irp || NULL == Info) return STATUS_INVALID_PARAMETER;

	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
	PIO_STACK_LOCATION StackLocation = (PIO_STACK_LOCATION) IoGetCurrentIrpStackLocation(Irp);
	//ULONG cbIn = StackLocation->Parameters.DeviceIoControl.InputBufferLength;
	//ULONG cbOut = StackLocation->Parameters.DeviceIoControl.OutputBufferLength;
	ULONG IoCode = StackLocation->Parameters.DeviceIoControl.IoControlCode;	
	*Info = 0;		// initialize output data length	
	
	UNREFERENCED_PARAMETER(pdx);

	switch (IoCode)
	{	
	case _io_test: 
		{

			enable_btf();

			status = STATUS_SUCCESS;
			break;
		}
	default:
		{
			log_err "invalid iocode=0x%08x", IoCode log_end
			status = STATUS_INVALID_PARAMETER;			
		}
	}// switch

	return status;
}

