	PRESERVE8
	AREA  AreaGestor_SC, CODE, READONLY
	EXPORT gestor_SC_in
gestor_SC_in PROC
	MRS    R0,CPSR
	ORR    R0,R0,#0x80
	MSR    CPSR_c,R0
	bx lr
	ENDP
		
	EXPORT gestor_SC_out
gestor_SC_out PROC		
	MRS    R0,CPSR
	BIC    R0,R0,#0x80
	MSR    CPSR_c,R0
	bx lr
	ENDP
	END
