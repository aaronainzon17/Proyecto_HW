#include <LPC210X.H>                            // LPC21XX Peripheral Registers
#include "temporizador.h"
//#include "cola.h"
#include "eventos.h"

static volatile unsigned int timer0_int_count = 0;
static volatile unsigned int timer1_int_count = 0;
static volatile unsigned int TC_init = 0; 								//Ticks al empezar
static volatile unsigned int periodo_alarm = 0; 					//Periodo del temporizador
static volatile unsigned int siguiente_periodo = 0;

void timer0_ISR (void) __irq;    // Generate Interrupt Alarmas
void timer1_ISR (void) __irq;    // Generate Interrupt 

void temporizador_iniciar(void){
		timer0_int_count = 0;
		timer1_int_count = 0;
		// configuration of Timer 0
		T0MR0 = 149999;                        // Interrumpe cada 0,05ms = 150.000-1 counts
    T0MCR = 3;                     					// Generates an interrupt and resets the count when the value of MR0 is reached

    // configuration of the IRQ slot number 0 of the VIC for Timer 0 Interrupt
		VICVectAddr0 = (unsigned long)timer0_ISR;          // set interrupt vector in 0
    // 0x20 bit 5 enables vectored IRQs. 
		// 4 is the number of the interrupt assigned. Number 4 is the Timer 0 (see table 40 of the LPC2105 user manual  
		VICVectCntl0 = 0x20 | 4;                   
    VICIntEnable = VICIntEnable | 0x00000010;                  // Enable Timer0 Interrupt
	
		timer1_int_count = 0;	
	// configuration of Timer 0
		T1MR0 = 149999;                        // Interrumpe cada 0,05ms = 150.000-1 counts
    T1MCR = 3;                     // Generates an interrupt and resets the count when the value of MR0 is reached

    // configuration of the IRQ slot number 0 of the VIC for Timer 0 Interrupt
		VICVectAddr1 = (unsigned long)timer1_ISR;          // set interrupt vector in 0
    // 0x20 bit 5 enables vectored IRQs. 
		// 4 is the number of the interrupt assigned. Number 4 is the Timer 0 (see table 40 of the LPC2105 user manual  
		VICVectCntl1 = 0x20 | 4;                   
    VICIntEnable = VICIntEnable | 0x00000020;                  // Enable Timer0 Interrupt
}

void temporizador_empezar(void){
	T1TCR = 1;                             // Timer1 Enable
	timer1_int_count = 0;
	TC_init = T1TC;
}

int temporizador_leer(void){
	return T1TC - TC_init;
}

int temporizador_parar(void){
	T1TCR = 0;
	return T1TC - TC_init;
}

void timer1_ISR(void) __irq{
	 timer1_int_count++;
	 T1IR = 1;                              			// Clear interrupt flag
	 VICVectAddr = 1;                            	// Acknowledge Interrupt
}

void temporizador_periodo(int periodo){
		T0TCR = 1;                             			// Timer0 Enable
	  periodo_alarm = periodo * 10;
		siguiente_periodo = timer0_int_count + periodo_alarm;
}

void timer0_ISR (void) __irq {
    timer0_int_count++;
		if (timer0_int_count != 0 && timer0_int_count == siguiente_periodo){	// Cuando llega al final encolamos evento
			//cola_guardar_eventos(ID_Alarma,AuxData);
		}
    T0IR = 1;                              			// Clear interrupt flag
    VICVectAddr = 0;                            // Acknowledge Interrupt
}
