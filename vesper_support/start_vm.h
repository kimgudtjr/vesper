/**----------------------------------------------------------------------------
 * start_vm.h
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 18:10:2013   20:31 created
**---------------------------------------------------------------------------*/
#pragma once

#include <ntintsafe.h>

#define VMXON_RGN_SIZE		4096
#define VMCS_RGN_SIZE		4096
#define VMX_RGN_BLOCKSIZE	4096

#define FAKE_STACK_SIZE		0x2000
#define FAKE_STACK_TAG		'ekaf'




//> somma

//warning C4201: nonstandard extension used : nameless struct/union
//warning C4214: nonstandard extension used : bit field types other than int
#pragma warning(disable: 4201)
#pragma warning(disable: 4214)

//> VM-execution control bits
#define PROC_BASED_HLT_EXITING			7
#define PROC_BASED_USE_IOBITMAP			25

//> exit reason
#define EXIT_REASON_IO_INSTRUCTION      30

#define IO_BITMAP_A			0x00002000
#define IO_BITMAP_A_HIGH	0x00002001
#define IO_BITMAP_B			0x00002002
#define IO_BITMAP_B_HIGH	0x00002003

//> keyboard stuff 8042
/* 
   8042 Status Register (port 64h read)

	|7|6|5|4|3|2|1|0|  8042 Status Register
	 | | | | | | | `---- output register (60h) has data for system
	 | | | | | | `----- input register (60h/64h) has data for 8042
	 | | | | | `------ system flag (set to 0 after power on reset)
	 | | | | `------- data in input register is command (1) or data (0)
	 | | | `-------- 1=keyboard enabled, 0=keyboard disabled (via switch)
	 | | `--------- 1=transmit timeout (data transmit not complete)
	 | `---------- 1=receive timeout (data transmit not complete)
	 `----------- 1=even parity rec'd, 0=odd parity rec'd (should be odd)
 */

//> Status register bits
#define KEYB_STATUS_OBUFFER_FULL        (1 << 0)
#define KEYB_STATUS_IBUFFER_FULL        (1 << 1)
#define KEYB_STATUS_TRANSMIT_TIMEOUT    (1 << 5)
#define KEYB_STATUS_PARITY_ERROR        (1 << 7)

//> In READ mode. Can be read at any time
#define KEYB_REGISTER_STATUS  0x64

//> In READ mode. Should be read if bit 0 of status register is 1
#define KEYB_REGISTER_OUTPUT  0x60

//> In WRITE mode. Data should only be written if Bit 1 of the status register
//	is zero (register is empty)
#define KEYB_REGISTER_DATA    0x60


// EXIT REASON == EXIT_REASON_IO_INSTRUCTION 일때 
// Exit Qualification 값
typedef struct _EXITQL_IO_INSTRUCTION
{
	union
	{
		UINT32		value;
		struct 
		{
			UINT32	size_of_access	: 3;		// 0,	0 = 1byte, 1 = 2bytes, 3 = 4bytes. other values are not used.
			UINT32	direction		: 1;		// 3,	0 = out, 1 = in
			UINT32	instr_string	: 1;		// 4,	0 = not string, 1 = string
			UINT32	rep_prefixed	: 1;		// 5,	0 = not rep, 1 = rep
			UINT32	operand_encoding: 1;		// 6,	0 = DX, 1 = immediate
			UINT32	reserved_7_15	: 8;		// 7,	reserved, cleared to 0
			UINT32	port_number		: 16;		// 15,	port number	 (as specified in DX or in an immediate operand)
			//UINT64	reserved_32_63	: 32;		// 16,	reserved, cleared to 0. (these bits exist only on processors that support intel 64 architecture)
		};
	};
} EXITQL_IO_INSTRUCTION, *PEXITQL_IO_INSTRUCTION;

void set_bit32(IN OUT ULONG* bit32_value, IN ULONG single_bit);









typedef struct _VCPU
{
	BOOLEAN					allocated;

	UCHAR*					vmxon_region;
	UINT32					vmxon_region_size;
	PHYSICAL_ADDRESS		vmxon_region_physical;

	UCHAR*					vmcs_region;
	UINT32					vmcs_region_size;
	PHYSICAL_ADDRESS		vmcs_region_physical; 

	UCHAR*					io_bitmap_a;
	UCHAR*					io_bitmap_b;
	PHYSICAL_ADDRESS		io_bitmap_a_physical;
	PHYSICAL_ADDRESS		io_bitmap_b_physical;

	UCHAR*					fake_stack;

	// todo handle_io_instruction() 함수로 구현하기
	EXITQL_IO_INSTRUCTION	io_instr_exitq;
	UINT32					port;
	UINT32					io_size;
	UINT8					port_status;
	UINT8					pc;

	UINT_PTR				vmx_fake_stack;

} VCPU, *PVCPU;


//< somma


#define IA32_VMX_BASIC_MSR_CODE			0x480
#define IA32_FEATURE_CONTROL_CODE		0x03A


typedef struct _VMX_FEATURES
{
	unsigned SSE3		:1;		// SSE3 Extensions
	unsigned RES1		:2;
	unsigned MONITOR	:1;		// MONITOR/WAIT
	unsigned DS_CPL		:1;		// CPL qualified Debug Store
	unsigned VMX		:1;		// Virtual Machine Technology
	unsigned RES2		:1;
	unsigned EST		:1;		// Enhanced Intel?Speedstep Technology
	unsigned TM2		:1;		// Thermal monitor 2
	unsigned SSSE3		:1;		// SSSE3 extensions
	unsigned CID		:1;		// L1 context ID
	unsigned RES3		:2;
	unsigned CX16		:1;		// CMPXCHG16B
	unsigned xTPR		:1;		// Update control
	unsigned PDCM		:1;		// Performance/Debug capability MSR
	unsigned RES4		:2;
	unsigned DCA		:1;
	unsigned RES5		:13;
	
} VMX_FEATURES;

//////////////
//          //
//  EFLAGS  //
//          //
//////////////
typedef struct _EFLAGS
{
	unsigned Reserved1	:10;
	unsigned ID			:1;		// Identification flag
	unsigned VIP		:1;		// Virtual interrupt pending
	unsigned VIF		:1;		// Virtual interrupt flag
	unsigned AC			:1;		// Alignment check
	unsigned VM			:1;		// Virtual 8086 mode
	unsigned RF			:1;		// Resume flag
	unsigned Reserved2	:1;
	unsigned NT			:1;		// Nested task flag
	unsigned IOPL		:2;		// I/O privilege level
	unsigned OF			:1;
	unsigned DF			:1;
	unsigned IF			:1;		// Interrupt flag
	unsigned TF			:1;		// Task flag
	unsigned SF			:1;		// Sign flag
	unsigned ZF			:1;		// Zero flag
	unsigned Reserved3	:1;
	unsigned AF			:1;		// Borrow flag
	unsigned Reserved4	:1;
	unsigned PF			:1;		// Parity flag
	unsigned Reserved5	:1;
	unsigned CF			:1;		// Carry flag [Bit 0]

} EFLAGS;

///////////
//       //
//  MSR  //
//       //
///////////
typedef struct _VMSR
{
	ULONG		Hi;
	ULONG		Lo;

} VMSR;

typedef struct _IA32_VMX_BASIC_MSR
{

	unsigned RevId			:32;	// Bits 31...0 contain the VMCS revision identifier
	unsigned szVmxOnRegion  :12;	// Bits 43...32 report # of bytes for VMXON region 
	unsigned RegionClear	:1;		// Bit 44 set only if bits 32-43 are clear
	unsigned Reserved1		:3;		// Undefined
	unsigned PhyAddrWidth	:1;		// Physical address width for referencing VMXON, VMCS, etc.
	unsigned DualMon		:1;		// Reports whether the processor supports dual-monitor
									// treatment of SMI and SMM
	unsigned MemType		:4;		// Memory type that the processor uses to access the VMCS
	unsigned VmExitReport	:1;		// Reports weather the procesor reports info in the VM-exit
									// instruction information field on VM exits due to execution
									// of the INS and OUTS instructions
	unsigned Reserved2		:9;		// Undefined

} IA32_VMX_BASIC_MSR;


typedef struct _IA32_FEATURE_CONTROL_MSR
{
	unsigned Lock			:1;		// Bit 0 is the lock bit - cannot be modified once lock is set
	unsigned Reserved1		:1;		// Undefined
	unsigned EnableVmxon	:1;		// Bit 2. If this bit is clear, VMXON causes a general protection exception
	unsigned Reserved2		:29;	// Undefined
	unsigned Reserved3		:32;	// Undefined

} IA32_FEATURE_CONTROL_MSR;

/////////////////
//             //
//  REGISTERS  //
//             //
/////////////////
typedef struct _CR0_REG
{
	unsigned PE			:1;			// Protected Mode Enabled [Bit 0]
	unsigned MP			:1;			// Monitor Coprocessor FLAG
	unsigned EM			:1;			// Emulate FLAG
	unsigned TS			:1;			// Task Switched FLAG
	unsigned ET			:1;			// Extension Type FLAG
	unsigned NE			:1;			// Numeric Error
	unsigned Reserved1	:10;		// 
	unsigned WP			:1;			// Write Protect
	unsigned Reserved2	:1;			// 
	unsigned AM			:1;			// Alignment Mask
	unsigned Reserved3	:10;		// 
	unsigned NW			:1;			// Not Write-Through
	unsigned CD			:1;			// Cache Disable
	unsigned PG			:1;			// Paging Enabled

} CR0_REG;

typedef struct _CR4_REG
{
	unsigned VME		:1;			// Virtual Mode Extensions
	unsigned PVI		:1;			// Protected-Mode Virtual Interrupts
	unsigned TSD		:1;			// Time Stamp Disable
	unsigned DE			:1;			// Debugging Extensions
	unsigned PSE		:1;			// Page Size Extensions
	unsigned PAE		:1;			// Physical Address Extension
	unsigned MCE		:1;			// Machine-Check Enable
	unsigned PGE		:1;			// Page Global Enable
	unsigned PCE		:1;			// Performance-Monitoring Counter Enable
	unsigned OSFXSR		:1;			// OS Support for FXSAVE/FXRSTOR
	unsigned OSXMMEXCPT	:1;			// OS Support for Unmasked SIMD Floating-Point Exceptions
	unsigned Reserved1	:2;			// 
	unsigned VMXE		:1;			// Virtual Machine Extensions Enabled
	unsigned Reserved2	:18;		// 

} CR4_REG;

typedef struct _MISC_DATA
{
	unsigned	Reserved1		:6;		// [0-5]
	unsigned	ActivityStates	:3;		// [6-8]
	unsigned	Reserved2		:7;		// [9-15]
	unsigned	CR3Targets		:9;		// [16-24]

	// 512*(N+1) is the recommended maximum number of MSRs
	unsigned	MaxMSRs			:3;		// [25-27]

	unsigned	Reserved3		:4;		// [28-31]
	unsigned	MSEGRevID		:32;	// [32-63]

} MISC_DATA;

/////////////////
//             //
//  SELECTORS  //
//             //
/////////////////

typedef struct _VGDTR
{
	unsigned	Limit		:16;
	unsigned	BaseLo		:16;
	unsigned	BaseHi		:16;

} VGDTR;

typedef struct _VIDTR
{
	unsigned	Limit		:16;
	unsigned	BaseLo		:16;
	unsigned	BaseHi		:16;

} VIDTR;

typedef struct	_SEG_DESCRIPTOR
{
	unsigned	LimitLo	:16;
	unsigned	BaseLo	:16;
	unsigned	BaseMid	:8;
	unsigned	Type	:4;
	unsigned	System	:1;
	unsigned	DPL		:2;
	unsigned	Present	:1;
	unsigned	LimitHi	:4;
	unsigned	AVL		:1;
	unsigned	L		:1;
	unsigned	DB		:1;
	unsigned	Gran	:1;		// Granularity
	unsigned	BaseHi	:8;
	
} SEG_DESCRIPTOR;

#pragma warning(default: 4201)
#pragma warning(default: 4214)








///////////
//       //
//  Log  //
//       //
///////////
#define Log( message, value ) { DbgPrint("[vmm] %-40s [%08X]\n", message, value ); }


NTSTATUS start_vm();
void stop_vm();












/*__declspec( naked )*/ VOID StartVMX( );
/*__declspec( naked )*/ VOID VMMEntryPoint( );




VOID ClearBit( ULONG * dword, ULONG bit );
void set_bit32(IN OUT ULONG* bit32_value, IN ULONG single_bit);
VOID SetBit( ULONG * dword, ULONG bit );


VOID WriteVMCS( ULONG encoding, ULONG value );
VOID ReadMSR( ULONG msrEncoding );
VOID WriteMSR( ULONG msrEncoding );
ULONG GetSegmentDescriptorLimit( ULONG gdt_base , USHORT seg_selector );
ULONG GetSegmentDescriptorDPL( ULONG gdt_base , USHORT seg_selector );
ULONG GetSegmentDescriptorBase( ULONG gdt_base , USHORT seg_selector );	




NTSTATUS allocate_vcpu(OUT VCPU& cpu);
void     free_vcpu(IN OUT VCPU& cpu);
//NTSTATUS handle_io_instruction(IN UINT32 exit_qualification, edx....)
