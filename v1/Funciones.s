;EXPORT f_actualizar_c_arm
;f_actualizar_c_arm    

		STMDB R13!,{R4-R9,R14}
		MOV R6,R0							; MOVEMOS A R6 CUADRICULA
		MOV R7,#0x00000000					; R7 ES CELDAS VAC?AS = 0
		MOV R8,#0x00000000					; R8 ES i
INI_I
		CMP R8,#0x00000009					
		BGT FUERA_I
		MOV R9,#0x00000000					; R9 ES j
INI_J
		CMP R9,#0x00000009
		BGT FUERA_J
		ADD R1,R6,R8,LSL #5					; INLINE FUNCION celda_establecer_todos_candidatos
		ADD R0,R1,R9,LSL #1
		LDRB R1,[R0]
		AND R1,R1,#0x0000007F
		STRH R1,[R0]								
		ADD R9,R9,#0x00000001				; j++
		B INI_J
FUERA_J
		ADD R8,R8,#0x00000001				; i++
		B INI_I
FUERA_I
		MOV R8,#0x00000000					; R8 ES i
INI_I_2
		CMP R8,#0x00000009					
		BGT FUERA_I_2
		MOV R9,#0x00000000					; R9 ES j
INI_J_2
		CMP R9,#0x00000009
		BGT FUERA_J_2
		ADD R1,R6,R8,LSL #5					; INLINE CELDA_LEER_VALOR
		ADD R1,R1,R9,LSL #1
		LDRH R0,[R1]
		AND R1,R0,#0x0000000F
		CMP R1,#0x00000000
		ADDEQ R7,R7,#0x00000001				; CELDAS VAC?AS ++
		MOVEQ R1,R8							; PARAMETRO I
		MOVEQ R2,R9							; PARAMETRO J
		MOVEQ R0,R6							; PARAMETRO CUADRICULA
		BLNE candidatos_propagar_c
		ADD R9,R9,#0x00000001				; j++
		B INI_J_2
FUERA_J_2
		ADD R8,R8,#0x00000001				; i++
		B INI_I_2
FUERA_I_2
		MOV R0,R7							;
		LDMIA	R13!,{R4-R9,R14}
		BX R14