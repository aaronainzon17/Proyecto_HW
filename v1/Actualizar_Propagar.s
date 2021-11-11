	; Funcion: 
	;	Función candidatos_actualizar_arm_arm, adaptación de las funciones actualizar y propagar en arm con inlining. 
	; Parametros: 
	;	CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS] es la celda a tratar lo pasamos por R0
	; Descripcion:
	; 	Calcula todas las listas de candidatos (9x9) necesario tras borrar o cambiar un valor (listas corrompidas)
	; 	retorna el numero de celdas vacias Init del sudoku en codigo C invocando a propagar en C
	; 	Recibe la cuadricula como primer parametro y devuelve en celdas_vacias el numero de celdas vacias
	;   La función incorpora ambos códigos en arm tanto el de actualizar como el de propagar sin llamadas
	;   ahorrando así el tiempo que supone la llamada a la función propagar
	PRESERVE8
	AREA DATOS, data
region DCB 0,0,0,3,3,3,6,6,6
	AREA    f_candidatos_actualizar_arm_arm, CODE, READONLY
	EXPORT candidatos_actualizar_arm_arm
candidatos_actualizar_arm_arm	
	STMDB R13!,{R4-R9,R14}
	MOV R6,R0							; MOVEMOS A R6 CUADRICULA
	MOV R7,#0x00000000					; R7 ES CELDAS VACIAS = 0
	MOV R8,#0x00000000					; R8 ES i
INI_I
	CMP R8,#0x00000009					; i < NUM_FILAS
	BEQ FUERA_I
	MOV R9,#0x00000000					; R9 ES j
INI_J
	CMP R9,#0x00000009					; j < NUM_FILAS
	BEQ FUERA_J
	ADD R1,R6,R8,LSL #5					; INLINE FUNCION celda_establecer_todos_candidatos LSL #5 es para desplazar $r8 bytes
	ADD R0,R1,R9,LSL #1					; En R0 guardamos la direccion de la celda
	LDRB R1,[R0]						; R1 = contenido de la celda
	AND R1,R1,#0x0000007F				; Ponemos los candidatos a 0
	STRH R1,[R0]								
	ADD R9,R9,#0x00000001				; j++
	B INI_J
FUERA_J
	ADD R8,R8,#0x00000001				; i++
	B INI_I
FUERA_I
	MOV R8,#0x00000000					; R8 ES i
INI_I_2
	CMP R8,#0x00000009					; i < NUM_FILAS
	BEQ FUERA_I_2
	MOV R9,#0x00000000					; R9 ES j
INI_J_2
	CMP R9,#0x00000009					; i < NUM_FILAS
	BEQ FUERA_J_2
	ADD R1,R6,R8,LSL #5					; INLINE CELDA_LEER_VALOR
	ADD R1,R1,R9,LSL #1					; En R1 guardamos la direccion de la celda
	LDRH R0,[R1]						; R1 = contenido de la celda
	AND R1,R0,#0x0000000F				; Cogemos solo el valor
	CMP R1,#0x00000000					; Valor != 0
	ADDEQ R7,R7,#0x00000001				; CELDAS VAC?AS ++
	BEQ FIN_PROPAGAR
	MOVNE R1,R8							; PARAMETRO I
	MOVNE R2,R9							; PARAMETRO J
	MOVNE R0,R6							; PARAMETRO CUADRICULA
	;---------------------------------------------------------------------------------------BLNE candidatos_propagar_arm
	STMDB R13!,{R4-R10}
	MOV R6,R0									;@cuadricula
	MOV R7,R1									; fila
	MOV R8,R2									; columna
	
	LDR R2,=region								; @ del inicio del vector region		
	
	ADD R0,R6,R7,LSL #5							; valor = celda_leer_valor
	ADD R1,R0,R8,LSL #1							; R1 = @ celda
	LDRH R0,[R1]								; celda
	AND R1,R0,#0x0000000F						; R1 = valor 
	MOV R9,#0x00000000							; R9 es la j del primer for (iterador)
INI_FOR_1_AUX
	CMP R9,#0x00000009
	BEQ FIN_FOR_1_AUX
	ADD R4,R6,R7,LSL #5							; @ fila
	ADD R0,R4,R9,LSL #1							; R0 es la @ de la celda
	;LDR R1,[R13]	
	;celda eliminar candidatos
	MOV R4,#0x00000007
	LDRH R5,[R0]								; R5 = celdaptr
	SUB R3,R1,#0x00000001						; valor - 1
	ADD R4,R4,R3								; 7 + (valor - 1)
	MOV R10,#0x00000001
	ORR R5,R5,R10,LSL R4						; celdaptr OR 7 + (valor - 1)
	STRH R5,[R0]
	;fin celda eliminar candidatos 
	ADD R9,R9,#0x00000001						; i++
	B INI_FOR_1_AUX
FIN_FOR_1_AUX
	MOV R9,#0x00000000							; R9 es la i del segundo for
INI_FOR_2_AUX
	CMP R9,#0x00000009
	BEQ FIN_FOR_2_AUX
	ADD R4,R6,R9,LSL #5							; @ columna
	ADD R0,R4,R8,LSL #1							; R0 = @ celda
												; celda eliminar candidatos
	MOV R4,#0x00000007
	LDRH R5,[R0]								; R5 = celdaptr
	SUB R3,R1,#0x00000001						; valor - 1
	ADD R4,R4,R3								; 7 + (valor - 1)
	MOV R10,#0x00000001
	ORR R5,R5,R10,LSL R4						; celdaptr OR 7 + (valor - 1)
	STRH R5,[R0]
	;fin celda eliminar candidatos 
	ADD R9,R9,#0x00000001						; i++
	B INI_FOR_2_AUX
FIN_FOR_2_AUX

	ADD R4,R2,R7
	LDRB R3,[R4]								; R3 = init_region[fila]
	ADD R4,R2,R8
	LDRB R5,[R4]								; R5 = init_region[columna]
	ADD R4,R3,#0x00000003						; R4 = end_i
	ADD R9,R5,#0x00000003						; R9 = end_j	
INI_I_AUX
	CMP R3,R4									; i > end_i
	BEQ FUERA_I_AUX
	MOV R10,R5									; j = init_region[columna]
INI_J_AUX
	CMP R10,R9									; j < end_j
	BEQ FUERA_J_AUX
	ADD R2,R6,R3,LSL #5
	ADD R0,R2,R10,LSL #1 						; R0 = cuadricula[i][j]
												; celda eliminar candidatos
	MOV R7,#0x00000007
	LDRH R2,[R0]								; R2 = contenido de la celda
	SUB R8,R1,#0x00000001						; valor - 1
	ADD R7,R7,R8								; 7 + (valor - 1)
	MOV R8,#0x00000001
	ORR R2,R2,R8,LSL R7							; celdaptr OR 7 + (valor - 1)
	STRH R2,[R0]
												; fin celda eliminar candidatos 				
	ADD R10,R10,#0x00000001						; j++
	B INI_J_AUX
FUERA_J_AUX
	ADD R3,R3,#0x00000001						; i++
	B INI_I_AUX
FUERA_I_AUX
	LDMIA	R13!,{R4-R10}
	;---------------------------------------------------------------------------------------FIN candidatos_propagar
FIN_PROPAGAR
	ADD R9,R9,#0x00000001						; j++
	B INI_J_2
FUERA_J_2
	ADD R8,R8,#0x00000001						; i++
	B INI_I_2
FUERA_I_2
	MOV R0,R7									; Pasamos Celdad Vacias a R0 para retornarlo
	LDMIA	R13!,{R4-R9,R14}
	BX R14
	
	END