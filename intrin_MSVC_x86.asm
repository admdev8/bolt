;PUBLIC	_intrin_SHL

_TEXT	SEGMENT use32

arg3 equ esp+4
arg4 equ esp+8

@intrin_SHL@16 PROC
	; set flags
	pushfd
	pop	ebx

	and	ebx, 0fffff72h
	mov	eax, dword ptr [arg4]
	or	ebx, [eax]
	push	ebx
	popfd

	mov	eax, ecx
	movzx	ecx, dl
	shl	eax, cl
	mov	ebx, dword ptr [arg3]
	mov	DWORD PTR [ebx], eax

	; store flags:
	pushfd
	pop	eax
	mov	ebx, dword ptr [arg4]
	mov	DWORD PTR [ebx], eax
	ret	8
@intrin_SHL@16 ENDP

@intrin_SHR@16 PROC
	; set flags
	pushfd
	pop	ebx
	and	ebx, 0fffff72h
	mov	eax, dword ptr [arg4]
	or	ebx, dword ptr [eax]
	push	ebx
	popfd
	
	mov	eax, ecx
	movzx	ecx, dl
	shr	eax, cl
	mov	ebx, dword ptr [arg3]
	mov	DWORD PTR [ebx], eax

	; store flags:
	pushfd
	pop	eax
	mov	ebx, dword ptr [arg4]
	mov	DWORD PTR [ebx], eax
	ret	8
@intrin_SHR@16 ENDP

@intrin_SAR@16 PROC
	; set flags
	pushfd
	pop	ebx
	and	ebx, 0fffff72h
	mov	eax, dword ptr [arg4]
	or	ebx, dword ptr [eax]
	push	ebx
	popfd
	
	mov	eax, ecx
	movzx	ecx, dl
	sar	eax, cl
	mov	ebx, dword ptr [arg3]
	mov	DWORD PTR [ebx], eax

	; store flags:
	pushfd
	pop	eax
	mov	ebx, dword ptr [arg4]
	mov	DWORD PTR [ebx], eax
	ret	8
@intrin_SAR@16 ENDP

@intrin_ADD@16 PROC
	; set flags
	pushfd
	pop	ebx
	and	ebx, 0fffff72h
	mov	eax, dword ptr [arg4]
	or	ebx, dword ptr [eax]
	push	ebx
	popfd
	
	add	ecx, edx
	mov	eax, DWORD PTR [arg3]
	mov	DWORD PTR [eax], ecx

	; store flags:
	pushfd
	pop	eax
	mov	ebx, DWORD PTR [arg4]
	mov	DWORD PTR [ebx], eax
	ret	8
@intrin_ADD@16 ENDP

@intrin_ADC@16 PROC
	; set flags
	pushfd
	pop	ebx
	and	ebx, 0fffff72h
	mov	eax, dword ptr [arg4]
	or	ebx, dword ptr [eax]
	push	ebx
	popfd
	
	adc	ecx, edx
	mov	eax, DWORD PTR [arg3]
	mov	DWORD PTR [eax], ecx

	; store flags:
	pushfd
	pop	eax
	mov	ebx, DWORD PTR [arg4]
	mov	DWORD PTR [ebx], eax
	ret	8
@intrin_ADC@16 ENDP

@intrin_SUB@16 PROC
	; set flags
	pushfd
	pop	ebx
	and	ebx, 0fffff72h
	mov	eax, dword ptr [arg4]
	or	ebx, dword ptr [eax]
	push	ebx
	popfd
	
	sub	ecx, edx
	mov	eax, DWORD PTR [arg3]
	mov	DWORD PTR [eax], ecx

	; store flags:
	pushfd
	pop	eax
	mov	ebx, DWORD PTR [arg4]
	mov	DWORD PTR [ebx], eax
	ret	8
@intrin_SUB@16 ENDP

@intrin_SBB@16 PROC
	; set flags
	pushfd
	pop	ebx
	and	ebx, 0fffff72h
	mov	eax, dword ptr [arg4]
	or	ebx, dword ptr [eax]
	push	ebx
	popfd
	
	sbb	ecx, edx
	mov	eax, DWORD PTR [arg3]
	mov	DWORD PTR [eax], ecx

	; store flags:
	pushfd
	pop	eax
	mov	ebx, DWORD PTR [arg4]
	mov	DWORD PTR [ebx], eax
	ret	8
@intrin_SBB@16 ENDP

@intrin_XOR@16 PROC
	; set flags
	pushfd
	pop	ebx
	and	ebx, 0fffff72h
	mov	eax, dword ptr [arg4]
	or	ebx, dword ptr [eax]
	push	ebx
	popfd
	
	xor	ecx, edx
	mov	eax, DWORD PTR [arg3]
	mov	DWORD PTR [eax], ecx

	; store flags:
	pushfd
	pop	eax
	mov	ebx, DWORD PTR [arg4]
	mov	DWORD PTR [ebx], eax
	ret	8
@intrin_XOR@16 ENDP

@intrin_OR@16 PROC
	; set flags
	pushfd
	pop	ebx
	and	ebx, 0fffff72h
	mov	eax, dword ptr [arg4]
	or	ebx, dword ptr [eax]
	push	ebx
	popfd
	
	or	ecx, edx
	mov	eax, DWORD PTR [arg3]
	mov	DWORD PTR [eax], ecx

	; store flags:
	pushfd
	pop	eax
	mov	ebx, DWORD PTR [arg4]
	mov	DWORD PTR [ebx], eax
	ret	8
@intrin_OR@16 ENDP

@intrin_AND@16 PROC
	; set flags
	pushfd
	pop	ebx
	and	ebx, 0fffff72h
	mov	eax, dword ptr [arg4]
	or	ebx, dword ptr [eax]
	push	ebx
	popfd
	
	and	ecx, edx
	mov	eax, DWORD PTR [arg3]
	mov	DWORD PTR [eax], ecx

	; store flags:
	pushfd
	pop	eax
	mov	ebx, DWORD PTR [arg4]
	mov	DWORD PTR [ebx], eax
	ret	8
@intrin_AND@16 ENDP

@intrin_NOT@12 PROC
	; set flags
	pushfd
	pop	ebx
	and	ebx, 0fffff72h
	mov	eax, dword ptr [arg3]
	or	ebx, dword ptr [eax]
	push	ebx
	popfd

	not	ecx
	mov	DWORD PTR [edx], ecx

	; store flags:
	pushfd
	pop	eax
	mov	ebx, dword ptr [arg3]
	mov	DWORD PTR [ebx], eax
	ret	4
@intrin_NOT@12 ENDP

@intrin_NEG@12 PROC
	; set flags
	pushfd
	pop	ebx
	and	ebx, 0fffff72h
	mov	eax, dword ptr [arg3]
	or	ebx, dword ptr [eax]
	push	ebx
	popfd

	neg	ecx
	mov	DWORD PTR [edx], ecx

	; store flags:
	pushfd
	pop	eax
	mov	ebx, dword ptr [arg3]
	mov	DWORD PTR [ebx], eax
	ret	4
@intrin_NEG@12 ENDP

_TEXT	ENDS

	END
