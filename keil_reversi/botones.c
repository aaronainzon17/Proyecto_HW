#include "botones.h"
#include <LPC210X.H>                            // LPC21XX Peripheral Registers

// variable para comprobar que se hacen las interrupciones que deberían hacerse
static volatile int eint0_cuenta = 0;
// variable que se activa al detectar una nueva pulsación
static volatile int eint0_nueva_pulsacion = 0;

void eint0_ISR (void) __irq {
	eint0_cuenta++;
	VICIntEnClr = 0x00004000;    //deshabilitamos eint0
	EXTINT =  EXTINT | 1;        // clear interrupt flag        
	VICVectAddr = 0;             // Acknowledge Interrupt
	eint0_nueva_pulsacion = 1;
}

void eint0_clear_nueva_pulsacion(){
	eint0_nueva_pulsacion = 0;
};

unsigned int eint0_read_nueva_pulsacion(){
	return eint0_nueva_pulsacion;
};

unsigned int eint0_read_cuenta(){
	return eint0_nueva_pulsacion;
};

void eint0_init (void) {
// NOTA: según el manual se puede configurar cómo se activan las interrupciones: por flanco o nivel, alta o baja. 
// Se usarían los registros EXTMODE y EXTPOLAR. 
// Sin embargo parece que el simulador los ignora y no aparecen en la ventana de ocnfiguración de EXT Interrupts
// configure EXTINT0 active if a rising-edge is detected
//	EXTMODE 	=	1; //1 edge, 0 level
//	EXTPOLAR	=	1; // 1 high, rising-edge; 0 low, falling-edge
//  prueba = EXTMODE;
	eint0_nueva_pulsacion = 0;
	eint0_cuenta = 0;
	EXTINT =  EXTINT | 1;        // clear interrupt flag     	
	// configuration of the IRQ slot number 2 of the VIC for EXTINT0
	VICVectAddr2 = (unsigned long)eint0_ISR;          // set interrupt vector in 0
    // 0x20 bit 5 enables vectored IRQs. 
		// 14 is the number of the interrupt assigned. Number 14 is the EINT0 (see table 40 of the LPC2105 user manual  
	PINSEL1 		= PINSEL1 & 0xfffffffC;	//Sets bits 0 and 1 to 0
	PINSEL1 		= PINSEL1 | 1;					//Enable the EXTINT0 interrupt
	VICVectCntl2 = 0x20 | 14;                   
  VICIntEnable = VICIntEnable | 0x00004000;                  // Enable EXTINT0 Interrupt
}
