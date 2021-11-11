	; Función actualizar_arm_c, función en arm que llama a la función propagar en c.
	; Cabecera: candidatos_actualizar_arm_c(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS])
	; Parametros: CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS] es la celda a tratar
	; Descripcion:
	; calcula todas las listas de candidatos (9x9)
	; necesario tras borrar o cambiar un valor (listas corrompidas)
	; retorna el numero de celdas vacias
	; Init del sudoku en codigo C invocando a propagar en C
	; Recibe la cuadricula como primer parametro
	; y devuelve en celdas_vacias el numero de celdas vacias
	PRESERVE8
	AREA    f_candidatos_actualizar_arm_c, CODE, READONLY
	EXPORT candidatos_actualizar_arm_c
	IMPORT candidatos_propagar_c
candidatos_actualizar_arm_c		
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
	ADD R0,R1,R9,LSL #1					; En R1 previamente hemos calculado la fila y en R0 calculamos la celda
	LDRB R1,[R0]						; Cargamos en R1 el contenido de la celda
	AND R1,R1,#0x0000007F				; Ponemos a 0 los candidatos
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
	CMP R9,#0x00000009					; j < NUM_FILAS
	BEQ FUERA_J_2
	ADD R1,R6,R8,LSL #5					; INLINE CELDA_LEER_VALOR
	ADD R1,R1,R9,LSL #1					; Calculamos la celda en R1
	LDRH R0,[R1]						; Guardamos el contenido de la celda en R0
	AND R1,R0,#0x0000000F
	CMP R1,#0x00000000
	ADDEQ R7,R7,#0x00000001				; CELDAS VAC?AS ++
	MOVNE R1,R8							; PARAMETRO I
	MOVNE R2,R9							; PARAMETRO J
	MOVNE R0,R6							; PARAMETRO CUADRICULA
	BLNE candidatos_propagar_c
	ADD R9,R9,#0x00000001				; j++
	B INI_J_2
FUERA_J_2
	ADD R8,R8,#0x00000001				; i++
	B INI_I_2
FUERA_I_2
	MOV R0,R7							; Pasamos Celdad Vacias a R0 para retornarlo
	LDMIA	R13!,{R4-R9,R14}
	BX R14
	
	END