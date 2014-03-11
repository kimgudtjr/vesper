;;=============================================================================
;; 
;;
;;=============================================================================
.686
.model flat, StdCall
OPTION CASEMAP:NONE
.CODE

; void _break();
;------------------------------------------------------------------------------
_break proc stdcall 
	int 3
	ret
_break endp


;;=============================================================================
;; MSR stuff
;;=============================================================================

; void __stdcall x86_read_msr(IN UINT32 msr_index, OUT MSR* msr);
;------------------------------------------------------------------------------
_rdmsr MACRO
	byte	0fh, 032h
ENDM

x86_read_msr PROC StdCall _msr_index, _msr
	pushad	
	mov		ecx, _msr_index
	_rdmsr	
	
	mov		ecx, dword ptr [ebp+0ch]		; _msr
	mov		dword ptr [ecx], eax			; _msr.low
	mov		dword ptr [ecx+04h], edx		; _msr.high

	popad	
	ret
x86_read_msr ENDP

; void __stdcall x86_write_msr(IN UINT32 msr_index, IN UINT32 msr_low, IN UINT32 msr_high)
;------------------------------------------------------------------------------
_wrmsr	MACRO
	byte	0fh, 030h	
ENDM

x86_write_msr PROC StdCall _msr_index, _msr_low, _msr_high
	pushad
	mov		ecx, _msr_index
	mov		edx, _msr_high
	mov		eax, _msr_low
	_wrmsr
	popad
	ret
x86_write_msr ENDP


END




