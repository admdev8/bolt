PUBLIC	intrin_SHL

_TEXT	SEGMENT

intrin_SHL PROC
	; set flags
	pushfq
	pop	rbx

	and	rbx, 0fffff72h
	or	rbx, qword ptr [r9]
	push	rbx
	popfq
	
	mov	rax, rcx
	movzx	rcx, dl
	shl	rax, cl
	mov	QWORD PTR [r8], rax

	; store flags:
	pushfq
	pop	rax
	mov	QWORD PTR [r9], rax
	ret	0
intrin_SHL ENDP

intrin_SHR PROC
	; set flags
	pushfq
	pop	rbx
	and	rbx, 0fffff72h
	or	rbx, qword ptr [r9]
	push	rbx
	popfq
	
	mov	rax, rcx
	movzx	rcx, dl
	shr	rax, cl
	mov	QWORD PTR [r8], rax

	; store flags:
	pushfq
	pop	rax
	mov	QWORD PTR [r9], rax
	ret	0
intrin_SHR ENDP

intrin_SAR PROC
	; set flags
	pushfq
	pop	rbx
	and	rbx, 0fffff72h
	or	rbx, qword ptr [r9]
	push	rbx
	popfq
	
	mov	rax, rcx
	movzx	rcx, dl
	sar	rax, cl
	mov	QWORD PTR [r8], rax

	; store flags:
	pushfq
	pop	rax
	mov	QWORD PTR [r9], rax
	ret	0
intrin_SAR ENDP

intrin_ADD PROC
	; set flags
	pushfq
	pop	rbx
	and	rbx, 0fffff72h
	or	rbx, qword ptr [r9]
	push	rbx
	popfq
	
	add	rcx, rdx
	mov	QWORD PTR [r8], rcx

	; store flags:
	pushfq
	pop	rax
	mov	QWORD PTR [r9], rax
	ret	0
intrin_ADD ENDP

intrin_ADC PROC
	; set flags
	pushfq
	pop	rbx
	and	rbx, 0fffff72h
	or	rbx, qword ptr [r9]
	push	rbx
	popfq
	
	adc	rcx, rdx
	mov	QWORD PTR [r8], rcx

	; store flags:
	pushfq
	pop	rax
	mov	QWORD PTR [r9], rax
	ret	0
intrin_ADC ENDP

intrin_SUB PROC
	; set flags
	pushfq
	pop	rbx
	and	rbx, 0fffff72h
	or	rbx, qword ptr [r9]
	push	rbx
	popfq
	
	sub	rcx, rdx
	mov	QWORD PTR [r8], rcx

	; store flags:
	pushfq
	pop	rax
	mov	QWORD PTR [r9], rax
	ret	0
intrin_SUB ENDP

intrin_SBB PROC
	; set flags
	pushfq
	pop	rbx
	and	rbx, 0fffff72h
	or	rbx, qword ptr [r9]
	push	rbx
	popfq
	
	sbb	rcx, rdx
	mov	QWORD PTR [r8], rcx

	; store flags:
	pushfq
	pop	rax
	mov	QWORD PTR [r9], rax
	ret	0
intrin_SBB ENDP

intrin_XOR PROC
	; set flags
	pushfq
	pop	rbx
	and	rbx, 0fffff72h
	or	rbx, qword ptr [r9]
	push	rbx
	popfq
	
	xor	rcx, rdx
	mov	QWORD PTR [r8], rcx

	; store flags:
	pushfq
	pop	rax
	mov	QWORD PTR [r9], rax
	ret	0
intrin_XOR ENDP

intrin_OR PROC
	; set flags
	pushfq
	pop	rbx
	and	rbx, 0fffff72h
	or	rbx, qword ptr [r9]
	push	rbx
	popfq
	
	or	rcx, rdx
	mov	QWORD PTR [r8], rcx

	; store flags:
	pushfq
	pop	rax
	mov	QWORD PTR [r9], rax
	ret	0
intrin_OR ENDP

intrin_AND PROC
	; set flags
	pushfq
	pop	rbx
	and	rbx, 0fffff72h
	or	rbx, qword ptr [r9]
	push	rbx
	popfq
	
	and	rcx, rdx
	mov	QWORD PTR [r8], rcx

	; store flags:
	pushfq
	pop	rax
	mov	QWORD PTR [r9], rax
	ret	0
intrin_AND ENDP

intrin_NOT PROC
	; set flags
	pushfq
	pop	rbx
	and	rbx, 0fffff72h
	or	rbx, qword ptr [r8]
	push	rbx
	popfq

	not	rcx
	mov	QWORD PTR [rdx], rcx

	; store flags:
	pushfq
	pop	rax
	mov	QWORD PTR [r8], rax
	ret	0
intrin_NOT ENDP

intrin_NEG PROC
	; set flags
	pushfq
	pop	rbx
	and	rbx, 0fffff72h
	or	rbx, qword ptr [r8]
	push	rbx
	popfq

	neg	rcx
	mov	QWORD PTR [rdx], rcx

	; store flags:
	pushfq
	pop	rax
	mov	QWORD PTR [r8], rax
	ret	0
intrin_NEG ENDP

_TEXT	ENDS

	END
