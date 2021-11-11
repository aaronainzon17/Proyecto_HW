/*#include <LPC210X.H>                            // LPC21XX Peripheral Registers
#include "mutex.h"

unsigned int semaphore;

void mutex_gatelock (void)
{
	 __asm
		 {
		 spin:
		 MOV r1, &semaphore
		 MOV r2, #1
		 SWP r3,r2,[r1]
		 CMP r3,#1
		 BEQ spin
		 }
 }
void mutex_gateunlock (void)
{
	__asm
		{
		MOV r1, &semaphore
		MOV r2, #0
		SWP r0,r2,[r1]
		}
}