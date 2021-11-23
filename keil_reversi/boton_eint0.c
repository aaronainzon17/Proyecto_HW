#include "boton_eint0.h"
#include "cola.h"
#include <LPC210X.H>                            // LPC21XX Peripheral Registers

// variable para comprobar que se hacen las interrupciones que deber�an hacerse
static volatile unsigned int eint1_count = 0;
static volatile unsigned int eint2_count = 0;
// variable que se activa al detectar una nueva pulsaci�n
static volatile int eint1_nueva_pulsacion = 0;
static volatile int eint2_nueva_pulsacion = 0;
static volatile int BIT_EINT1 = 15;
static volatile int BIT_EINT2 = 16;

void eint1_ISR (void) __irq;
void eint2_ISR (void) __irq;


void eint1_ISR (void) __irq {
	eint1_count++;
	EXTINT |= 0x2;        // clear interrupt flag        
	VICVectAddr = 1;             // Acknowledge Interrupt
	eint1_nueva_pulsacion = 1;		//Ponemos a 1 el flag de pulsación
	cola_guardar_eventos(1,0);			//Encolamos el evento de pulsación en EINT1
	VICIntEnClr |= (1<< BIT_EINT1);			// Inhabilitamos las interrupciones
	VICIntEnable &= ~(1<< BIT_EINT1);			// Inhabilitamos las interrupciones
}
void eint2_ISR (void) __irq {
	eint2_count++;
	EXTINT |= 0x4;        // clear interrupt flag        
	VICVectAddr = 1;             // Acknowledge Interrupt
	eint2_nueva_pulsacion = 1;
	cola_guardar_eventos(2,0);
	VICIntEnClr |= (1<< BIT_EINT2);			// Inhabilitamos las interrupciones
	VICIntEnable &= ~(1<< BIT_EINT2);
}

void eint1_clear_nueva_pulsacion(void){
	eint1_nueva_pulsacion = 0;
	EXTINT |= 0x2;        // clear interrupt flag
	VICIntEnClr &= ~(1<< BIT_EINT1);			// habilitamos las interrupciones
	VICIntEnable = VICIntEnable | 0x00008000;                  // Enable EXTINT0 Interrupt
};

void eint2_clear_nueva_pulsacion(void){
	eint2_nueva_pulsacion = 0;
	EXTINT |= 0x4;        // clear interrupt flag   
	VICIntEnClr &= ~(1<< BIT_EINT2);			// habilitamos las interrupciones
	VICIntEnable = VICIntEnable | 0x00010000;                  // Enable EXTINT0 Interrupt
};

unsigned int eint1_read_nueva_pulsacion(void){
	return eint1_nueva_pulsacion;
};

unsigned int eint2_read_nueva_pulsacion(void){
	return eint2_nueva_pulsacion;
};

unsigned int eint1_read_count(void){
	return eint1_count;
};

unsigned int eint2_read_count(void){
	return eint2_count;
};

void eint0_init (void) {
// NOTA: seg�n el manual se puede configurar c�mo se activan las interrupciones: por flanco o nivel, alta o baja. 
// Se usar�an los registros EXTMODE y EXTPOLAR. 
// Sin embargo parece que el simulador los ignora y no aparecen en la ventana de ocnfiguraci�n de EXT Interrupts
// configure EXTINT0 active if a rising-edge is detected
//	EXTMODE 	=	1; //1 edge, 0 level
//	EXTPOLAR	=	1; // 1 high, rising-edge; 0 low, falling-edge
//  prueba = EXTMODE;
	eint1_nueva_pulsacion = 0;
	eint1_count = 0;
	EXTINT |= 0x2;        // clear interrupt flag       	
	// configuration of the IRQ slot number 2 of the VIC for EXTINT0
	VICVectAddr2 = (unsigned long)eint1_ISR;          // set interrupt vector in 0
	VICVectCntl2 = 0x20 | 15;    
    // 0x20 bit 5 enables vectored IRQs. 
		// 15 is the number of the interrupt assigned. Number 15 is the EINT1 (see table 40 of the LPC2105 user manual  
	PINSEL0 		= PINSEL0 & 0xCfffffff;	//Sets bits 0 and 1 to 0
	PINSEL0 		= PINSEL0 | 0x20000000;					
	               
  VICIntEnable = VICIntEnable | 0x00008000;                 // Enable EXTINT1 Interrupt
	
	eint2_nueva_pulsacion = 0;
	eint2_count = 0;
	EXTINT |= 0x4;        // clear interrupt flag     	
	// configuration of the IRQ slot number 2 of the VIC for EXTINT0
	VICVectAddr3 = (unsigned long)eint2_ISR;          // set interrupt vector in 0
	VICVectCntl3 = 0x20 | 16;   
    // 0x20 bit 5 enables vectored IRQs. 
		// 16 is the number of the interrupt assigned. Number 16 is the EINT2 (see table 40 of the LPC2105 user manual  
	PINSEL0 		= PINSEL0 & 0x3fffffff;	//Sets bits 0 and 1 to 0
	PINSEL0 		= PINSEL0 | 0x80000000;					
	VICVectCntl3 = 0x20 | 16;                   
  VICIntEnable = VICIntEnable | 0x00010000;                  // Enable EXTINT2 Interrupt
}
