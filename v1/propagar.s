	; Funcion: 
	;	Función propagar_arm, adaptación de la función propagar a ARM.
	; Parametros de la funcion: 
	;	R0: @tablero, R1: fila, R2:columna 
	; Descripcion:
	; 	La funcion candidatos_propagar_arm propaga el valor de una determinada celda en C
	; 	para actualizar las listas de candidatos de las celdas en su su fila, columna y región
	; 	Recibe como parametro la cuadricula, y la fila y columna de la celda a propagar; no devuelve nada
	PRESERVE8
	AREA DATOS, data
region DCB 0,0,0,3,3,3,6,6,6
	AREA    f_candidatos_propagar_arm, CODE, READONLY
	EXPORT candidatos_propagar_arm
candidatos_propagar_arm
	STMDB R13!,{R4-R10,R14}
	MOV R6,R0									; R6 es @cuadricula
	MOV R7,R1									; R7 es la fila
	MOV R8,R2									; R8 es la columna
	
	LDR R2,=region								; R2 es la @ del inicio del vector region		
	
	ADD R0,R6,R7,LSL #5							; valor = celda_leer_valor
	ADD R1,R0,R8,LSL #1							; En R1 hemos calculado la @ celda
	LDRH R0,[R1]								; En R0 guardamos el valor de la celda
	AND R1,R0,#0x0000000F						; valor 							
	MOV R9,#0x00000000							; R9 es la j del primer for (iterador)
INI_FOR_1
	CMP R9,#0x00000009
	BEQ FIN_FOR_1
	ADD R4,R6,R7,LSL #5							; @ fila
	ADD R0,R4,R9,LSL #1							; En R0 guardamos la @ celda
	MOV R4,#0x00000007
	LDRH R5,[R0]								; En R5 guardamos el valor de la celda
	SUB R3,R1,#0x00000001						; valor - 1
	ADD R4,R4,R3								; 7 + (valor - 1)
	MOV R10,#0x00000001
	ORR R5,R5,R10,LSL R4						; celdaptr OR 7 + (valor - 1)
	STRH R5,[R0]
	;fin celda eliminar candidatos 
	ADD R9,R9,#0x00000001
	B INI_FOR_1
FIN_FOR_1
	MOV R9,#0x00000000							; R9 es la i del segundo for
INI_FOR_2
	CMP R9,#0x00000009
	BEQ FIN_FOR_2
	ADD R4,R6,R9,LSL #5							; @ columna
	ADD R0,R4,R8,LSL #1							; En R0 guardamos la @ celda
	;LDR R1,[R13]
	;celda eliminar candidatos
	MOV R4,#0x00000007
	LDRH R5,[R0]								; En R5 guardamos el valor de la celda
	SUB R3,R1,#0x00000001						; valor - 1
	ADD R4,R4,R3								; 7 + (valor - 1)
	MOV R10,#0x00000001
	ORR R5,R5,R10,LSL R4						; celdaptr OR 7 + (valor - 1)
	STRH R5,[R0]
	;fin celda eliminar candidatos 
	ADD R9,R9,#0x00000001
	B INI_FOR_2
FIN_FOR_2

	ADD R4,R2,R7
	LDRB R3,[R4]								; R3 = init_region[fila]
	ADD R4,R2,R8
	LDRB R5,[R4]								; R5 = init_region[columna]
	ADD R4,R3,#0x00000003						; R4 = end_i
	ADD R9,R5,#0x00000003						; R9 = end_j	
INI_I
	CMP R3,R4					
	BEQ FUERA_I
	MOV R10,R5
INI_J
	CMP R10,R9
	BEQ FUERA_J
	ADD R2,R6,R3,LSL #5
	ADD R0,R2,R10,LSL #1 						; En R0 guardamos cuadricula[i][j]
	;celda eliminar candidatos
	MOV R7,#0x00000007
	LDRH R2,[R0]								; R2 = celdaptr
	SUB R8,R1,#0x00000001						; valor - 1
	ADD R7,R7,R8								; 7 + (valor - 1)
	MOV R8,#0x00000001
	ORR R2,R2,R8,LSL R7							; celdaptr OR 7 + (valor - 1)
	STRH R2,[R0]
	;fin celda eliminar candidatos 				
	ADD R10,R10,#0x00000001						; j++
	B INI_J
FUERA_J
	ADD R3,R3,#0x00000001						; i++
	B INI_I
FUERA_I
	LDMIA	R13!,{R4-R10,R14}
	BX R14
	END