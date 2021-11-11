	PRESERVE8
	AREA DATOS, data
semaphore DCB 1
	AREA    sem, CODE, READONLY
	EXPORT lock_sem
lock_sem PROC
spin
	MOV r1, &semaphore
	MOV r2, #1
	SWP r3,r2,[r1]
	CMP r3, #1
	BEQ spin
	ENDP
		
	END