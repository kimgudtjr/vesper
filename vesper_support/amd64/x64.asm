;;=============================================================================
;; 
;;
;;=============================================================================
.CODE


; void _break();
;------------------------------------------------------------------------------
_break PROC
	int 3
	ret
_break ENDP

;;=============================================================================
;; MSR stuff
;;=============================================================================

;ULONG64 x64_read_msr(IN UINT32 msr_index);
x64_read_msr proc
	rdmsr
	shl	rdx, 32
	or	rax, rdx
	ret
x64_read_msr endp

; void x64_write_msr(IN UINT32 msr_index, IN UINT32 msr_low, IN UINT32 msr_high)
x64_write_msr proc
	mov eax, edx
	mov edx, r8d
	wrmsr
	ret
x64_write_msr ENDP




END




